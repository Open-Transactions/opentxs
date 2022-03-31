// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "api/session/client/Factory.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iterator>
#include <limits>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/api/session/Factory.hpp"
#if OT_BLOCKCHAIN
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/core/contract/peer/Factory.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/BlockchainBlockHeader.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"

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

#if OT_BLOCKCHAIN
auto Factory::BitcoinBlock(
    const opentxs::blockchain::Type chain,
    const ReadView bytes) const noexcept
    -> std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>
{
    return factory::BitcoinBlock(client_, chain, bytes);
}

auto Factory::BitcoinBlock(
    const opentxs::blockchain::block::Header& previous,
    const Transaction_p generationTransaction,
    const std::uint32_t nBits,
    const UnallocatedVector<Transaction_p>& extraTransactions,
    const std::int32_t version,
    const AbortFunction abort) const noexcept
    -> std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>
{
    return factory::BitcoinBlock(
        api_,
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
    UnallocatedVector<OutputBuilder>&& scripts,
    const UnallocatedCString& coinbase,
    const std::int32_t version) const noexcept -> Transaction_p
{
    static const auto outpoint = opentxs::blockchain::block::Outpoint{};

    const auto serializedVersion = boost::endian::little_int32_buf_t{version};
    const auto locktime = boost::endian::little_uint32_buf_t{0};
    const auto sequence = boost::endian::little_uint32_buf_t{0xffffffff};
    const auto cb = [&] {
        const auto bip34 =
            opentxs::blockchain::block::bitcoin::internal::EncodeBip34(height);
        auto output = space(bip34.size() + coinbase.size());
        auto it = output.data();
        std::memcpy(it, bip34.data(), bip34.size());
        std::advance(it, bip34.size());
        std::memcpy(it, coinbase.data(), coinbase.size());
        output.resize(std::min<std::size_t>(output.size(), 100u));

        return output;
    }();
    const auto cs = opentxs::blockchain::bitcoin::CompactSize{cb.size()};
    auto inputs = UnallocatedVector<std::unique_ptr<
        opentxs::blockchain::block::bitcoin::internal::Input>>{};
    inputs.emplace_back(factory::BitcoinTransactionInput(
        api_,
        chain,
        outpoint.Bytes(),
        cs,
        reader(cb),
        ReadView{reinterpret_cast<const char*>(&sequence), sizeof(sequence)},
        true,
        {}));
    auto outputs = UnallocatedVector<std::unique_ptr<
        opentxs::blockchain::block::bitcoin::internal::Output>>{};
    auto index{-1};
    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    for (auto& [amount, pScript, keys] : scripts) {
        if (false == bool(pScript)) { return {}; }

        const auto& script = *pScript;
        auto bytes = Space{};
        script.Serialize(writer(bytes));
        outputs.emplace_back(factory::BitcoinTransactionOutput(
            api_,
            chain,
            static_cast<std::uint32_t>(++index),
            Amount{amount},
            factory::BitcoinScript(chain, reader(bytes), Position::Output),
            std::move(keys)));
    }

    return factory::BitcoinTransaction(
        api_,
        chain,
        Clock::now(),
        serializedVersion,
        locktime,
        false,  // TODO segwit
        factory::BitcoinTransactionInputs(std::move(inputs)),
        factory::BitcoinTransactionOutputs(std::move(outputs)));
}

auto Factory::BitcoinTransaction(
    const opentxs::blockchain::Type chain,
    const ReadView bytes,
    const bool isGeneration,
    const Time& time) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Transaction>
{
    using Encoded = opentxs::blockchain::bitcoin::EncodedTransaction;

    return factory::BitcoinTransaction(
        api_,
        chain,
        isGeneration ? 0u : std::numeric_limits<std::size_t>::max(),
        time,
        Encoded::Deserialize(api_, chain, bytes));
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
            return factory::BitcoinBlockHeader(client_, serialized);
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
            return factory::BitcoinBlockHeader(client_, type, raw);
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
        client_, opentxs::blockchain::Type::UnitTest, hash, parent, height);
}
#endif  // OT_BLOCKCHAIN

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
