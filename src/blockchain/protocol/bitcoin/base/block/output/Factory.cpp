// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionOutput.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/output/Imp.hpp"
#include "blockchain/protocol/bitcoin/base/block/output/OutputPrivate.hpp"
#include "internal/blockchain/token/Types.hpp"
#include "internal/core/Amount.hpp"
#include "internal/core/Factory.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Position.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const opentxs::Amount& value,
    blockchain::protocol::bitcoin::base::block::Script script,
    std::optional<const blockchain::token::cashtoken::Value> cashtoken,
    const UnallocatedSet<blockchain::crypto::Key>& keys,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Output
{
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Output;
    using BlankType = blockchain::protocol::bitcoin::base::block::OutputPrivate;

    try {
        auto keySet = Set<blockchain::crypto::Key>{alloc.result_};
        keySet.clear();
        std::for_each(std::begin(keys), std::end(keys), [&](const auto& key) {
            keySet.emplace(key);
        });

        return pmr::construct<ReturnType>(
            alloc.result_,
            chain,
            index,
            value,
            std::move(script),
            std::move(keySet),
            ReturnType::default_version_,
            std::move(cashtoken));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const opentxs::Amount& value,
    const network::blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    std::optional<const blockchain::token::cashtoken::Value> cashtoken,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Output
{
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Output;
    using BlankType = blockchain::protocol::bitcoin::base::block::OutputPrivate;

    try {

        return pmr::construct<ReturnType>(
            alloc.result_,
            chain,
            index,
            value,
            opentxs::internal::Amount::SerializeBitcoinSize() + cs.Total(),
            script,
            ReturnType::default_version_,
            std::move(cashtoken));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinTransactionOutput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Output
{
    using enum blockchain::protocol::bitcoin::base::block::script::Position;
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Output;
    using BlankType = blockchain::protocol::bitcoin::base::block::OutputPrivate;

    try {
        auto value = factory::Amount(in.value());
        auto cs = network::blockchain::bitcoin::CompactSize(in.script().size());
        auto keys = Set<blockchain::crypto::Key>{alloc.result_};
        auto pkh = ReturnType::PubkeyHashes{alloc.result_};
        keys.clear();
        pkh.clear();
        using Payer = identifier::Generic;
        using Payee = identifier::Generic;
        using Correction = std::pair<Payer, Payee>;
        auto corrections = UnallocatedVector<Correction>{};

        for (const auto& key : in.key()) {
            const auto subchain = static_cast<blockchain::crypto::Subchain>(
                static_cast<std::uint8_t>(key.subchain()));
            auto keyid = blockchain::crypto::Key{
                factory.AccountIDFromBase58(key.subaccount()),
                subchain,
                key.index()};

            if (blockchain::crypto::Subchain::Outgoing == subchain) {
                LogError()("opentxs::factory::")(__func__)(
                    ": invalid key detected in transaction")
                    .Flush();
                auto sender = crypto.SenderContact(keyid);
                auto recipient = crypto.RecipientContact(keyid);

                if (sender.empty() || recipient.empty()) { OT_FAIL; }

                corrections.emplace_back(
                    std::move(sender), std::move(recipient));
            } else {
                keys.emplace(std::move(keyid));
            }
        }

        for (const auto& pattern : in.pubkey_hash()) { pkh.emplace(pattern); }

        auto* out = pmr::construct<ReturnType>(
            alloc.result_,
            chain,
            in.version(),
            in.index(),
            value,
            factory::BitcoinScript(
                chain, in.script(), Output, true, false, alloc),
            std::nullopt,
            std::move(keys),
            [&]() -> blockchain::block::Position {
                if (const auto& hash = in.mined_block(); 0 < hash.size()) {

                    return {in.mined_height(), hash};
                } else {

                    return {};
                }
            }(),
            static_cast<blockchain::node::TxoState>(in.state()),
            [&] {
                auto tags = UnallocatedSet<blockchain::node::TxoTag>{};

                for (const auto& tag : in.tag()) {
                    tags.emplace(static_cast<blockchain::node::TxoTag>(tag));
                }

                return tags;
            }(),
            blockchain::token::cashtoken::deserialize(in));

        for (const auto& payer : in.payer()) {
            if (false == payer.empty()) {
                out->SetPayer([&] {
                    auto id = identifier::Generic{};
                    id.Assign(payer.data(), payer.size());

                    return id;
                }());
            }
        }

        for (const auto& payee : in.payee()) {
            if (false == payee.empty()) {
                out->SetPayee([&] {
                    auto id = identifier::Generic{};
                    id.Assign(payee.data(), payee.size());

                    return id;
                }());
            }
        }

        for (const auto& [payer, payee] : corrections) {
            out->SetPayer(payer);
            out->SetPayee(payee);
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
