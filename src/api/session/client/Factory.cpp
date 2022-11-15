// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Factory

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "api/session/client/Factory.hpp"  // IWYU pragma: associated

#include <BlockchainBlockHeader.pb.h>
#include <cstddef>
#include <exception>
#include <limits>
#include <utility>

#include "internal/api/session/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Input.hpp"    // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Inputs.hpp"   // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Output.hpp"   // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Outputs.hpp"  // IWYU pragma: keep
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/core/contract/peer/Factory.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/BlockchainBlockHeader.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto SessionFactoryAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::Factory>
{
    using ReturnType = api::session::client::Factory;

    try {

        return std::make_unique<ReturnType>(parent);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session::client
{
Factory::Factory(const api::session::Client& parent)
    : session::imp::Factory(parent)
    , client_(parent)
{
}

auto Factory::BitcoinBlock(
    const opentxs::blockchain::Type chain,
    const ReadView bytes) const noexcept
    -> std::shared_ptr<const opentxs::blockchain::bitcoin::block::Block>
{
    return factory::BitcoinBlock(client_.Crypto(), chain, bytes);
}

auto Factory::BitcoinBlock(
    const opentxs::blockchain::block::Header& previous,
    const Transaction_p generationTransaction,
    const std::uint32_t nBits,
    const UnallocatedVector<Transaction_p>& extraTransactions,
    const std::int32_t version,
    const AbortFunction abort) const noexcept
    -> std::shared_ptr<const opentxs::blockchain::bitcoin::block::Block>
{
    return factory::BitcoinBlock(
        api_.Crypto(),
        previous,
        generationTransaction,
        nBits,
        extraTransactions,
        version,
        abort);
}

auto Factory::BitcoinGenerationTransaction(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::Height height,
    UnallocatedVector<blockchain::OutputBuilder>&& scripts,
    const UnallocatedCString& coinbase,
    const std::int32_t version) const noexcept -> Transaction_p
{
    return factory::BitcoinTransaction(
        api_.Crypto(), chain, height, std::move(scripts), coinbase, version);
}

auto Factory::BitcoinTransaction(
    const opentxs::blockchain::Type chain,
    const ReadView bytes,
    const bool isGeneration,
    const Time& time) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::bitcoin::block::Transaction>
{
    return factory::BitcoinTransaction(
        api_.Crypto(),
        chain,
        isGeneration ? 0_uz : std::numeric_limits<std::size_t>::max(),
        time,
        bytes);
}

auto Factory::BlockHeader(const proto::BlockchainBlockHeader& serialized) const
    -> BlockHeaderP
{
    if (false == proto::Validate(serialized, VERBOSE)) { return {}; }

    const auto type(static_cast<opentxs::blockchain::Type>(serialized.type()));

    switch (type) {
        case opentxs::blockchain::Type::Bitcoin:
        case opentxs::blockchain::Type::Bitcoin_testnet3:
        case opentxs::blockchain::Type::BitcoinCash:
        case opentxs::blockchain::Type::BitcoinCash_testnet3:
        case opentxs::blockchain::Type::Litecoin:
        case opentxs::blockchain::Type::Litecoin_testnet4:
        case opentxs::blockchain::Type::PKT:
        case opentxs::blockchain::Type::PKT_testnet:
        case opentxs::blockchain::Type::BitcoinSV:
        case opentxs::blockchain::Type::BitcoinSV_testnet3:
        case opentxs::blockchain::Type::eCash:
        case opentxs::blockchain::Type::eCash_testnet3:
        case opentxs::blockchain::Type::UnitTest: {
            return factory::BitcoinBlockHeader(client_.Crypto(), serialized);
        }
        case opentxs::blockchain::Type::Unknown:
        case opentxs::blockchain::Type::Ethereum_frontier:
        case opentxs::blockchain::Type::Ethereum_ropsten:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported type (")(
                static_cast<std::uint32_t>(type))(")")
                .Flush();

            return {};
        }
    }
}

auto Factory::BlockHeader(const ReadView bytes) const -> BlockHeaderP
{
    return BlockHeader(proto::Factory<proto::BlockchainBlockHeader>(bytes));
}

auto Factory::BlockHeader(
    const opentxs::blockchain::Type type,
    const ReadView raw) const -> BlockHeaderP
{
    switch (type) {
        case opentxs::blockchain::Type::Bitcoin:
        case opentxs::blockchain::Type::Bitcoin_testnet3:
        case opentxs::blockchain::Type::BitcoinCash:
        case opentxs::blockchain::Type::BitcoinCash_testnet3:
        case opentxs::blockchain::Type::Litecoin:
        case opentxs::blockchain::Type::Litecoin_testnet4:
        case opentxs::blockchain::Type::PKT:
        case opentxs::blockchain::Type::PKT_testnet:
        case opentxs::blockchain::Type::BitcoinSV:
        case opentxs::blockchain::Type::BitcoinSV_testnet3:
        case opentxs::blockchain::Type::eCash:
        case opentxs::blockchain::Type::eCash_testnet3:
        case opentxs::blockchain::Type::UnitTest: {
            return factory::BitcoinBlockHeader(client_.Crypto(), type, raw);
        }
        case opentxs::blockchain::Type::Unknown:
        case opentxs::blockchain::Type::Ethereum_frontier:
        case opentxs::blockchain::Type::Ethereum_ropsten:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported type (")(
                static_cast<std::uint32_t>(type))(")")
                .Flush();

            return {};
        }
    }
}

auto Factory::BlockHeader(const opentxs::blockchain::block::Block& block) const
    -> BlockHeaderP
{
    return block.Header().clone();
}

auto Factory::BlockHeaderForUnitTests(
    const opentxs::blockchain::block::Hash& hash,
    const opentxs::blockchain::block::Hash& parent,
    const opentxs::blockchain::block::Height height) const -> BlockHeaderP
{
    return factory::BitcoinBlockHeader(
        client_.Crypto(),
        opentxs::blockchain::Type::UnitTest,
        hash,
        parent,
        height);
}

auto Factory::PeerObject(
    const Nym_p& senderNym,
    const UnallocatedCString& message) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, senderNym, message)};
}

auto Factory::PeerObject(
    const Nym_p& senderNym,
    const UnallocatedCString& payment,
    const bool isPayment) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, senderNym, payment, isPayment)};
}
auto Factory::PeerObject(const Nym_p& senderNym, otx::blind::Purse&& purse)
    const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, senderNym, std::move(purse))};
}

auto Factory::PeerObject(
    const OTPeerRequest request,
    const OTPeerReply reply,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, request, reply, version)};
}

auto Factory::PeerObject(
    const OTPeerRequest request,
    const VersionNumber version) const -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, request, version)};
}

auto Factory::PeerObject(
    const Nym_p& signerNym,
    const proto::PeerObject& serialized) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, signerNym, serialized)};
}

auto Factory::PeerObject(
    const Nym_p& recipientNym,
    const opentxs::Armored& encrypted,
    const opentxs::PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::factory::PeerObject(client_, recipientNym, encrypted, reason)};
}
}  // namespace opentxs::api::session::client
