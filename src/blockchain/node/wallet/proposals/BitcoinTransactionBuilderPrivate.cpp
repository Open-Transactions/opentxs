// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/proposals/BitcoinTransactionBuilderPrivate.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <BlockchainTransactionProposal.pb.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/transaction/TransactionPrivate.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Spend.hpp"
#include "internal/blockchain/node/SpendPolicy.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/core/Amount.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Category.hpp"
#include "opentxs/blockchain/Type.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Funding.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/TxoTag.hpp"    // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Element.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/OP.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Pattern.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Position.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/bitcoincash/token/cashtoken/Types.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/display/Definition.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
template <typename CB>
auto BitcoinTransactionBuilderPrivate::make_change_output(
    const crypto::Element& element,
    CB get_output,
    std::optional<TxoTag> tag) noexcept(false) -> void
{
    add_change_output([&] {
        using enum protocol::bitcoin::base::block::script::Position;
        // TODO allocator
        auto script =
            factory::BitcoinScript(chain_, std::invoke(get_output), Output, {});

        if (false == script.IsValid()) {

            throw std::runtime_error{"Failed to construct script"};
        }

        auto output = factory::BitcoinTransactionOutput(
            chain_,
            shorten(outputs_.size()),
            0,
            std::move(script),
            std::nullopt,  // TODO cashtoken
            {element.KeyID()},
            {}  // TODO allocator
        );

        if (tag.has_value()) { output.Internal().AddTag(*tag); }

        return output;
    }());
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
using namespace std::literals;

using enum BuildResult;
using enum Funding;
using enum SendResult;

BitcoinTransactionBuilderPrivate::BitcoinTransactionBuilderPrivate(
    const api::Session& api,
    const node::Manager& node,
    const identifier::Generic& id,
    const Type chain,
    database::Wallet& db,
    node::Spend& proposal,
    std::promise<SendOutcome>& promise) noexcept
    : api_(api)
    , node_(node)
    , id_(id)
    , chain_(chain)
    , db_(db)
    , proposal_(proposal)
    , promise_(promise)
    , sender_(api_.Wallet().Nym(proposal_.Spender()))
    , self_contact_(api_.Crypto().Blockchain().Internal().Contacts().ContactID(
          sender_->ID()))
    , fee_rate_(node_.Internal().FeeRate())
    , version_(1)
    , lock_time_(0)
    , segwit_(false)
    , outputs_()
    , change_()
    , inputs_()
    , fixed_overhead_(sizeof(version_) + sizeof(lock_time_))
    , input_count_()
    , output_count_()
    , input_total_()
    , witness_total_()
    , output_total_()
    , input_value_()
    , output_value_()
    , notification_value_()
    , change_keys_()
    , outgoing_keys_(proposal_.Internal().OutgoingKeys())
{
    assert_false(nullptr == sender_);
}

auto BitcoinTransactionBuilderPrivate::add_change() noexcept -> bool
{
    try {
        if (change_.empty()) {
            const auto reason =
                api_.Factory().PasswordPrompt("Calculating change output");
            const auto& element = next_change_element(reason);
            make_change_output(
                element, [&, this] { return make_p2pkh_change(element); });
        }

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto BitcoinTransactionBuilderPrivate::add_change_output(
    Output output) noexcept(false) -> void
{
    if (false == output.IsValid()) {

        throw std::runtime_error{"Failed to construct change output"};
    }

    auto& internal = output.Internal();
    internal.SetPayee(self_contact_);
    internal.SetPayer(self_contact_);
    internal.AddTag(TxoTag::Change);
    output_value_ += internal.Value();
    output_total_ += internal.CalculateSize();
    change_.emplace_back(std::move(output));
    output_count_ = outputs_.size() + change_.size();
}

auto BitcoinTransactionBuilderPrivate::add_existing_inputs(
    alloc::Strategy alloc) noexcept(false) -> void
{
    const auto existing = db_.GetReserved(proposal_.ID(), alloc);
    const auto add_existing = [this](const auto& txo) {
        if (false == add_input(txo)) {

            throw std::runtime_error{"failed to add input"};
        }
    };
    std::ranges::for_each(existing, add_existing);
}

auto BitcoinTransactionBuilderPrivate::add_input(const UTXO& utxo) noexcept
    -> bool
{
    auto input = factory::BitcoinTransactionInput(
        chain_, utxo, std::nullopt, {}  // TODO allocator
    );

    if (false == input.IsValid()) {
        LogError()()("Failed to construct input").Flush();

        return false;
    }

    LogTrace()()("adding previous output ")(utxo.first.str())(" to transaction")
        .Flush();
    input_count_ = inputs_.size();
    input.Internal().GetBytes(input_total_, witness_total_);
    const auto amount = Amount{utxo.second.Value()};
    input_value_ += amount;
    inputs_.emplace_back(std::move(input), amount);

    return true;
}

auto BitcoinTransactionBuilderPrivate::add_output(
    const identifier::Generic& contact,
    const Amount& value,
    crypto::AddressStyle type,
    std::string_view bytes,
    std::int32_t& index) noexcept(false) -> void
{
    using enum protocol::bitcoin::base::block::script::Position;

    return add_output(
        contact,
        value,
        factory::BitcoinScript(chain_, make_script(type, bytes), Output, {}),
        index);  // TODO allocator
}

auto BitcoinTransactionBuilderPrivate::add_output(
    const identifier::Generic& contact,
    const Amount& value,
    std::string_view pubkey,
    std::int32_t& index) noexcept(false) -> void
{
    using enum protocol::bitcoin::base::block::script::Position;

    return add_output(
        contact,
        value,
        factory::BitcoinScript(
            chain_,
            [&] {
                using protocol::bitcoin::base::block::internal::Opcode;
                using protocol::bitcoin::base::block::internal::PushData;
                using enum protocol::bitcoin::base::block::script::OP;
                auto elements =
                    protocol::bitcoin::base::block::ScriptElements{};
                elements.emplace_back(PushData(pubkey));
                elements.emplace_back(Opcode(CHECKSIG));

                return elements;
            }(),
            Output,
            {}),
        index);  // TODO allocator
}

auto BitcoinTransactionBuilderPrivate::add_output(
    const identifier::Generic& contact,
    const Amount& value,
    protocol::bitcoin::base::block::Script&& script,
    std::int32_t& index) noexcept(false) -> void
{
    if (false == script.IsValid()) {

        throw std::runtime_error{"failed to construct script"};
    }

    auto& txout = outputs_.emplace_back(factory::BitcoinTransactionOutput(
        chain_,
        static_cast<std::uint32_t>(++index),
        value,
        std::move(script),
        std::nullopt,  // TODO cashtoken
        {},
        {}  // TODO allocator
        ));

    if (false == txout.IsValid()) {
        outputs_.pop_back();

        throw std::runtime_error{"failed to construct output"};
    }

    txout.Internal().SetPayer(self_contact_);

    if (false == contact.empty()) { txout.Internal().SetPayee(contact); }

    output_value_ += txout.Value();
    output_total_ += txout.Internal().CalculateSize();
}

auto BitcoinTransactionBuilderPrivate::add_signatures(
    const ReadView preimage,
    const blockchain::protocol::bitcoin::base::SigHash& sigHash,
    Input& input) const noexcept -> bool
{
    const auto reason = api_.Factory().PasswordPrompt(__func__);
    const auto& output = input.Internal().Spends();
    using enum protocol::bitcoin::base::block::script::Pattern;

    switch (output.Script().Type()) {
        case PayToWitnessPubkeyHash:
        case PayToPubkeyHash: {
            return add_signatures_p2pkh(
                preimage, sigHash, reason, output, input);
        }
        case PayToPubkey: {
            return add_signatures_p2pk(
                preimage, sigHash, reason, output, input);
        }
        case PayToMultisig: {
            return add_signatures_p2ms(
                preimage, sigHash, reason, output, input);
        }
        default: {
            LogError()()("Unsupported input type").Flush();

            return false;
        }
    }
}

auto BitcoinTransactionBuilderPrivate::add_signatures_p2ms(
    const ReadView preimage,
    const blockchain::protocol::bitcoin::base::SigHash& sigHash,
    const PasswordPrompt& reason,
    const Output& spends,
    Input& input) const noexcept -> bool
{
    const auto& script = spends.Script();

    if ((1u != script.M().value()) || (3u != script.N().value())) {
        LogError()()("Unsupported multisig pattern").Flush();

        return false;
    }

    auto keys = Vector<ByteArray>{};
    auto signatures = Vector<Space>{};
    auto views = protocol::bitcoin::base::block::internal::Input::Signatures{};
    const auto& api = api_.Crypto().Blockchain();

    for (const auto& id : input.Keys({})) {  // TODO allocator
        LogVerbose()()("Loading element ")(crypto::print(id, api_.Crypto()))(
            " to sign previous output ")(input.PreviousOutput().str())
            .Flush();
        const auto& node = api.GetKey(id);

        if (const auto got = node.KeyID(); got != id) {
            LogError()()("api::Blockchain::GetKey returned the wrong key")
                .Flush();
            LogError()()("requested: ")(crypto::print(id, api_.Crypto()))
                .Flush();
            LogError()()("      got: ")(crypto::print(got, api_.Crypto()))
                .Flush();

            LogAbort()().Abort();
        }

        const auto& key = node.PrivateKey(reason);

        assert_true(key.IsValid());

        if (key.PublicKey() != script.MultisigPubkey(0).value()) {
            LogError()()("Pubkey mismatch").Flush();

            continue;
        }

        auto& sig = signatures.emplace_back();
        sig.reserve(80);
        const auto haveSig =
            key.SignDER(preimage, hash_type(), writer(sig), reason);

        if (false == haveSig) {
            LogError()()("Failed to obtain signature").Flush();

            return false;
        }

        sig.emplace_back(sigHash.flags_);

        assert_true(0 < key.PublicKey().size());

        views.emplace_back(reader(sig), ReadView{});
    }

    if (0 == views.size()) {
        LogError()()("No keys available for signing ")(
            input.PreviousOutput().str())
            .Flush();

        return false;
    }

    if (false == input.Internal().AddMultisigSignatures(views)) {
        LogError()()("Failed to apply signature").Flush();

        return false;
    }

    return true;
}

auto BitcoinTransactionBuilderPrivate::add_signatures_p2pk(
    const ReadView preimage,
    const blockchain::protocol::bitcoin::base::SigHash& sigHash,
    const PasswordPrompt& reason,
    const Output& spends,
    Input& input) const noexcept -> bool
{
    auto keys = Vector<ByteArray>{};
    auto signatures = Vector<Space>{};
    auto views = protocol::bitcoin::base::block::internal::Input::Signatures{};
    const auto& api = api_.Crypto().Blockchain();

    for (const auto& id : input.Keys({})) {  // TODO allocator
        LogVerbose()()("Loading element ")(crypto::print(id, api_.Crypto()))(
            " to sign previous output ")(input.PreviousOutput().str())
            .Flush();
        const auto& node = api.GetKey(id);

        if (const auto got = node.KeyID(); got != id) {
            LogError()()("api::Blockchain::GetKey returned the wrong key")
                .Flush();
            LogError()()("requested: ")(crypto::print(id, api_.Crypto()))
                .Flush();
            LogError()()("      got: ")(crypto::print(got, api_.Crypto()))
                .Flush();

            LogAbort()().Abort();
        }

        const auto& pub =
            validate(Match::ByValue, node, input.PreviousOutput(), spends);

        if (false == pub.IsValid()) { continue; }

        const auto& key = get_private_key(pub, node, reason);

        if (false == key.IsValid()) { continue; }

        auto& sig = signatures.emplace_back();
        sig.reserve(80);
        const auto haveSig =
            key.SignDER(preimage, hash_type(), writer(sig), reason);

        if (false == haveSig) {
            LogError()()("Failed to obtain signature").Flush();

            return false;
        }

        sig.emplace_back(sigHash.flags_);

        assert_true(0 < key.PublicKey().size());

        views.emplace_back(reader(sig), ReadView{});
    }

    if (0 == views.size()) {
        LogError()()("No keys available for signing ")(
            input.PreviousOutput().str())
            .Flush();

        return false;
    }

    if (false == input.Internal().AddSignatures(views)) {
        LogError()()("Failed to apply signature").Flush();

        return false;
    }

    return true;
}

auto BitcoinTransactionBuilderPrivate::add_signatures_p2pkh(
    const ReadView preimage,
    const blockchain::protocol::bitcoin::base::SigHash& sigHash,
    const PasswordPrompt& reason,
    const Output& spends,
    Input& input) const noexcept -> bool
{
    auto keys = Vector<ByteArray>{};
    auto signatures = Vector<Space>{};
    auto views = protocol::bitcoin::base::block::internal::Input::Signatures{};
    const auto& api = api_.Crypto().Blockchain();

    for (const auto& id : input.Keys({})) {
        LogVerbose()()("Loading element ")(crypto::print(id, api_.Crypto()))(
            " to sign previous output ")(input.PreviousOutput().str())
            .Flush();
        const auto& node = api.GetKey(id);

        if (const auto got = node.KeyID(); got != id) {
            LogError()()("api::Blockchain::GetKey returned the wrong key")
                .Flush();
            LogError()()("requested: ")(crypto::print(id, api_.Crypto()))
                .Flush();
            LogError()()("      got: ")(crypto::print(got, api_.Crypto()))
                .Flush();

            LogAbort()().Abort();
        }

        const auto& pub =
            validate(Match::ByHash, node, input.PreviousOutput(), spends);

        if (false == pub.IsValid()) { continue; }

        const auto& key = get_private_key(pub, node, reason);

        if (false == key.IsValid()) { continue; }

        const auto& pubkey =
            keys.emplace_back(api_.Factory().DataFromBytes(key.PublicKey()));
        auto& sig = signatures.emplace_back();
        sig.reserve(80);
        const auto haveSig =
            key.SignDER(preimage, hash_type(), writer(sig), reason);

        if (false == haveSig) {
            LogError()()("Failed to obtain signature").Flush();

            return false;
        }

        sig.emplace_back(sigHash.flags_);

        assert_true(0 < key.PublicKey().size());

        views.emplace_back(reader(sig), pubkey.Bytes());
    }

    if (0 == views.size()) {
        LogError()()("No keys available for signing ")(
            input.PreviousOutput().str())
            .Flush();

        return false;
    }

    if (false == input.Internal().AddSignatures(views)) {
        LogError()()("Failed to apply signature").Flush();

        return false;
    }

    return true;
}

auto BitcoinTransactionBuilderPrivate::add_sweep_inputs(
    BuildResult& output,
    SendResult& rc) noexcept -> bool
{
    const auto& log = LogTrace();
    auto alloc = alloc::Strategy{};  // TODO

    try {
        const auto utxos = [&, this] {
            const auto get = [&, this](auto state) {
                switch (proposal_.Funding()) {
                    case SweepAccount: {

                        return node_.Wallet().GetOutputs(spender(), state);
                    }
                    case SweepSubaccount: {
                        const auto& id = proposal_.SweepFromSubaccount();

                        return node_.Wallet().GetOutputs(spender(), id, state);
                    }
                    case SweepKey: {
                        const auto& key = proposal_.SweepFromKey();

                        return node_.Wallet().GetOutputs(key, state);
                    }
                    case Default:
                    default: {
                        LogAbort()()("proposal is not a sweep transaction")
                            .Abort();
                    }
                }
            };
            constexpr auto confirmed = TxoState::ConfirmedNew;

            if (auto out = get(confirmed); false == out.empty()) {

                return out;
            } else if (proposal_.SpendUnconfirmedIncoming()) {
                constexpr auto unconfirmed = TxoState::UnconfirmedNew;

                return get(unconfirmed);
            } else {

                return out;
            }
        }();

        for (const auto& [outpoint, _] : utxos) {
            auto utxo = db_.ReserveUTXO(log, spender(), id_, outpoint, alloc);

            if (utxo.has_value()) {
                if (false == add_input(utxo.value())) {
                    output = PermanentFailure;
                    rc = InputCreationError;

                    throw std::runtime_error{"failed to add input"};
                }
            } else {
                LogError()()("unable to reserve ")(outpoint)(" for sweep")
                    .Flush();
            }
        }

        if (inputs_.empty() || (0 == input_value_)) {
            output = PermanentFailure;
            rc = InsufficientFunds;

            throw std::runtime_error{"no value to sweep"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto BitcoinTransactionBuilderPrivate::bip_69() noexcept -> void
{
    auto inputSort = [](const auto& lhs, const auto& rhs) -> auto {
        return lhs.first.PreviousOutput() < rhs.first.PreviousOutput();
    };
    auto outputSort = [](const auto& lhs, const auto& rhs) -> auto {
        if (lhs.Value() == rhs.Value()) {
            auto lScript = Space{};
            auto rScript = Space{};
            lhs.Script().Serialize(writer(lScript));
            rhs.Script().Serialize(writer(rScript));

            return std::lexicographical_compare(
                std::begin(lScript),
                std::end(lScript),
                std::begin(rScript),
                std::end(rScript));
        } else {

            return lhs.Value() < rhs.Value();
        }
    };

    std::ranges::sort(inputs_, inputSort);
    std::ranges::sort(outputs_, outputSort);
    auto index{-1};

    for (auto& output : outputs_) { output.Internal().SetIndex(++index); }
}

auto BitcoinTransactionBuilderPrivate::BuildNormalTransaction() noexcept
    -> BuildResult
{
    const auto& log = LogTrace();
    const auto& crypto = api_.Crypto();
    auto alloc = alloc::Strategy{};  // TODO
    auto output = Success;
    auto rc = UnspecifiedError;
    auto txid = block::TransactionHash{};
    auto post = ScopeGuard{[&] {
        switch (output) {
            case TemporaryFailure: {
            } break;
            case PermanentFailure: {
                release_keys();
                [[fallthrough]];
            }
            case Success:
            default: {

                promise_.set_value({rc, txid});
            }
        }
    }};

    if (false == create_outputs()) {
        LogError()()("Failed to create outputs").Flush();
        output = PermanentFailure;
        rc = OutputCreationError;

        return output;
    }

    if (false == create_notifications(2u * dust())) {
        LogError()()("Failed to create notifications").Flush();
        output = PermanentFailure;
        rc = OutputCreationError;

        return output;
    }

    if (false == add_change()) {
        LogError()()("Failed to allocate change output").Flush();
        output = PermanentFailure;
        rc = ChangeError;

        return output;
    }

    while (false == is_funded()) {
        using node::internal::SpendPolicy;
        const auto policy = proposal_.Internal().Policy();
        log()("asking database for outputs to fund proposal ")(id_, crypto)
            .Flush();
        auto candidate = db_.ReserveUTXO(log, spender(), id_, policy, alloc);
        auto [utxo, haveMore] = candidate;

        if (false == utxo.has_value()) {
            if (haveMore) {
                LogError()()("Insufficient confirmed funds").Flush();
                output = PermanentFailure;
                rc = InsufficientConfirmedFunds;
            } else {
                LogError()()("Insufficient funds").Flush();
                output = PermanentFailure;
                rc = InsufficientFunds;
            }

            return output;
        }

        if (false == add_input(utxo.value())) {
            LogError()()("Failed to add input").Flush();
            output = PermanentFailure;
            rc = InputCreationError;

            return output;
        }
    }

    assert_true(is_funded());

    finalize_outputs();

    return finalize(rc, txid);
}

auto BitcoinTransactionBuilderPrivate::BuildSweepTransaction() noexcept
    -> BuildResult
{
    auto output = Success;
    auto rc = UnspecifiedError;
    auto txid = block::TransactionHash{};
    auto post = ScopeGuard{[&] {
        switch (output) {
            case TemporaryFailure: {
            } break;
            case PermanentFailure: {
                release_keys();
                [[fallthrough]];
            }
            case Success:
            default: {

                promise_.set_value({rc, txid});
            }
        }
    }};

    if (false == add_sweep_inputs(output, rc)) { return output; }

    if (has_notification()) {
        assert_false(has_output());

        if (false == create_notifications(0u)) {
            LogError()()("Failed to create notifications").Flush();
            output = PermanentFailure;
            rc = OutputCreationError;

            return output;
        }

        if (false == is_funded()) {
            LogError()()("Insufficient funds").Flush();
            output = PermanentFailure;
            rc = InsufficientFunds;

            return output;
        }

        finalize_outputs();
    } else {
        if (has_output()) {
            if (false == create_outputs()) {
                LogError()()("Failed to create destination output").Flush();
                output = PermanentFailure;
                rc = OutputCreationError;

                return output;
            }
        } else {
            if (false == add_change()) {
                LogError()()("Failed to allocate change output").Flush();
                output = PermanentFailure;
                rc = ChangeError;

                return output;
            }

            std::ranges::move(change_, std::back_inserter(outputs_));
            change_.clear();
        }

        const auto amount = excess_value();

        if (amount < dust()) {
            LogError()()("Insufficient funds").Flush();
            output = PermanentFailure;
            rc = InsufficientFunds;

            return output;
        }

        assert_true(1_uz == outputs_.size());

        auto& txout = outputs_[0];
        txout.Internal().SetValue(amount);
        output_value_ += txout.Value();
    }

    return finalize(rc, txid);
}

auto BitcoinTransactionBuilderPrivate::bytes() const noexcept -> std::size_t
{
    // NOTE assumes one additional output to account for change
    const auto outputs =
        network::blockchain::bitcoin::CompactSize{output_count_.Value() + 1};
    const auto base = fixed_overhead_ + input_count_.Size() + input_total_ +
                      outputs.Size() + output_total_ + p2pkh_output_bytes_;

    if (false == segwit_) { return base; }

    static constexpr auto markerBytes = 2_uz;
    const auto segwit = markerBytes + witness_total_;
    const auto total = base + segwit;
    const auto scale = params::get(chain_).SegwitScaleFactor();

    assert_true(0 < scale);

    const auto factor = scale - 1u;
    // TODO check for std::size_t overflow?
    const auto wu = (segwit * factor) + total;
    static constexpr auto ceil = [](const auto a, const auto b) {
        return (a + (b - 1u)) / b;
    };

    return ceil(wu, scale);
}

auto BitcoinTransactionBuilderPrivate::create_outputs() noexcept -> bool
{
    try {
        auto index = std::int32_t{-1};
        const auto make_addr = [&, this](const auto& data) {
            const auto& [contact, value, type, bytes] = data;
            add_output(contact, value, type, bytes.Bytes(), index);
        };
        const auto make_pc = [&, this](const auto& data) {
            const auto& [contact, value, recipient, keyID, pubkey] = data;
            add_output(contact, value, pubkey.Bytes(), index);
        };
        const auto addr = proposal_.Internal().AddressRecipients();
        const auto pc = proposal_.Internal().PaymentCodeRecipients();
        std::ranges::for_each(addr, make_addr);
        std::ranges::for_each(pc, make_pc);
        output_count_ = outputs_.size() + change_.size();

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto BitcoinTransactionBuilderPrivate::create_notifications(
    std::uint64_t value) noexcept -> bool
{
    try {
        const auto reason = api_.Factory().PasswordPrompt(
            proposal_.Internal().PasswordPrompt());

        for (const auto& recipient : proposal_.Notifications()) {
            const auto& element = next_change_element(reason);
            make_change_output(
                element,
                [&, this] {
                    return make_notification(element, recipient, reason);
                },
                TxoTag::Notification);
            notification_value_ += value;
        }

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto BitcoinTransactionBuilderPrivate::distribute_change_amounts() noexcept
    -> void
{
    const auto count = change_.size();

    assert_true(0_uz < count);

    auto remaining{excess_value().Internal().ExtractUInt64()};
    const auto share = remaining / count;

    assert_true(share >= dust());

    for (auto& change : change_) {
        change.Internal().SetValue(share);
        output_value_ += change.Value();
        remaining -= share;
    }

    if (0 < remaining) {
        constexpr auto satoshi = 1u;

        for (auto& change : change_) {
            change.Internal().SetValue(change.Value() + satoshi);
            output_value_ += satoshi;
            remaining -= satoshi;

            if (0 >= remaining) { break; }
        }
    }

    assert_true(0 == remaining);

    std::ranges::move(change_, std::back_inserter(outputs_));
    change_.clear();
}

auto BitcoinTransactionBuilderPrivate::drop_unnecessary_change() noexcept
    -> void
{
    constexpr auto can_drop_last = [](auto& change) {
        if (change.empty()) { return false; }

        const auto& last = change.back().Internal();

        return false == last.Tags().contains(TxoTag::Notification);
    };

    while (can_drop_last(change_)) {
        const auto& api = api_.Crypto().Blockchain();
        const auto& last = change_.back().Internal();

        for (const auto& key : last.Keys(alloc::Default{})) {
            api.Release(key);
            change_keys_.erase(key);
        }

        output_value_ -= last.Value();
        output_total_ -= last.CalculateSize();
        change_.pop_back();
        output_count_ = outputs_.size() + change_.size();
    }
}

auto BitcoinTransactionBuilderPrivate::dust() const noexcept -> std::uint64_t
{
    // TODO this should account for script type
    const auto amount = 148 * fee_rate_ / 1000;
    auto dust = std::uint64_t{};

    try {
        dust = amount.Internal().ExtractUInt64();
    } catch (const std::exception& e) {
        LogError()()("error calculating dust: ")(e.what()).Flush();
    }

    return dust;
}

auto BitcoinTransactionBuilderPrivate::excess_value() const noexcept -> Amount
{
    return input_value_ - (output_value_ + required_fee());
}

auto BitcoinTransactionBuilderPrivate::finalize(
    SendResult& rc,
    block::TransactionHash& txid) noexcept -> BuildResult
{
    const auto& log = LogTrace();
    auto alloc = alloc::Strategy{};  // TODO

    try {
        if (false == sign_inputs()) {
            rc = SignatureError;

            throw std::runtime_error{"transaction signing failure"};
        }

        auto transaction = finalize_transaction();

        if (false == transaction.IsValid()) {
            rc = UnspecifiedError;

            throw std::runtime_error{"failed to instantiate transaction"};
        }

        proposal_.Internal().Add([&] {
            const auto proto =
                transaction.Internal().asBitcoin().Serialize(api_);

            if (false == proto.has_value()) {
                rc = SerializationError;

                throw std::runtime_error{"failed to serialize transaction"};
            }

            return *proto;
        }());
        const auto proto = [&] {
            auto out = proto::BlockchainTransactionProposal{};

            if (false == proposal_.Internal().Serialize(out)) {
                rc = SerializationError;

                throw std::runtime_error{
                    "failed to serialize finished proposal"};
            }

            return out;
        }();

        if (!db_.FinalizeProposal(log, id_, proto, transaction, alloc)) {
            rc = DatabaseError;

            throw std::runtime_error{"database error finalizing proposal"};
        }

        txid = transaction.ID();
        const auto sent =
            node_.Internal().BroadcastTransaction(transaction, true);

        if (sent) {
            auto bytes = api_.Factory().Data();
            transaction.Internal().asBitcoin().Serialize(bytes.WriteInto());
            LogConsole()("Broadcasting ")(blockchain::print(chain_))(
                " transaction ")
                .asHex(txid)
                .Flush();
            LogConsole().asHex(bytes).Flush();
            log(transaction.Print(api_.Crypto())).Flush();
        } else {
            rc = SendFailed;

            throw std::runtime_error{"Failed to send tx"};
        }

        rc = Sent;
        proposal_.Internal().AddNotification(transaction.ID());

        return Success;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return PermanentFailure;
    }
}

auto BitcoinTransactionBuilderPrivate::finalize_outputs() noexcept -> void
{
    assert_true(is_funded());
    assert_false(change_.empty());

    if (excess_value() < dust()) { drop_unnecessary_change(); }

    if (false == change_.empty()) { distribute_change_amounts(); }

    bip_69();
}

auto BitcoinTransactionBuilderPrivate::finalize_transaction() noexcept
    -> Transaction
{
    auto inputs = [&] {
        auto output = Vector<Input>{};  // TODO allocator
        output.reserve(inputs_.size());
        output.clear();
        std::ranges::transform(
            inputs_, std::back_inserter(output), [](auto& input) -> auto {
                return std::move(input.first);
            });

        return output;
    }();

    return factory::BitcoinTransaction(
        api_.Crypto(),
        chain_,
        Clock::now(),
        version_,
        lock_time_,
        segwit_,
        std::move(inputs),
        std::move(outputs_),
        {}  // TODO allocator
    );
}

auto BitcoinTransactionBuilderPrivate::get_private_key(
    const opentxs::crypto::asymmetric::key::EllipticCurve& pubkey,
    const blockchain::crypto::Element& element,
    const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    const auto& key = element.PrivateKey(reason);

    if (false == key.IsValid()) {
        LogError()()("failed to obtain private key ")(
            crypto::print(element.KeyID(), api_.Crypto()))
            .Flush();

        return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
    }

    assert_true(key.HasPrivate());

    if (key.PublicKey() != pubkey.PublicKey()) {
        const auto got = api_.Factory().DataFromBytes(key.PublicKey());
        const auto expected = api_.Factory().DataFromBytes(pubkey.PublicKey());
        const auto [account, subchain, index] = element.KeyID();
        LogAbort()()("Derived private key for "
                     "account ")(account, api_.Crypto())(" subchain ")(
            static_cast<std::uint32_t>(subchain))(" index ")(
            index)(" does not correspond to the expected public key. Got ")
            .asHex(got)(" expected ")
            .asHex(expected)
            .Abort();
    }

    return key;
}

auto BitcoinTransactionBuilderPrivate::has_notification() const noexcept -> bool
{
    return false == proposal_.Notifications().empty();
}

auto BitcoinTransactionBuilderPrivate::has_output() const noexcept -> bool
{
    return false == proposal_.Internal().AddressRecipients().empty();
}

auto BitcoinTransactionBuilderPrivate::hash_type() const noexcept
    -> opentxs::crypto::HashType
{
    return opentxs::crypto::HashType::Sha256D;
}

auto BitcoinTransactionBuilderPrivate::init_bip143(
    Bip143& bip143) const noexcept -> bool
{
    if (bip143.has_value()) { return true; }

    auto success{false};
    const auto postcondition = ScopeGuard{[&]() {
        if (false == success) { bip143 = std::nullopt; }
    }};
    bip143.emplace();

    assert_true(bip143.has_value());

    auto& hashes = bip143.value();
    auto cb = [&](const auto& preimage, auto& dest) -> bool {
        return api_.Crypto().Hash().Digest(
            opentxs::crypto::HashType::Sha256D,
            reader(preimage),
            preallocated(dest.size(), dest.data()));
    };

    {
        auto preimage = space(inputs_.size() * sizeof(block::Outpoint));
        auto* it = preimage.data();

        for (const auto& [input, amount] : inputs_) {
            const auto& outpoint = input.PreviousOutput();
            std::memcpy(it, &outpoint, sizeof(outpoint));
            std::advance(it, sizeof(outpoint));
        }

        if (false == cb(preimage, hashes.outpoints_)) {
            LogError()()("Failed to hash outpoints").Flush();

            return false;
        }
    }

    {
        auto preimage = space(inputs_.size() * sizeof(std::uint32_t));
        auto* it = preimage.data();

        for (const auto& [input, value] : inputs_) {
            const auto sequence = input.Sequence();
            std::memcpy(it, &sequence, sizeof(sequence));
            std::advance(it, sizeof(sequence));
        }

        if (false == cb(preimage, hashes.sequences_)) {
            LogError()()("Failed to hash sequences").Flush();

            return false;
        }
    }

    {
        auto preimage = space(output_total_);
        auto* it = preimage.data();

        for (const auto& output : outputs_) {
            const auto& internal = output.Internal();
            const auto size = internal.CalculateSize();

            if (!internal.Serialize(preallocated(size, it)).has_value()) {
                LogError()()("Failed to serialize output").Flush();

                return false;
            }

            std::advance(it, size);
        }

        if (false == cb(preimage, hashes.outputs_)) {
            LogError()()("Failed to hash outputs").Flush();

            return false;
        }
    }

    success = true;

    return true;
}

auto BitcoinTransactionBuilderPrivate::init_txcopy(
    Transaction& txcopy) const noexcept -> bool
{
    if (txcopy.IsValid()) { return true; }

    auto inputs = Vector<Input>{};  // TODO allocator
    inputs.reserve(inputs_.size());
    inputs.clear();
    std::ranges::transform(
        inputs_,
        std::back_inserter(inputs),
        [](const auto& input) -> auto {  // TODO allocator
            return input.first.Internal().SignatureVersion({});
        });
    auto outputs = Vector<Output>{};  // TODO allocator
    outputs.reserve(outputs_.size());
    outputs.clear();
    std::ranges::copy(outputs_, std::back_inserter(outputs));

    txcopy = factory::BitcoinTransaction(
        api_.Crypto(),
        chain_,
        Clock::now(),
        version_,
        lock_time_,
        false,
        inputs,
        outputs,
        {}  // TODO allocator
    );

    return txcopy.IsValid();
}

auto BitcoinTransactionBuilderPrivate::is_funded() const noexcept -> bool
{
    return input_value_ >=
           (output_value_ + notification_value_ + required_fee());
}

auto BitcoinTransactionBuilderPrivate::is_segwit(const Input& input) noexcept
    -> bool
{
    using enum protocol::bitcoin::base::block::script::Pattern;

    switch (input.Internal().Spends().Script().Type()) {
        case PayToWitnessPubkeyHash:
        case PayToWitnessScriptHash:
        case PayToTaproot: {

            return true;
        }
        default: {

            return false;
        }
    }
}

auto BitcoinTransactionBuilderPrivate::make_notification(
    const crypto::Element& element,
    const PaymentCode& recipient,
    const PasswordPrompt& reason) const noexcept(false)
    -> protocol::bitcoin::base::block::ScriptElements
{
    const auto& key = element.PrivateKey(reason);

    if (false == key.IsValid()) {

        throw std::runtime_error{"Failed to load private change key"};
    }

    const auto keys =
        sender_->PaymentCodeSecret(reason).GenerateNotificationElements(
            recipient, key, reason);
    using enum protocol::bitcoin::base::block::script::OP;
    using protocol::bitcoin::base::block::internal::Opcode;
    using protocol::bitcoin::base::block::internal::PushData;
    auto out = protocol::bitcoin::base::block::ScriptElements{};

    if (3_uz != keys.size()) {

        throw std::runtime_error{"Failed to obtain notification elements"};
    }

    out.emplace_back(Opcode(ONE));
    out.emplace_back(PushData(reader(keys.at(0))));
    out.emplace_back(PushData(reader(keys.at(1))));
    out.emplace_back(PushData(reader(keys.at(2))));
    out.emplace_back(Opcode(THREE));
    out.emplace_back(Opcode(CHECKMULTISIG));

    return out;
}

auto BitcoinTransactionBuilderPrivate::make_p2pkh_change(
    const crypto::Element& element) const noexcept(false)
    -> protocol::bitcoin::base::block::ScriptElements
{
    const auto pkh = element.PubkeyHash();
    using enum crypto::AddressStyle;

    return make_script(p2pkh, pkh.Bytes());
}

auto BitcoinTransactionBuilderPrivate::make_script(
    crypto::AddressStyle type,
    std::string_view bytes) const noexcept(false)
    -> protocol::bitcoin::base::block::ScriptElements
{
    using protocol::bitcoin::base::block::internal::Opcode;
    using protocol::bitcoin::base::block::internal::PushData;
    using protocol::bitcoin::base::block::script::OP;
    using enum protocol::bitcoin::base::block::script::OP;
    using enum crypto::AddressStyle;
    auto elements = protocol::bitcoin::base::block::ScriptElements{};

    switch (type) {
        case p2pkh: {
            elements.emplace_back(Opcode(DUP));
            elements.emplace_back(Opcode(HASH160));
            elements.emplace_back(PushData(bytes));
            elements.emplace_back(Opcode(EQUALVERIFY));
            elements.emplace_back(Opcode(CHECKSIG));
        } break;
        case p2wpkh: {
            elements.emplace_back(Opcode(ZERO));
            elements.emplace_back(PushData(bytes));
        } break;
        case p2sh: {
            elements.emplace_back(Opcode(HASH160));
            elements.emplace_back(PushData(bytes));
            elements.emplace_back(Opcode(EQUAL));
        } break;
        case p2wsh: {
            elements.emplace_back(Opcode(ZERO));
            elements.emplace_back(PushData(bytes));
        } break;
        case p2tr: {
            elements.emplace_back(Opcode(ONE));
            elements.emplace_back(PushData(bytes));
        } break;
        case unknown_address_style:
        case ethereum_account:
        default: {

            throw std::runtime_error{
                "unsupported address type: "s.append(crypto::print(type))};
        }
    }

    return elements;
}

auto BitcoinTransactionBuilderPrivate::next_change_element(
    const PasswordPrompt& reason) noexcept(false) -> const crypto::Element&
{
    const auto& account =
        api_.Crypto().Blockchain().Account(sender_->ID(), chain_);
    const auto& element = account.GetNextChangeKey(reason);
    change_keys_.emplace(element.KeyID());

    return element;
}

auto BitcoinTransactionBuilderPrivate::operator()() noexcept -> BuildResult
{
    auto alloc = alloc::Strategy{};  // TODO

    try {
        add_existing_inputs(alloc);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
        promise_.set_value({InputCreationError, {}});

        return PermanentFailure;
    }

    if (Default == proposal_.Funding()) {

        return BuildNormalTransaction();
    } else {

        return BuildSweepTransaction();
    }
}

auto BitcoinTransactionBuilderPrivate::print() const noexcept
    -> UnallocatedCString
{
    auto text = std::stringstream{};
    text << "\n     version: " << std::to_string(version_.value()) << '\n';
    text << "   lock time: " << std::to_string(lock_time_.value()) << '\n';
    text << " input count: " << std::to_string(inputs_.size()) << '\n';

    const auto& definition = blockchain::GetDefinition(chain_);
    for (const auto& [input, value] : inputs_) {
        const auto& outpoint = input.PreviousOutput();
        text << " * " << outpoint.str()
             << ", sequence: " << std::to_string(input.Sequence())
             << ", value: " << definition.Format(value) << '\n';
    }

    text << "output count: " << std::to_string(outputs_.size()) << '\n';

    for (const auto& output : outputs_) {
        text << " * bytes: "
             << std::to_string(output.Internal().CalculateSize())
             << ", value: " << definition.Format(output.Value()) << '\n';
    }

    const auto fee = input_value_ - output_value_;
    text << "total output value: " << definition.Format(output_value_) << '\n';
    text << " total input value: " << definition.Format(input_value_) << '\n';
    text << "               fee: " << definition.Format(fee);

    return text.str();
}

auto BitcoinTransactionBuilderPrivate::release_keys() noexcept -> void
{
    const auto& api = api_.Crypto().Blockchain();

    for (const auto& key : outgoing_keys_) { api.Release(key); }

    for (const auto& key : change_keys_) { api.Release(key); }
}

auto BitcoinTransactionBuilderPrivate::required_fee() const noexcept -> Amount
{
    return (bytes() * fee_rate_) / 1000;
}

auto BitcoinTransactionBuilderPrivate::sign_input(
    const int index,
    Input& input,
    Transaction& txcopy,
    Bip143& bip143) const noexcept -> bool
{
    using enum Category;
    using enum Type;

    switch (category(chain_)) {
        case output_based: {
            static constexpr auto bch = BitcoinCash;

            if (is_descended_from(associated_mainnet(chain_), bch)) {

                return sign_input_bch(index, input, bip143);
            } else if (is_segwit(input)) {

                return sign_input_segwit(index, input, bip143);
            } else {

                return sign_input_btc(index, input, txcopy);
            }
        }
        case unknown_category:
        case balance_based:
        default: {
            LogError()()("Unsupported chain: ")(blockchain::print(chain_))
                .Flush();

            return false;
        }
    }
}

auto BitcoinTransactionBuilderPrivate::sign_input_bch(
    const int index,
    Input& input,
    Bip143& bip143) const noexcept -> bool
{
    if (false == init_bip143(bip143)) {
        LogError()()("Error instantiating bip143").Flush();

        return false;
    }

    const auto sigHash = blockchain::protocol::bitcoin::base::SigHash{chain_};
    const auto preimage = bip143->Preimage(
        index, outputs_.size(), version_, lock_time_, sigHash, input);

    return add_signatures(preimage.Bytes(), sigHash, input);
}

auto BitcoinTransactionBuilderPrivate::sign_input_btc(
    const int index,
    Input& input,
    Transaction& txcopy) const noexcept -> bool
{
    if (false == init_txcopy(txcopy)) {
        LogError()()("Error instantiating txcopy").Flush();

        return false;
    }

    const auto sigHash = blockchain::protocol::bitcoin::base::SigHash{chain_};
    auto preimage =
        txcopy.Internal().asBitcoin().GetPreimageBTC(index, sigHash);

    if (0 == preimage.size()) {
        LogError()()("Error obtaining signing preimage").Flush();

        return false;
    }

    std::ranges::copy(sigHash, std::back_inserter(preimage));

    return add_signatures(reader(preimage), sigHash, input);
}

auto BitcoinTransactionBuilderPrivate::sign_input_segwit(
    const int index,
    Input& input,
    Bip143& bip143) const noexcept -> bool
{
    if (false == init_bip143(bip143)) {
        LogError()()("Error instantiating bip143").Flush();

        return false;
    }

    segwit_ = true;
    const auto sigHash = blockchain::protocol::bitcoin::base::SigHash{chain_};
    const auto preimage = bip143->Preimage(
        index, outputs_.size(), version_, lock_time_, sigHash, input);

    return add_signatures(preimage.Bytes(), sigHash, input);
}

auto BitcoinTransactionBuilderPrivate::sign_inputs() noexcept -> bool
{
    auto index = int{-1};
    auto txcopy = Transaction{};
    auto bip143 = std::optional<protocol::bitcoin::base::Bip143Hashes>{};

    for (auto& [input, value] : inputs_) {
        if (false == sign_input(++index, input, txcopy, bip143)) {
            LogError()()("Failed to sign input ")(index).Flush();

            return false;
        }
    }

    return true;
}

auto BitcoinTransactionBuilderPrivate::spender() const noexcept
    -> const identifier::Nym&
{
    return sender_->ID();
}

auto BitcoinTransactionBuilderPrivate::validate(
    const Match match,
    const blockchain::crypto::Element& element,
    const block::Outpoint& outpoint,
    const Output& output) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    const auto [account, subchain, index] = element.KeyID();
    LogTrace()()("considering spend key ")(index)(" from subchain ")(
        static_cast<std::uint32_t>(subchain))(" of account ")(
        account, api_.Crypto())(" for previous output ")(outpoint.str())
        .Flush();

    const auto& key = element.Key();

    if (false == key.IsValid()) {
        LogError()()("missing public key").Flush();

        return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
    }

    if (Match::ByValue == match) {
        const auto expected = output.Script().Pubkey();

        if (false == expected.has_value()) {
            LogError()()("wrong output script type").Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }

        if (key.PublicKey() != expected.value()) {
            LogError()()("Provided public key does not match expected value")
                .Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }
    } else {
        const auto expected = output.Script().PubkeyHash();

        if (false == expected.has_value()) {
            LogError()()("wrong output script type").Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }

        if (element.PubkeyHash().Bytes() != expected.value()) {
            LogError()()("Provided public key does not match expected hash")
                .Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }
    }

    return key;
}
}  // namespace opentxs::blockchain::node::wallet
