// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/NotificationStateData.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/HDPath.pb.h"

namespace opentxs::blockchain::node::wallet
{
NotificationStateData::NotificationStateData(
    const api::Session& api,
    const node::internal::Network& node,
    Accounts& parent,
    const WalletDatabase& db,
    const std::function<void(const Identifier&, const char*)>& taskFinished,
    Outstanding& jobCounter,
    const filter::Type filter,
    const Type chain,
    const identifier::Nym& nym,
    PaymentCode&& code,
    proto::HDPath&& path) noexcept
    : SubchainStateData(
          api,
          node,
          parent,
          db,
          OTNymID{nym},
          crypto::SubaccountType::Notification,
          calculate_id(api, chain, code),
          taskFinished,
          jobCounter,
          filter,
          Subchain::Notification)
    , path_(std::move(path))
    , index_(*this, scan_, rescan_, progress_, std::move(code))
    , code_(index_.code_)
{
    init();
    auto reason =
        api_.Factory().PasswordPrompt("Verifying / updating contact data");
    auto mNym = api_.Wallet().mutable_Nym(nym, reason);
    const auto type = BlockchainToUnit(chain);
    const auto existing = mNym.PaymentCode(type);
    const auto expected = code_.asBase58();

    if (existing != expected) {
        mNym.AddPaymentCode(expected, type, existing.empty(), true, reason);
    }
}

auto NotificationStateData::calculate_id(
    const api::Session& api,
    const Type chain,
    const PaymentCode& code) noexcept -> OTIdentifier
{
    auto preimage = api.Factory().Data(code.ID().Bytes());
    preimage->Concatenate(&chain, sizeof(chain));
    auto output = api.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto NotificationStateData::handle_confirmed_matches(
    const block::bitcoin::Block& block,
    const block::Position& position,
    const block::Matches& confirmed) noexcept -> void
{
    const auto& [utxo, general] = confirmed;
    LogVerbose()(OT_PRETTY_CLASS())(general.size())(" confirmed matches for ")(
        code_.asBase58())(" on ")(DisplayString(node_.Chain()))
        .Flush();

    if (0u == general.size()) { return; }

    const auto reason = init_keys();

    for (const auto& match : general) {
        const auto& [txid, elementID] = match;
        const auto& [version, subchainID] = elementID;
        LogVerbose()(OT_PRETTY_CLASS())(DisplayString(node_.Chain()))(
            " transaction ")(txid->asHex())(" contains a version ")(
            version)(" notification for ")(code_.asBase58())
            .Flush();
        const auto tx = block.at(txid->Bytes());

        OT_ASSERT(tx);

        process(match, *tx, reason);
    }
}

auto NotificationStateData::handle_mempool_matches(
    const block::Matches& matches,
    std::unique_ptr<const block::bitcoin::Transaction> tx) noexcept -> void
{
    const auto& [utxo, general] = matches;

    if (0u == general.size()) { return; }

    const auto reason = init_keys();

    for (const auto& match : general) {
        const auto& [txid, elementID] = match;
        const auto& [version, subchainID] = elementID;
        LogVerbose()(OT_PRETTY_CLASS())(DisplayString(node_.Chain()))(
            " mempool transaction ")(txid->asHex())(" contains a version ")(
            version)(" notification for ")(code_.asBase58())
            .Flush();
        process(match, *tx, reason);
    }
}

auto NotificationStateData::init_keys() noexcept -> OTPasswordPrompt
{
    const auto reason = api_.Factory().PasswordPrompt(
        "Decoding payment code notification transaction");

    if (auto key{code_.Key()}; key) {
        if (false == key->HasPrivate()) {
            auto seed{path_.root()};
            const auto upgraded = code_.Internal().AddPrivateKeys(
                seed, *path_.child().rbegin(), reason);

            if (false == upgraded) { OT_FAIL; }
        }
    } else {
        OT_FAIL;
    }

    return reason;
}

auto NotificationStateData::process(
    const block::Match match,
    const block::bitcoin::Transaction& tx,
    const PasswordPrompt& reason) noexcept -> void
{
    const auto& [txid, elementID] = match;
    const auto& [version, subchainID] = elementID;

    for (const auto& output : tx.Outputs()) {
        const auto& script = output.Script();

        if (script.IsNotification(version, code_)) {
            const auto elements = [&] {
                auto out = std::pmr::vector<Space>{};

                for (auto i{0u}; i < 3u; ++i) {
                    const auto view = script.MultisigPubkey(i);

                    OT_ASSERT(view.has_value());

                    const auto& value = view.value();
                    auto* start =
                        reinterpret_cast<const std::byte*>(value.data());
                    auto* stop = std::next(start, value.size());
                    out.emplace_back(start, stop);
                }

                return out;
            }();

            auto sender =
                code_.DecodeNotificationElements(version, elements, reason);

            if (0u == sender.Version()) { continue; }

            LogVerbose()(OT_PRETTY_CLASS())(
                "decoded incoming notification from ")(sender.asBase58())(
                " on ")(DisplayString(node_.Chain()))(" for ")(code_.asBase58())
                .Flush();
            const auto& account =
                api_.Crypto().Blockchain().Internal().PaymentCodeSubaccount(
                    owner_, code_, sender, path_, node_.Chain(), reason);
            LogVerbose()(OT_PRETTY_CLASS())("Created new account ")(
                account.ID())
                .Flush();
        }
    }
}

auto NotificationStateData::type() const noexcept -> std::stringstream
{
    auto output = std::stringstream{};
    output << "Payment code notification";

    return output;
}
}  // namespace opentxs::blockchain::node::wallet
