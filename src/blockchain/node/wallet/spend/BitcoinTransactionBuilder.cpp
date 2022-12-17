// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::OP
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::script::Pattern
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::script::Position
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoTag
// IWYU pragma: no_forward_declare opentxs::crypto::HashType

#include "blockchain/node/wallet/spend/BitcoinTransactionBuilder.hpp"  // IWYU pragma: associated

#include <BlockchainOutputMultisigDetails.pb.h>
#include <BlockchainTransactionOutput.pb.h>
#include <BlockchainTransactionProposedNotification.pb.h>
#include <BlockchainTransactionProposedOutput.pb.h>
#include <HDPath.pb.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Input.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/core/Amount.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Opcodes.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Pattern.hpp"   // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"

namespace be = boost::endian;

namespace opentxs::blockchain::node::wallet
{
struct BitcoinTransactionBuilder::Imp {
    auto IsFunded() const noexcept -> bool
    {
        return input_value_ > (output_value_ + required_fee());
    }
    auto Spender() const noexcept -> const identifier::Nym&
    {
        return sender_->ID();
    }

    auto AddChange(const Proposal& data) noexcept -> bool
    {
        try {
            const auto reason = api_.Factory().PasswordPrompt(__func__);
            const auto& account =
                api_.Crypto().Blockchain().Account(sender_->ID(), chain_);
            const auto& element = account.GetNextChangeKey(reason);
            const auto keyID = element.KeyID();
            change_keys_.emplace(keyID);
            auto isNotification{false};
            auto output = [&] {
                auto elements = [&] {
                    namespace bb = opentxs::blockchain::bitcoin::block;
                    namespace bi = bb::internal;
                    auto out = bb::ScriptElements{};

                    if (const auto size{data.notification().size()}; 1 < size) {
                        throw std::runtime_error{
                            "Multiple notifications not yet supported"};
                    } else if (1 == size) {
                        const auto& notif = data.notification(0);
                        const auto recipient =
                            api_.Factory().InternalSession().PaymentCode(
                                notif.recipient());
                        const auto message =
                            UnallocatedCString{
                                "Constructing notification transaction to "} +
                            recipient.asBase58();
                        const auto mreason =
                            api_.Factory().PasswordPrompt(message);
                        const auto pc = [&] {
                            auto code =
                                api_.Factory().InternalSession().PaymentCode(
                                    notif.sender());
                            const auto& path = notif.path();
                            auto seed{path.root()};
                            const auto rc = code.Internal().AddPrivateKeys(
                                seed, *path.child().rbegin(), mreason);

                            if (false == rc) {
                                throw std::runtime_error{
                                    "Failed to load private keys"};
                            }

                            return code;
                        }();
                        const auto& key = element.PrivateKey(reason);

                        if (false == key.IsValid()) {
                            throw std::runtime_error{
                                "Failed to load private change key"};
                        }

                        const auto keys = pc.GenerateNotificationElements(
                            recipient, key, reason);

                        if (3u != keys.size()) {
                            throw std::runtime_error{
                                "Failed to obtain notification elements"};
                        }

                        using enum bitcoin::block::script::OP;
                        out.emplace_back(bi::Opcode(ONE));
                        out.emplace_back(bi::PushData(reader(keys.at(0))));
                        out.emplace_back(bi::PushData(reader(keys.at(1))));
                        out.emplace_back(bi::PushData(reader(keys.at(2))));
                        out.emplace_back(bi::Opcode(THREE));
                        out.emplace_back(bi::Opcode(CHECKMULTISIG));
                        isNotification = true;
                    } else {
                        const auto pkh = element.PubkeyHash();
                        using enum bitcoin::block::script::OP;
                        out.emplace_back(bi::Opcode(DUP));
                        out.emplace_back(bi::Opcode(HASH160));
                        out.emplace_back(bi::PushData(pkh.Bytes()));
                        out.emplace_back(bi::Opcode(EQUALVERIFY));
                        out.emplace_back(bi::Opcode(CHECKSIG));
                    }

                    return out;
                }();
                using enum bitcoin::block::script::Position;
                // TODO allocator
                auto script =
                    factory::BitcoinScript(chain_, elements, Output, {});

                if (false == script.IsValid()) {
                    throw std::runtime_error{"Failed to construct script"};
                }

                return factory::BitcoinTransactionOutput(
                    chain_,
                    shorten(outputs_.size()),
                    Amount{0},
                    std::move(script),
                    {keyID},
                    {}  // TODO allocator
                );
            }();

            if (false == output.IsValid()) {
                throw std::runtime_error{"Failed to construct output"};
            }

            auto& internal = output.Internal();

            if (isNotification) { internal.AddTag(TxoTag::Notification); }

            {
                output_value_ += internal.Value();
                output_total_ += internal.CalculateSize();

                OT_ASSERT(0 < output.Keys({}).size());  // TODO allocator

                internal.SetPayee(self_contact_);
                internal.SetPayer(self_contact_);
                internal.AddTag(TxoTag::Change);

                if (isNotification) { internal.AddTag(TxoTag::Notification); }
            }

            change_.emplace_back(std::move(output));
            output_count_ = outputs_.size() + change_.size();

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    auto AddInput(const UTXO& utxo) noexcept -> bool
    {
        auto input = factory::BitcoinTransactionInput(
            chain_, utxo, std::nullopt, {}  // TODO allocator
        );

        if (false == input.IsValid()) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct input").Flush();

            return false;
        }

        LogTrace()(OT_PRETTY_CLASS())("adding previous output ")(
            utxo.first.str())(" to transaction")
            .Flush();
        input_count_ = inputs_.size();
        input.Internal().GetBytes(input_total_, witness_total_);
        const auto amount = Amount{utxo.second.Value()};
        input_value_ += amount;
        inputs_.emplace_back(std::move(input), amount);

        return true;
    }
    auto CreateOutputs(const Proposal& proposal) noexcept -> bool
    {
        namespace bb = opentxs::blockchain::bitcoin::block;
        namespace bi = bb::internal;

        auto index = std::int32_t{-1};

        for (const auto& output : proposal.output()) {
            // TODO allocator
            auto script = bb::Script{};
            using enum bitcoin::block::script::Position;

            if (output.has_raw()) {
                // TODO allocator
                script = factory::BitcoinScript(
                    chain_, output.raw(), Output, true, false, {});
            } else {
                auto elements = bb::ScriptElements{};
                using enum bitcoin::block::script::OP;

                if (output.has_pubkeyhash()) {
                    if (output.segwit()) {  // P2WPKH
                        elements.emplace_back(bi::Opcode(ZERO));
                        elements.emplace_back(
                            bi::PushData(output.pubkeyhash()));
                    } else {  // P2PKH
                        elements.emplace_back(bi::Opcode(DUP));
                        elements.emplace_back(bi::Opcode(HASH160));
                        elements.emplace_back(
                            bi::PushData(output.pubkeyhash()));
                        elements.emplace_back(bi::Opcode(EQUALVERIFY));
                        elements.emplace_back(bi::Opcode(CHECKSIG));
                    }
                } else if (output.has_scripthash()) {
                    if (output.segwit()) {  // P2WSH
                        elements.emplace_back(bi::Opcode(ZERO));
                        elements.emplace_back(
                            bi::PushData(output.scripthash()));
                    } else {  // P2SH
                        elements.emplace_back(bi::Opcode(HASH160));
                        elements.emplace_back(
                            bi::PushData(output.scripthash()));
                        elements.emplace_back(bi::Opcode(EQUAL));
                    }
                } else if (output.has_pubkey()) {
                    if (output.segwit()) {  // P2TR
                        elements.emplace_back(bi::Opcode(ONE));
                        elements.emplace_back(bi::PushData(output.pubkey()));
                    } else {  // P2PK
                        elements.emplace_back(bi::PushData(output.pubkey()));
                        elements.emplace_back(bi::Opcode(CHECKSIG));
                    }
                } else if (output.has_multisig()) {  // P2MS
                    const auto& ms = output.multisig();
                    const auto M = static_cast<std::uint8_t>(ms.m());
                    const auto N = static_cast<std::uint8_t>(ms.n());
                    elements.emplace_back(
                        bi::Opcode(static_cast<bb::script::OP>(M + 80)));

                    for (const auto& key : ms.pubkey()) {
                        elements.emplace_back(bi::PushData(key));
                    }

                    elements.emplace_back(
                        bi::Opcode(static_cast<bb::script::OP>(N + 80)));
                    elements.emplace_back(bi::Opcode(CHECKMULTISIG));
                } else {
                    LogError()(OT_PRETTY_CLASS())("Unsupported output type")
                        .Flush();

                    return false;
                }

                // TODO allocator
                script = factory::BitcoinScript(chain_, elements, Output, {});
            }

            if (false == script.IsValid()) {
                LogError()(OT_PRETTY_CLASS())("Failed to construct script")
                    .Flush();

                return false;
            }

            auto txout = factory::BitcoinTransactionOutput(
                chain_,
                static_cast<std::uint32_t>(++index),
                factory::Amount(output.amount()),
                std::move(script),
                {},
                {}  // TODO allocator
            );

            if (false == txout.IsValid()) {
                LogError()(OT_PRETTY_CLASS())("Failed to construct output")
                    .Flush();

                return false;
            }

            txout.Internal().SetPayer(self_contact_);

            if (output.has_contact()) {
                const auto contactID = [&] {
                    auto out = identifier::Generic{};
                    out.Assign(
                        output.contact().data(), output.contact().size());

                    return out;
                }();
                txout.Internal().SetPayee(contactID);
            }

            output_value_ += txout.Value();
            output_total_ += txout.Internal().CalculateSize();
            outputs_.emplace_back(std::move(txout));
        }

        output_count_ = outputs_.size() + change_.size();

        return true;
    }
    auto FinalizeOutputs() noexcept -> void
    {
        OT_ASSERT(IsFunded());

        const auto excessValue =
            input_value_ - (output_value_ + required_fee());
        const auto& api = api_.Crypto().Blockchain();

        if (excessValue <= dust()) {
            for (const auto& key : change_keys_) { api.Release(key); }
        } else {
            OT_ASSERT(1 == change_.size());  // TODO

            auto& change = *change_.begin();
            change.Internal().SetValue(excessValue);
            output_value_ += change.Value();
            outputs_.emplace_back(std::move(change));
        }

        change_.clear();
        bip_69();
    }
    auto FinalizeTransaction() noexcept -> Transaction
    {
        auto inputs = [&] {
            auto output = Vector<Input>{};  // TODO allocator
            output.reserve(inputs_.size());
            output.clear();
            std::transform(
                std::begin(inputs_),
                std::end(inputs_),
                std::back_inserter(output),
                [](auto& input) -> auto{ return std::move(input.first); });

            return output;
        }();

        return factory::BitcoinTransaction(
            api_.Crypto(),
            chain_,
            Clock::now(),
            version_,
            lock_time_,
            segwit_,
            inputs,
            outputs_,
            {}  // TODO allocator
        );
    }
    auto ReleaseKeys() noexcept -> void
    {
        const auto& api = api_.Crypto().Blockchain();

        for (const auto& key : outgoing_keys_) { api.Release(key); }

        for (const auto& key : change_keys_) { api.Release(key); }
    }
    auto SignInputs() noexcept -> bool
    {
        auto index = int{-1};
        auto txcopy = Transaction{};
        auto bip143 = std::optional<bitcoin::Bip143Hashes>{};

        for (auto& [input, value] : inputs_) {
            if (false == sign_input(++index, input, txcopy, bip143)) {
                LogError()(OT_PRETTY_CLASS())("Failed to sign input ")(index)
                    .Flush();

                return false;
            }
        }

        return true;
    }

    Imp(const api::Session& api,
        database::Wallet& db,
        const identifier::Generic& id,
        const Proposal& proposal,
        const Type chain,
        const Amount feeRate) noexcept
        : api_(api)
        , sender_([&] {
            const auto nymid = [&] {
                auto out = identifier::Nym{};
                const auto& sender = proposal.initiator();
                out.Assign(sender.data(), sender.size());

                return out;
            }();

            OT_ASSERT(false == nymid.empty());

            return api_.Wallet().Nym(nymid);
        }())
        , self_contact_(
              api_.Crypto().Blockchain().Internal().Contacts().ContactID(
                  sender_->ID()))
        , chain_(chain)
        , fee_rate_(feeRate)
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
        , change_keys_()
        , outgoing_keys_([&] {
            auto out = UnallocatedSet<KeyID>{};

            for (const auto& output : proposal.output()) {
                if (false == output.has_paymentcodechannel()) { continue; }

                using CryptoSubchain = blockchain::crypto::Subchain;
                out.emplace(
                    api_.Factory().IdentifierFromBase58(
                        output.paymentcodechannel()),
                    CryptoSubchain::Outgoing,
                    output.index());
            }

            return out;
        }())
    {
        OT_ASSERT(sender_);
    }

private:
    using Input = bitcoin::block::Input;
    using Output = bitcoin::block::Output;
    using Bip143 = std::optional<bitcoin::Bip143Hashes>;
    using Hash = std::array<std::byte, 32>;

    static constexpr auto p2pkh_output_bytes_ = 34_uz;

    const api::Session& api_;
    const Nym_p sender_;
    const identifier::Generic self_contact_;
    const Type chain_;
    const Amount fee_rate_;
    const be::little_int32_buf_t version_;
    const be::little_uint32_buf_t lock_time_;
    mutable bool segwit_;
    UnallocatedVector<Output> outputs_;
    UnallocatedVector<Output> change_;
    UnallocatedVector<std::pair<Input, Amount>> inputs_;
    const std::size_t fixed_overhead_;
    bitcoin::CompactSize input_count_;
    bitcoin::CompactSize output_count_;
    std::size_t input_total_;
    std::size_t witness_total_;
    std::size_t output_total_;
    Amount input_value_;
    Amount output_value_;
    UnallocatedSet<KeyID> change_keys_;
    UnallocatedSet<KeyID> outgoing_keys_;

    static auto is_segwit(const Input& input) noexcept -> bool
    {
        using enum bitcoin::block::script::Pattern;

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

    auto add_signatures(
        const ReadView preimage,
        const blockchain::bitcoin::SigHash& sigHash,
        Input& input) const noexcept -> bool
    {
        const auto reason = api_.Factory().PasswordPrompt(__func__);
        const auto& output = input.Internal().Spends();
        using enum bitcoin::block::script::Pattern;

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
                LogError()(OT_PRETTY_CLASS())("Unsupported input type").Flush();

                return false;
            }
        }
    }
    auto add_signatures_p2ms(
        const ReadView preimage,
        const blockchain::bitcoin::SigHash& sigHash,
        const PasswordPrompt& reason,
        const Output& spends,
        Input& input) const noexcept -> bool
    {
        const auto& script = spends.Script();

        if ((1u != script.M().value()) || (3u != script.N().value())) {
            LogError()(OT_PRETTY_CLASS())("Unsupported multisig pattern")
                .Flush();

            return false;
        }

        auto keys = UnallocatedVector<ByteArray>{};
        auto signatures = UnallocatedVector<Space>{};
        auto views = bitcoin::block::internal::Input::Signatures{};
        const auto& api = api_.Crypto().Blockchain();

        for (const auto& id : input.Keys({})) {  // TODO allocator
            LogVerbose()(OT_PRETTY_CLASS())("Loading element ")(crypto::print(
                id))(" to sign previous output ")(input.PreviousOutput().str())
                .Flush();
            const auto& node = api.GetKey(id);

            if (const auto got = node.KeyID(); got != id) {
                LogError()(OT_PRETTY_CLASS())(
                    "api::Blockchain::GetKey returned the wrong key")
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("requested: ")(crypto::print(id))
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("      got: ")(crypto::print(got))
                    .Flush();

                OT_FAIL;
            }

            const auto& key = node.PrivateKey(reason);

            OT_ASSERT(key.IsValid());

            if (key.PublicKey() != script.MultisigPubkey(0).value()) {
                LogError()(OT_PRETTY_CLASS())("Pubkey mismatch").Flush();

                continue;
            }

            auto& sig = signatures.emplace_back();
            sig.reserve(80);
            const auto haveSig =
                key.SignDER(preimage, hash_type(), writer(sig), reason);

            if (false == haveSig) {
                LogError()(OT_PRETTY_CLASS())("Failed to obtain signature")
                    .Flush();

                return false;
            }

            sig.emplace_back(sigHash.flags_);

            OT_ASSERT(0 < key.PublicKey().size());

            views.emplace_back(reader(sig), ReadView{});
        }

        if (0 == views.size()) {
            LogError()(OT_PRETTY_CLASS())("No keys available for signing ")(
                input.PreviousOutput().str())
                .Flush();

            return false;
        }

        if (false == input.Internal().AddMultisigSignatures(views)) {
            LogError()(OT_PRETTY_CLASS())("Failed to apply signature").Flush();

            return false;
        }

        return true;
    }
    auto add_signatures_p2pk(
        const ReadView preimage,
        const blockchain::bitcoin::SigHash& sigHash,
        const PasswordPrompt& reason,
        const Output& spends,
        Input& input) const noexcept -> bool
    {
        auto keys = UnallocatedVector<ByteArray>{};
        auto signatures = UnallocatedVector<Space>{};
        auto views = bitcoin::block::internal::Input::Signatures{};
        const auto& api = api_.Crypto().Blockchain();

        for (const auto& id : input.Keys({})) {  // TODO allocator
            LogVerbose()(OT_PRETTY_CLASS())("Loading element ")(crypto::print(
                id))(" to sign previous output ")(input.PreviousOutput().str())
                .Flush();
            const auto& node = api.GetKey(id);

            if (const auto got = node.KeyID(); got != id) {
                LogError()(OT_PRETTY_CLASS())(
                    "api::Blockchain::GetKey returned the wrong key")
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("requested: ")(crypto::print(id))
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("      got: ")(crypto::print(got))
                    .Flush();

                OT_FAIL;
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
                LogError()(OT_PRETTY_CLASS())("Failed to obtain signature")
                    .Flush();

                return false;
            }

            sig.emplace_back(sigHash.flags_);

            OT_ASSERT(0 < key.PublicKey().size());

            views.emplace_back(reader(sig), ReadView{});
        }

        if (0 == views.size()) {
            LogError()(OT_PRETTY_CLASS())("No keys available for signing ")(
                input.PreviousOutput().str())
                .Flush();

            return false;
        }

        if (false == input.Internal().AddSignatures(views)) {
            LogError()(OT_PRETTY_CLASS())("Failed to apply signature").Flush();

            return false;
        }

        return true;
    }
    auto add_signatures_p2pkh(
        const ReadView preimage,
        const blockchain::bitcoin::SigHash& sigHash,
        const PasswordPrompt& reason,
        const Output& spends,
        Input& input) const noexcept -> bool
    {
        auto keys = UnallocatedVector<ByteArray>{};
        auto signatures = UnallocatedVector<Space>{};
        auto views = bitcoin::block::internal::Input::Signatures{};
        const auto& api = api_.Crypto().Blockchain();

        for (const auto& id : input.Keys({})) {  // FIXME
            LogVerbose()(OT_PRETTY_CLASS())("Loading element ")(crypto::print(
                id))(" to sign previous output ")(input.PreviousOutput().str())
                .Flush();
            const auto& node = api.GetKey(id);

            if (const auto got = node.KeyID(); got != id) {
                LogError()(OT_PRETTY_CLASS())(
                    "api::Blockchain::GetKey returned the wrong key")
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("requested: ")(crypto::print(id))
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("      got: ")(crypto::print(got))
                    .Flush();

                OT_FAIL;
            }

            const auto& pub =
                validate(Match::ByHash, node, input.PreviousOutput(), spends);

            if (false == pub.IsValid()) { continue; }

            const auto& key = get_private_key(pub, node, reason);

            if (false == key.IsValid()) { continue; }

            const auto& pubkey = keys.emplace_back(
                api_.Factory().DataFromBytes(key.PublicKey()));
            auto& sig = signatures.emplace_back();
            sig.reserve(80);
            const auto haveSig =
                key.SignDER(preimage, hash_type(), writer(sig), reason);

            if (false == haveSig) {
                LogError()(OT_PRETTY_CLASS())("Failed to obtain signature")
                    .Flush();

                return false;
            }

            sig.emplace_back(sigHash.flags_);

            OT_ASSERT(0 < key.PublicKey().size());

            views.emplace_back(reader(sig), pubkey.Bytes());
        }

        if (0 == views.size()) {
            LogError()(OT_PRETTY_CLASS())("No keys available for signing ")(
                input.PreviousOutput().str())
                .Flush();

            return false;
        }

        if (false == input.Internal().AddSignatures(views)) {
            LogError()(OT_PRETTY_CLASS())("Failed to apply signature").Flush();

            return false;
        }

        return true;
    }
    auto bytes() const noexcept -> std::size_t
    {
        // NOTE assumes one additional output to account for change
        const auto outputs = bitcoin::CompactSize{output_count_.Value() + 1};
        const auto base = fixed_overhead_ + input_count_.Size() + input_total_ +
                          outputs.Size() + output_total_ + p2pkh_output_bytes_;

        if (false == segwit_) { return base; }

        static constexpr auto markerBytes = 2_uz;
        const auto segwit = markerBytes + witness_total_;
        const auto total = base + segwit;
        const auto scale = params::get(chain_).SegwitScaleFactor();

        OT_ASSERT(0 < scale);

        const auto factor = scale - 1u;
        // TODO check for std::size_t overflow?
        const auto wu = (segwit * factor) + total;
        static constexpr auto ceil = [](const auto a, const auto b) {
            return (a + (b - 1u)) / b;
        };

        return ceil(wu, scale);
    }
    auto dust() const noexcept -> std::uint64_t
    {
        // TODO this should account for script type

        const auto amount = 148 * fee_rate_ / 1000;
        auto dust = std::uint64_t{};
        try {
            dust = amount.Internal().ExtractUInt64();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("error calculating dust: ")(e.what())
                .Flush();
        }
        return dust;
    }
    auto get_private_key(
        const opentxs::crypto::asymmetric::key::EllipticCurve& pubkey,
        const blockchain::crypto::Element& element,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&
    {
        const auto& key = element.PrivateKey(reason);

        if (false == key.IsValid()) {
            LogError()(OT_PRETTY_CLASS())("failed to obtain private key ")(
                crypto::print(element.KeyID()))
                .Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }

        OT_ASSERT(key.HasPrivate());

        if (key.PublicKey() != pubkey.PublicKey()) {
            const auto got = api_.Factory().DataFromBytes(key.PublicKey());
            const auto expected =
                api_.Factory().DataFromBytes(pubkey.PublicKey());
            const auto [account, subchain, index] = element.KeyID();
            LogAbort()(OT_PRETTY_CLASS())("Derived private key for "
                                          "account ")(account)(" subchain"
                                                               " ")(
                static_cast<std::uint32_t>(subchain))(" index ")(
                index)(" does not correspond to the expected public key. Got ")
                .asHex(got)(" expected ")
                .asHex(expected)
                .Abort();
        }

        return key;
    }
    auto hash_type() const noexcept -> opentxs::crypto::HashType
    {
        return opentxs::crypto::HashType::Sha256D;
    }
    auto init_bip143(Bip143& bip143) const noexcept -> bool
    {
        if (bip143.has_value()) { return true; }

        auto success{false};
        const auto postcondition = ScopeGuard{[&]() {
            if (false == success) { bip143 = std::nullopt; }
        }};
        bip143.emplace();

        OT_ASSERT(bip143.has_value());

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
                LogError()(OT_PRETTY_CLASS())("Failed to hash outpoints")
                    .Flush();

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
                LogError()(OT_PRETTY_CLASS())("Failed to hash sequences")
                    .Flush();

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
                    LogError()(OT_PRETTY_CLASS())("Failed to serialize output")
                        .Flush();

                    return false;
                }

                std::advance(it, size);
            }

            if (false == cb(preimage, hashes.outputs_)) {
                LogError()(OT_PRETTY_CLASS())("Failed to hash outputs").Flush();

                return false;
            }
        }

        success = true;

        return true;
    }
    auto init_txcopy(Transaction& txcopy) const noexcept -> bool
    {
        if (txcopy.IsValid()) { return true; }

        auto inputs = Vector<Input>{};  // TODO allocator
        inputs.reserve(inputs_.size());
        inputs.clear();
        std::transform(
            std::begin(inputs_),
            std::end(inputs_),
            std::back_inserter(inputs),
            [](const auto& input) -> auto{
                // TODO allocator
                return input.first.Internal().SignatureVersion({});
            });
        auto outputs = Vector<Output>{};  // TODO allocator
        outputs.reserve(outputs_.size());
        outputs.clear();
        std::copy(
            std::begin(outputs_),
            std::end(outputs_),
            std::back_inserter(outputs));

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
    auto print() const noexcept -> UnallocatedCString
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
        text << "total output value: " << definition.Format(output_value_)
             << '\n';
        text << " total input value: " << definition.Format(input_value_)
             << '\n';
        text << "               fee: " << definition.Format(fee);

        return text.str();
    }
    auto required_fee() const noexcept -> Amount
    {
        return (bytes() * fee_rate_) / 1000;
    }
    auto sign_input(
        const int index,
        Input& input,
        Transaction& txcopy,
        Bip143& bip143) const noexcept -> bool
    {
        switch (chain_) {
            case Type::BitcoinCash:
            case Type::BitcoinCash_testnet3:
            case Type::BitcoinSV:
            case Type::BitcoinSV_testnet3:
            case Type::eCash:
            case Type::eCash_testnet3: {

                return sign_input_bch(index, input, bip143);
            }
            case Type::Bitcoin:
            case Type::Bitcoin_testnet3:
            case Type::Litecoin:
            case Type::Litecoin_testnet4:
            case Type::PKT:
            case Type::PKT_testnet:
            case Type::UnitTest: {
                if (is_segwit(input)) {

                    return sign_input_segwit(index, input, bip143);
                }

                return sign_input_btc(index, input, txcopy);
            }
            case Type::Unknown:
            case Type::Ethereum_frontier:
            case Type::Ethereum_ropsten:
            default: {
                LogError()(OT_PRETTY_CLASS())("Unsupported chain").Flush();

                return false;
            }
        }
    }
    auto sign_input_bch(const int index, Input& input, Bip143& bip143)
        const noexcept -> bool
    {
        if (false == init_bip143(bip143)) {
            LogError()(OT_PRETTY_CLASS())("Error instantiating bip143").Flush();

            return false;
        }

        const auto sigHash = blockchain::bitcoin::SigHash{chain_};
        const auto preimage = bip143->Preimage(
            index, outputs_.size(), version_, lock_time_, sigHash, input);

        return add_signatures(preimage.Bytes(), sigHash, input);
    }
    auto sign_input_btc(const int index, Input& input, Transaction& txcopy)
        const noexcept -> bool
    {
        if (false == init_txcopy(txcopy)) {
            LogError()(OT_PRETTY_CLASS())("Error instantiating txcopy").Flush();

            return false;
        }

        const auto sigHash = blockchain::bitcoin::SigHash{chain_};
        auto preimage =
            txcopy.Internal().asBitcoin().GetPreimageBTC(index, sigHash);

        if (0 == preimage.size()) {
            LogError()(OT_PRETTY_CLASS())("Error obtaining signing preimage")
                .Flush();

            return false;
        }

        std::copy(sigHash.begin(), sigHash.end(), std::back_inserter(preimage));

        return add_signatures(reader(preimage), sigHash, input);
    }
    auto sign_input_segwit(const int index, Input& input, Bip143& bip143)
        const noexcept -> bool
    {
        if (false == init_bip143(bip143)) {
            LogError()(OT_PRETTY_CLASS())("Error instantiating bip143").Flush();

            return false;
        }

        segwit_ = true;
        const auto sigHash = blockchain::bitcoin::SigHash{chain_};
        const auto preimage = bip143->Preimage(
            index, outputs_.size(), version_, lock_time_, sigHash, input);

        return add_signatures(preimage.Bytes(), sigHash, input);
    }
    enum class Match : bool { ByValue, ByHash };
    auto validate(
        const Match match,
        const blockchain::crypto::Element& element,
        const block::Outpoint& outpoint,
        const Output& output) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&
    {
        const auto [account, subchain, index] = element.KeyID();
        LogTrace()(OT_PRETTY_CLASS())("considering spend key ")(
            index)(" from subchain ")(static_cast<std::uint32_t>(subchain))(
            " of account ")(account)(" for previous "
                                     "output ")(outpoint.str())
            .Flush();

        const auto& key = element.Key();

        if (false == key.IsValid()) {
            LogError()(OT_PRETTY_CLASS())("missing public key").Flush();

            return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
        }

        if (Match::ByValue == match) {
            const auto expected = output.Script().Pubkey();

            if (false == expected.has_value()) {
                LogError()(OT_PRETTY_CLASS())("wrong output script type")
                    .Flush();

                return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
            }

            if (key.PublicKey() != expected.value()) {
                LogError()(OT_PRETTY_CLASS())(
                    "Provided public key does not match expected value")
                    .Flush();

                return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
            }
        } else {
            const auto expected = output.Script().PubkeyHash();

            if (false == expected.has_value()) {
                LogError()(OT_PRETTY_CLASS())("wrong output script type")
                    .Flush();

                return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
            }

            if (element.PubkeyHash().Bytes() != expected.value()) {
                LogError()(OT_PRETTY_CLASS())(
                    "Provided public key does not match expected hash")
                    .Flush();

                return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
            }
        }

        return key;
    }

    auto bip_69() noexcept -> void
    {
        auto inputSort = [](const auto& lhs, const auto& rhs) -> auto
        {
            return lhs.first.PreviousOutput() < rhs.first.PreviousOutput();
        };
        auto outputSort = [](const auto& lhs, const auto& rhs) -> auto
        {
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

        std::sort(std::begin(inputs_), std::end(inputs_), inputSort);
        std::sort(std::begin(outputs_), std::end(outputs_), outputSort);
        auto index{-1};

        for (auto& output : outputs_) { output.Internal().SetIndex(++index); }
    }
};

BitcoinTransactionBuilder::BitcoinTransactionBuilder(
    const api::Session& api,
    database::Wallet& db,
    const identifier::Generic& id,
    const Proposal& proposal,
    const Type chain,
    const Amount feeRate) noexcept
    : imp_(std::make_unique<Imp>(api, db, id, proposal, chain, feeRate))
{
    OT_ASSERT(imp_);
}

auto BitcoinTransactionBuilder::AddChange(const Proposal& data) noexcept -> bool
{
    return imp_->AddChange(data);
}

auto BitcoinTransactionBuilder::AddInput(const UTXO& utxo) noexcept -> bool
{
    return imp_->AddInput(utxo);
}

auto BitcoinTransactionBuilder::CreateOutputs(const Proposal& proposal) noexcept
    -> bool
{
    return imp_->CreateOutputs(proposal);
}

auto BitcoinTransactionBuilder::FinalizeOutputs() noexcept -> void
{
    return imp_->FinalizeOutputs();
}

auto BitcoinTransactionBuilder::FinalizeTransaction() noexcept -> Transaction
{
    return imp_->FinalizeTransaction();
}

auto BitcoinTransactionBuilder::IsFunded() const noexcept -> bool
{
    return imp_->IsFunded();
}

auto BitcoinTransactionBuilder::ReleaseKeys() noexcept -> void
{
    return imp_->ReleaseKeys();
}

auto BitcoinTransactionBuilder::SignInputs() noexcept -> bool
{
    return imp_->SignInputs();
}
auto BitcoinTransactionBuilder::Spender() const noexcept
    -> const identifier::Nym&
{
    return imp_->Spender();
}

BitcoinTransactionBuilder::~BitcoinTransactionBuilder() = default;
}  // namespace opentxs::blockchain::node::wallet
