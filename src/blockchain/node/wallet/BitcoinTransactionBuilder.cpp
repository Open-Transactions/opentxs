// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/BitcoinTransactionBuilder.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "Proto.hpp"
#include "core/Amount.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainOutputMultisigDetails.pb.h"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"
#include "serialization/protobuf/BlockchainTransactionProposedNotification.pb.h"
#include "serialization/protobuf/BlockchainTransactionProposedOutput.pb.h"
#include "serialization/protobuf/HDPath.pb.h"
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

            auto pOutput = [&] {
                auto elements = [&] {
                    namespace bb = opentxs::blockchain::block::bitcoin;
                    namespace bi = bb::internal;
                    auto out = bb::ScriptElements{};

                    if (const auto size{data.notification().size()}; 1 < size) {
                        throw std::runtime_error{
                            "Multiple notifications not yet supported"};
                    } else if (1 == size) {
                        const auto& notif = data.notification(0);
                        const auto recipient =
                            api_.Factory().PaymentCode(notif.recipient());
                        const auto message =
                            std::string{
                                "Constructing notification transaction to "} +
                            recipient.asBase58();
                        const auto reason =
                            api_.Factory().PasswordPrompt(message);
                        const auto pc = [&] {
                            auto out =
                                api_.Factory().PaymentCode(notif.sender());
                            const auto& path = notif.path();
                            auto seed{path.root()};
                            const auto rc = out.Internal().AddPrivateKeys(
                                seed, *path.child().rbegin(), reason);

                            if (false == rc) {
                                throw std::runtime_error{
                                    "Failed to load private keys"};
                            }

                            return out;
                        }();
                        const auto pKey = element.PrivateKey(reason);

                        if (!pKey) {
                            throw std::runtime_error{
                                "Failed to load private change key"};
                        }

                        const auto& key = *pKey;
                        const auto keys = pc.GenerateNotificationElements(
                            recipient, key, reason);

                        if (3u != keys.size()) {
                            throw std::runtime_error{
                                "Failed to obtain notification elements"};
                        }

                        out.emplace_back(bi::Opcode(bb::OP::ONE));
                        out.emplace_back(bi::PushData(reader(keys.at(0))));
                        out.emplace_back(bi::PushData(reader(keys.at(1))));
                        out.emplace_back(bi::PushData(reader(keys.at(2))));
                        out.emplace_back(bi::Opcode(bb::OP::THREE));
                        out.emplace_back(bi::Opcode(bb::OP::CHECKMULTISIG));
                        isNotification = true;
                    } else {
                        const auto pkh = element.PubkeyHash();
                        out.emplace_back(bi::Opcode(bb::OP::DUP));
                        out.emplace_back(bi::Opcode(bb::OP::HASH160));
                        out.emplace_back(bi::PushData(pkh->Bytes()));
                        out.emplace_back(bi::Opcode(bb::OP::EQUALVERIFY));
                        out.emplace_back(bi::Opcode(bb::OP::CHECKSIG));
                    }

                    return out;
                }();
                using Position = block::bitcoin::Script::Position;
                auto pScript = factory::BitcoinScript(
                    chain_, std::move(elements), Position::Output);

                if (false == bool(pScript)) {
                    throw std::runtime_error{"Failed to construct script"};
                }

                if (std::numeric_limits<std::uint32_t>::max() <
                    outputs_.size()) {
                    throw std::runtime_error{"too many outputs"};
                }

                return factory::BitcoinTransactionOutput(
                    api_,
                    chain_,
                    static_cast<std::uint32_t>(outputs_.size()),
                    Amount{0},
                    std::move(pScript),
                    {keyID});
            }();

            if (false == bool(pOutput)) {
                throw std::runtime_error{"Failed to construct output"};
            }

            if (isNotification) { pOutput->AddTag(TxoTag::Notification); }

            {
                auto& output = *pOutput;
                output_value_ += output.Value();
                output_total_ += output.CalculateSize();

                OT_ASSERT(0 < output.Keys().size());

                output.SetPayee(self_contact_);
                output.SetPayer(self_contact_);
                output.AddTag(TxoTag::Change);

                if (isNotification) { output.AddTag(TxoTag::Notification); }
            }

            change_.emplace_back(std::move(pOutput));
            output_count_ = outputs_.size() + change_.size();

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    auto AddInput(const UTXO& utxo) noexcept -> bool
    {
        auto pInput = factory::BitcoinTransactionInput(api_, chain_, utxo);

        if (false == bool(pInput)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct input").Flush();

            return false;
        }

        const auto& input = *pInput;
        LogTrace()(OT_PRETTY_CLASS())("adding previous output ")(
            utxo.first.str())(" to transaction")
            .Flush();
        input_count_ = inputs_.size();
        input_total_ += input.CalculateSize();
        const auto amount = Amount{utxo.second->Value()};
        input_value_ += amount;
        inputs_.emplace_back(std::move(pInput), amount);

        return true;
    }
    auto CreateOutputs(const Proposal& proposal) noexcept -> bool
    {
        namespace bb = opentxs::blockchain::block::bitcoin;
        namespace bi = bb::internal;

        auto index = std::int32_t{-1};

        for (const auto& output : proposal.output()) {
            auto pScript = std::unique_ptr<bi::Script>{};
            using Position = block::bitcoin::Script::Position;

            if (output.has_raw()) {
                pScript = factory::BitcoinScript(
                    chain_, output.raw(), Position::Output);
            } else {
                auto elements = bb::ScriptElements{};

                if (output.has_pubkeyhash()) {
                    if (output.segwit()) {  // P2WPKH
                        elements.emplace_back(bi::Opcode(bb::OP::ZERO));
                        elements.emplace_back(
                            bi::PushData(output.pubkeyhash()));
                    } else {  // P2PKH
                        elements.emplace_back(bi::Opcode(bb::OP::DUP));
                        elements.emplace_back(bi::Opcode(bb::OP::HASH160));
                        elements.emplace_back(
                            bi::PushData(output.pubkeyhash()));
                        elements.emplace_back(bi::Opcode(bb::OP::EQUALVERIFY));
                        elements.emplace_back(bi::Opcode(bb::OP::CHECKSIG));
                    }
                } else if (output.has_scripthash()) {
                    if (output.segwit()) {  // P2WSH
                        elements.emplace_back(bi::Opcode(bb::OP::ZERO));
                        elements.emplace_back(
                            bi::PushData(output.scripthash()));
                    } else {  // P2SH
                        elements.emplace_back(bi::Opcode(bb::OP::HASH160));
                        elements.emplace_back(
                            bi::PushData(output.scripthash()));
                        elements.emplace_back(bi::Opcode(bb::OP::EQUAL));
                    }
                } else if (output.has_pubkey()) {
                    if (output.segwit()) {  // P2TR
                        elements.emplace_back(bi::Opcode(bb::OP::ONE));
                        elements.emplace_back(bi::PushData(output.pubkey()));
                    } else {  // P2PK
                        elements.emplace_back(bi::PushData(output.pubkey()));
                        elements.emplace_back(bi::Opcode(bb::OP::CHECKSIG));
                    }
                } else if (output.has_multisig()) {  // P2MS
                    const auto& ms = output.multisig();
                    const auto M = static_cast<std::uint8_t>(ms.m());
                    const auto N = static_cast<std::uint8_t>(ms.n());
                    elements.emplace_back(
                        bi::Opcode(static_cast<bb::OP>(M + 80)));

                    for (const auto& key : ms.pubkey()) {
                        elements.emplace_back(bi::PushData(key));
                    }

                    elements.emplace_back(
                        bi::Opcode(static_cast<bb::OP>(N + 80)));
                    elements.emplace_back(bi::Opcode(bb::OP::CHECKMULTISIG));
                } else {
                    LogError()(OT_PRETTY_CLASS())("Unsupported output type")
                        .Flush();

                    return false;
                }

                pScript = factory::BitcoinScript(
                    chain_, std::move(elements), Position::Output);
            }

            if (false == bool(pScript)) {
                LogError()(OT_PRETTY_CLASS())("Failed to construct script")
                    .Flush();

                return false;
            }

            auto pOutput = factory::BitcoinTransactionOutput(
                api_,
                chain_,
                static_cast<std::uint32_t>(++index),
                Amount{output.amount()},
                std::move(pScript),
                {});

            if (false == bool(pOutput)) {
                LogError()(OT_PRETTY_CLASS())("Failed to construct output")
                    .Flush();

                return false;
            }

            pOutput->SetPayer(self_contact_);

            if (output.has_contact()) {
                const auto contactID = [&] {
                    auto out = api_.Factory().Identifier();
                    out->Assign(
                        output.contact().data(), output.contact().size());

                    return out;
                }();
                pOutput->SetPayee(contactID);
            }

            output_value_ += pOutput->Value();
            output_total_ += pOutput->CalculateSize();
            outputs_.emplace_back(std::move(pOutput));
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
            change->SetValue(excessValue);
            output_value_ += change->Value();
            outputs_.emplace_back(std::move(change));
        }

        change_.clear();
        bip_69();
    }
    auto FinalizeTransaction() noexcept -> Transaction
    {
        auto inputs = factory::BitcoinTransactionInputs([&] {
            auto output = std::pmr::vector<Input>{};
            output.reserve(inputs_.size());

            for (auto& [input, value] : inputs_) {
                output.emplace_back(std::move(input));
            }

            return output;
        }());

        if (false == bool(inputs)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct inputs").Flush();

            return {};
        }

        auto outputs = factory::BitcoinTransactionOutputs(std::move(outputs_));

        if (false == bool(outputs)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct outputs")
                .Flush();

            return {};
        }

        return factory::BitcoinTransaction(
            api_,
            chain_,
            Clock::now(),
            version_,
            lock_time_,
            segwit_,
            std::move(inputs),
            std::move(outputs));
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

        for (const auto& [input, value] : inputs_) {
            if (false == sign_input(++index, *input, txcopy, bip143)) {
                LogError()(OT_PRETTY_CLASS())("Failed to sign input ")(index)
                    .Flush();

                return false;
            }
        }

        return true;
    }

    Imp(const api::Session& api,
        const node::internal::WalletDatabase& db,
        const Identifier& id,
        const Proposal& proposal,
        const Type chain,
        const Amount feeRate) noexcept
        : api_(api)
        , sender_([&] {
            const auto id = [&] {
                auto out = api_.Factory().NymID();
                const auto& sender = proposal.initiator();
                out->Assign(sender.data(), sender.size());

                return out;
            }();

            OT_ASSERT(false == id->empty());

            return api_.Wallet().Nym(id);
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
        , output_total_()
        , input_value_()
        , output_value_()
        , change_keys_()
        , outgoing_keys_([&] {
            auto out = std::pmr::set<KeyID>{};

            for (const auto& output : proposal.output()) {
                if (false == output.has_paymentcodechannel()) { continue; }

                using Subchain = blockchain::crypto::Subchain;
                out.emplace(
                    output.paymentcodechannel(),
                    Subchain::Outgoing,
                    output.index());
            }

            return out;
        }())
    {
        OT_ASSERT(sender_);
    }

private:
    using Input = std::unique_ptr<block::bitcoin::internal::Input>;
    using OutputType = block::bitcoin::internal::Output;
    using Output = std::unique_ptr<OutputType>;
    using Bip143 = std::optional<bitcoin::Bip143Hashes>;
    using Hash = std::array<std::byte, 32>;

    static constexpr auto p2pkh_output_bytes_ = std::size_t{34};

    const api::Session& api_;
    const Nym_p sender_;
    const OTIdentifier self_contact_;
    const Type chain_;
    const Amount fee_rate_;
    const be::little_int32_buf_t version_;
    const be::little_uint32_buf_t lock_time_;
    mutable bool segwit_;
    std::pmr::vector<Output> outputs_;
    std::pmr::vector<Output> change_;
    std::pmr::vector<std::pair<Input, Amount>> inputs_;
    const std::size_t fixed_overhead_;
    bitcoin::CompactSize input_count_;
    bitcoin::CompactSize output_count_;
    std::size_t input_total_;
    std::size_t output_total_;
    Amount input_value_;
    Amount output_value_;
    std::pmr::set<KeyID> change_keys_;
    std::pmr::set<KeyID> outgoing_keys_;

    static auto is_segwit(const block::bitcoin::internal::Input& input) noexcept
        -> bool
    {
        using Type = block::bitcoin::Script::Pattern;

        switch (input.Spends().Script().Type()) {
            case Type::PayToWitnessPubkeyHash:
            case Type::PayToWitnessScriptHash:
            case Type::PayToTaproot: {

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
        block::bitcoin::internal::Input& input) const noexcept -> bool
    {
        const auto reason = api_.Factory().PasswordPrompt(__func__);
        const auto& output = input.Spends();
        using Pattern = block::bitcoin::Script::Pattern;

        switch (output.Script().Type()) {
            case Pattern::PayToWitnessPubkeyHash:
            case Pattern::PayToPubkeyHash: {
                return add_signatures_p2pkh(
                    preimage, sigHash, reason, output, input);
            }
            case Pattern::PayToPubkey: {
                return add_signatures_p2pk(
                    preimage, sigHash, reason, output, input);
            }
            case Pattern::PayToMultisig: {
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
        const block::bitcoin::internal::Output& spends,
        block::bitcoin::internal::Input& input) const noexcept -> bool
    {
        const auto& script = spends.Script();

        if ((1u != script.M().value()) || (3u != script.N().value())) {
            LogError()(OT_PRETTY_CLASS())("Unsupported multisig pattern")
                .Flush();

            return false;
        }

        auto keys = std::pmr::vector<OTData>{};
        auto signatures = std::pmr::vector<Space>{};
        auto views = block::bitcoin::internal::Input::Signatures{};
        const auto& api = api_.Crypto().Blockchain();

        for (const auto& id : input.Keys()) {
            LogVerbose()(OT_PRETTY_CLASS())("Loading element ")(opentxs::print(
                id))(" to sign previous output ")(input.PreviousOutput().str())
                .Flush();
            const auto& node = api.GetKey(id);

            if (const auto got = node.KeyID(); got != id) {
                LogError()(OT_PRETTY_CLASS())(
                    "api::Blockchain::GetKey returned the wrong key")
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("requested: ")(opentxs::print(id))
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("      got: ")(
                    opentxs::print(got))
                    .Flush();

                OT_FAIL;
            }

            const auto pKey = node.PrivateKey(reason);

            OT_ASSERT(pKey);

            const auto& key = *pKey;

            if (key.PublicKey() != script.MultisigPubkey(0).value()) {
                LogError()(OT_PRETTY_CLASS())("Pubkey mismatch").Flush();

                continue;
            }

            auto& sig = signatures.emplace_back();
            sig.reserve(80);
            const auto haveSig =
                key.SignDER(preimage, hash_type(), sig, reason);

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

        if (false == input.AddMultisigSignatures(views)) {
            LogError()(OT_PRETTY_CLASS())("Failed to apply signature").Flush();

            return false;
        }

        return true;
    }
    auto add_signatures_p2pk(
        const ReadView preimage,
        const blockchain::bitcoin::SigHash& sigHash,
        const PasswordPrompt& reason,
        const block::bitcoin::internal::Output& spends,
        block::bitcoin::internal::Input& input) const noexcept -> bool
    {
        auto keys = std::pmr::vector<OTData>{};
        auto signatures = std::pmr::vector<Space>{};
        auto views = block::bitcoin::internal::Input::Signatures{};
        const auto& api = api_.Crypto().Blockchain();

        for (const auto& id : input.Keys()) {
            LogVerbose()(OT_PRETTY_CLASS())("Loading element ")(opentxs::print(
                id))(" to sign previous output ")(input.PreviousOutput().str())
                .Flush();
            const auto& node = api.GetKey(id);

            if (const auto got = node.KeyID(); got != id) {
                LogError()(OT_PRETTY_CLASS())(
                    "api::Blockchain::GetKey returned the wrong key")
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("requested: ")(opentxs::print(id))
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("      got: ")(
                    opentxs::print(got))
                    .Flush();

                OT_FAIL;
            }

            const auto pPublic =
                validate(Match::ByValue, node, input.PreviousOutput(), spends);

            if (!pPublic) { continue; }

            const auto& pub = *pPublic;
            const auto pKey = get_private_key(pub, node, reason);

            if (!pKey) { continue; }

            const auto& key = *pKey;
            auto& sig = signatures.emplace_back();
            sig.reserve(80);
            const auto haveSig =
                key.SignDER(preimage, hash_type(), sig, reason);

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

        if (false == input.AddSignatures(views)) {
            LogError()(OT_PRETTY_CLASS())("Failed to apply signature").Flush();

            return false;
        }

        return true;
    }
    auto add_signatures_p2pkh(
        const ReadView preimage,
        const blockchain::bitcoin::SigHash& sigHash,
        const PasswordPrompt& reason,
        const block::bitcoin::internal::Output& spends,
        block::bitcoin::internal::Input& input) const noexcept -> bool
    {
        auto keys = std::pmr::vector<OTData>{};
        auto signatures = std::pmr::vector<Space>{};
        auto views = block::bitcoin::internal::Input::Signatures{};
        const auto& api = api_.Crypto().Blockchain();

        for (const auto& id : input.Keys()) {
            LogVerbose()(OT_PRETTY_CLASS())("Loading element ")(opentxs::print(
                id))(" to sign previous output ")(input.PreviousOutput().str())
                .Flush();
            const auto& node = api.GetKey(id);

            if (const auto got = node.KeyID(); got != id) {
                LogError()(OT_PRETTY_CLASS())(
                    "api::Blockchain::GetKey returned the wrong key")
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("requested: ")(opentxs::print(id))
                    .Flush();
                LogError()(OT_PRETTY_CLASS())("      got: ")(
                    opentxs::print(got))
                    .Flush();

                OT_FAIL;
            }

            const auto pPublic =
                validate(Match::ByHash, node, input.PreviousOutput(), spends);

            if (!pPublic) { continue; }

            const auto& pub = *pPublic;
            const auto pKey = get_private_key(pub, node, reason);

            if (!pKey) { continue; }

            const auto& key = *pKey;

            const auto& pubkey =
                keys.emplace_back(api_.Factory().Data(key.PublicKey()));
            auto& sig = signatures.emplace_back();
            sig.reserve(80);
            const auto haveSig =
                key.SignDER(preimage, hash_type(), sig, reason);

            if (false == haveSig) {
                LogError()(OT_PRETTY_CLASS())("Failed to obtain signature")
                    .Flush();

                return false;
            }

            sig.emplace_back(sigHash.flags_);

            OT_ASSERT(0 < key.PublicKey().size());

            views.emplace_back(reader(sig), pubkey->Bytes());
        }

        if (0 == views.size()) {
            LogError()(OT_PRETTY_CLASS())("No keys available for signing ")(
                input.PreviousOutput().str())
                .Flush();

            return false;
        }

        if (false == input.AddSignatures(views)) {
            LogError()(OT_PRETTY_CLASS())("Failed to apply signature").Flush();

            return false;
        }

        return true;
    }
    auto bytes() const noexcept -> std::size_t
    {
        // NOTE assumes one additional output to account for change
        const auto outputs = bitcoin::CompactSize{output_count_.Value() + 1};

        return fixed_overhead_ + input_count_.Size() + input_total_ +
               outputs.Size() + output_total_ + p2pkh_output_bytes_;
    }
    auto dust() const noexcept -> std::size_t
    {
        // TODO this should account for script type

        const auto amount = 148 * fee_rate_ / 1000;
        auto dust = std::size_t{};
        try {
            dust = amount.Internal().extract_int<std::size_t>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())("error calculating dust: ")(e.what())
                .Flush();
        }
        return dust;
    }
    auto get_private_key(
        const opentxs::crypto::key::EllipticCurve& pubkey,
        const blockchain::crypto::Element& element,
        const PasswordPrompt& reason) const noexcept -> crypto::ECKey
    {
        auto pKey = element.PrivateKey(reason);

        if (!pKey) {
            LogError()(OT_PRETTY_CLASS())("failed to obtain private key ")(
                opentxs::print(element.KeyID()))
                .Flush();

            return {};
        }

        const auto& key = *pKey;

        OT_ASSERT(key.HasPrivate());

        if (key.PublicKey() != pubkey.PublicKey()) {
            const auto got = api_.Factory().Data(key.PublicKey());
            const auto expected = api_.Factory().Data(pubkey.PublicKey());
            const auto [account, subchain, index] = element.KeyID();
            LogError()(OT_PRETTY_CLASS())(
                "Derived private key for "
                "account ")(account)(" subchain"
                                     " ")(static_cast<std::uint32_t>(subchain))(
                " index ")(index)(" does not correspond to the "
                                  "expected public key. Got ")(got->asHex())(
                " expected ")(expected->asHex())
                .Flush();

            OT_FAIL;
        }

        return pKey;
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

        auto& output = bip143.value();
        auto cb = [&](const auto& preimage, auto& output) -> bool {
            return api_.Crypto().Hash().Digest(
                opentxs::crypto::HashType::Sha256D,
                reader(preimage),
                preallocated(output.size(), output.data()));
        };

        {
            auto preimage = space(inputs_.size() * sizeof(block::Outpoint));
            auto it = preimage.data();

            for (const auto& [input, amount] : inputs_) {
                const auto& outpoint = input->PreviousOutput();
                std::memcpy(it, &outpoint, sizeof(outpoint));
                std::advance(it, sizeof(outpoint));
            }

            if (false == cb(preimage, output.outpoints_)) {
                LogError()(OT_PRETTY_CLASS())("Failed to hash outpoints")
                    .Flush();

                return false;
            }
        }

        {
            auto preimage = space(inputs_.size() * sizeof(std::uint32_t));
            auto it = preimage.data();

            for (const auto& [input, value] : inputs_) {
                const auto sequence = input->Sequence();
                std::memcpy(it, &sequence, sizeof(sequence));
                std::advance(it, sizeof(sequence));
            }

            if (false == cb(preimage, output.sequences_)) {
                LogError()(OT_PRETTY_CLASS())("Failed to hash sequences")
                    .Flush();

                return false;
            }
        }

        {
            auto preimage = space(output_total_);
            auto it = preimage.data();

            for (const auto& output : outputs_) {
                const auto size = output->CalculateSize();

                if (false ==
                    output->Serialize(preallocated(size, it)).has_value()) {
                    LogError()(OT_PRETTY_CLASS())("Failed to serialize output")
                        .Flush();

                    return false;
                }

                std::advance(it, size);
            }

            if (false == cb(preimage, output.outputs_)) {
                LogError()(OT_PRETTY_CLASS())("Failed to hash outputs").Flush();

                return false;
            }
        }

        success = true;

        return true;
    }
    auto init_txcopy(Transaction& txcopy) const noexcept -> bool
    {
        if (txcopy) { return true; }

        auto inputCopy = std::pmr::vector<Input>{};
        std::transform(
            std::begin(inputs_),
            std::end(inputs_),
            std::back_inserter(inputCopy),
            [](const auto& input) -> auto {
                return input.first->SignatureVersion();
            });
        auto inputs = factory::BitcoinTransactionInputs(std::move(inputCopy));

        if (false == bool(inputs)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct inputs").Flush();

            return {};
        }

        auto outputCopy = std::pmr::vector<Output>{};
        std::transform(
            std::begin(outputs_),
            std::end(outputs_),
            std::back_inserter(outputCopy),
            [](const auto& output) -> auto { return output->clone(); });
        auto outputs =
            factory::BitcoinTransactionOutputs(std::move(outputCopy));

        if (false == bool(outputs)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct outputs")
                .Flush();

            return {};
        }

        txcopy = factory::BitcoinTransaction(
            api_,
            chain_,
            Clock::now(),
            version_,
            lock_time_,
            false,
            std::move(inputs),
            std::move(outputs));

        return bool(txcopy);
    }
    auto print() const noexcept -> std::string
    {
        auto text = std::stringstream{};
        text << "\n     version: " << std::to_string(version_.value()) << '\n';
        text << "   lock time: " << std::to_string(lock_time_.value()) << '\n';
        text << " input count: " << std::to_string(inputs_.size()) << '\n';

        const auto& definition = blockchain::GetDefinition(chain_);
        for (const auto& [input, value] : inputs_) {
            const auto& outpoint = input->PreviousOutput();
            text << " * " << outpoint.str()
                 << ", sequence: " << std::to_string(input->Sequence())
                 << ", value: " << definition.Format(value) << '\n';
        }

        text << "output count: " << std::to_string(outputs_.size()) << '\n';

        for (const auto& output : outputs_) {
            text << " * bytes: " << std::to_string(output->CalculateSize())
                 << ", value: " << definition.Format(output->Value()) << '\n';
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
        block::bitcoin::internal::Input& input,
        Transaction& txcopy,
        Bip143& bip143) const noexcept -> bool
    {
        switch (chain_) {
            case Type::BitcoinCash:
            case Type::BitcoinCash_testnet3: {

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
    auto sign_input_bch(
        const int index,
        block::bitcoin::internal::Input& input,
        Bip143& bip143) const noexcept -> bool
    {
        if (false == init_bip143(bip143)) {
            LogError()(OT_PRETTY_CLASS())("Error instantiating bip143").Flush();

            return false;
        }

        const auto sigHash = blockchain::bitcoin::SigHash{chain_};
        const auto preimage = bip143->Preimage(
            index, outputs_.size(), version_, lock_time_, sigHash, input);

        return add_signatures(reader(preimage), sigHash, input);
    }
    auto sign_input_btc(
        const int index,
        block::bitcoin::internal::Input& input,
        Transaction& txcopy) const noexcept -> bool
    {
        if (false == init_txcopy(txcopy)) {
            LogError()(OT_PRETTY_CLASS())("Error instantiating txcopy").Flush();

            return false;
        }

        const auto sigHash = blockchain::bitcoin::SigHash{chain_};
        auto preimage = txcopy->GetPreimageBTC(index, sigHash);

        if (0 == preimage.size()) {
            LogError()(OT_PRETTY_CLASS())("Error obtaining signing preimage")
                .Flush();

            return false;
        }

        std::copy(sigHash.begin(), sigHash.end(), std::back_inserter(preimage));

        return add_signatures(reader(preimage), sigHash, input);
    }
    auto sign_input_segwit(
        const int index,
        block::bitcoin::internal::Input& input,
        Bip143& bip143) const noexcept -> bool
    {
        if (false == init_bip143(bip143)) {
            LogError()(OT_PRETTY_CLASS())("Error instantiating bip143").Flush();

            return false;
        }

        segwit_ = true;
        const auto sigHash = blockchain::bitcoin::SigHash{chain_};
        const auto preimage = bip143->Preimage(
            index, outputs_.size(), version_, lock_time_, sigHash, input);

        return add_signatures(reader(preimage), sigHash, input);
    }
    enum class Match : bool { ByValue, ByHash };
    auto validate(
        const Match match,
        const blockchain::crypto::Element& element,
        const block::Outpoint& outpoint,
        const block::bitcoin::internal::Output& output) const noexcept
        -> crypto::ECKey
    {
        const auto [account, subchain, index] = element.KeyID();
        LogTrace()(OT_PRETTY_CLASS())("considering spend key ")(
            index)(" from subchain ")(static_cast<std::uint32_t>(subchain))(
            " of account ")(account)(" for previous "
                                     "output ")(outpoint.str())
            .Flush();

        auto pKey = element.Key();

        if (!pKey) {
            LogError()(OT_PRETTY_CLASS())("missing public key").Flush();

            return {};
        }

        const auto& key = *pKey;

        if (Match::ByValue == match) {
            const auto expected = output.Script().Pubkey();

            if (false == expected.has_value()) {
                LogError()(OT_PRETTY_CLASS())("wrong output script type")
                    .Flush();

                return {};
            }

            if (key.PublicKey() != expected.value()) {
                LogError()(OT_PRETTY_CLASS())(
                    "Provided public key does not match expected value")
                    .Flush();

                return {};
            }
        } else {
            const auto expected = output.Script().PubkeyHash();

            if (false == expected.has_value()) {
                LogError()(OT_PRETTY_CLASS())("wrong output script type")
                    .Flush();

                return {};
            }

            if (element.PubkeyHash()->Bytes() != expected.value()) {
                LogError()(OT_PRETTY_CLASS())(
                    "Provided public key does not match expected hash")
                    .Flush();

                return {};
            }
        }

        return pKey;
    }

    auto bip_69() noexcept -> void
    {
        auto inputSort = [](const auto& lhs, const auto& rhs) -> auto
        {
            return lhs.first->PreviousOutput() < rhs.first->PreviousOutput();
        };
        auto outputSort = [](const auto& lhs, const auto& rhs) -> auto
        {
            if (lhs->Value() == rhs->Value()) {
                auto lScript = Space{};
                auto rScript = Space{};
                lhs->Script().Serialize(writer(lScript));
                rhs->Script().Serialize(writer(rScript));

                return std::lexicographical_compare(
                    std::begin(lScript),
                    std::end(lScript),
                    std::begin(rScript),
                    std::end(rScript));
            } else {

                return lhs->Value() < rhs->Value();
            }
        };

        std::sort(std::begin(inputs_), std::end(inputs_), inputSort);
        std::sort(std::begin(outputs_), std::end(outputs_), outputSort);
        auto index{-1};

        for (const auto& output : outputs_) { output->SetIndex(++index); }
    }
};

BitcoinTransactionBuilder::BitcoinTransactionBuilder(
    const api::Session& api,
    const node::internal::WalletDatabase& db,
    const Identifier& id,
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
