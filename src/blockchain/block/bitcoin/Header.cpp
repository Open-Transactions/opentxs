// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Header.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BitcoinBlockHeaderFields.pb.h"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
#include "serialization/protobuf/BlockchainBlockLocalData.pb.h"
#include "util/Blank.hpp"

#define OT_BITCOIN_BLOCK_HEADER_SIZE 80

namespace opentxs::factory
{
auto BitcoinBlockHeader(
    const api::Session& api,
    const opentxs::blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    opentxs::blockchain::block::pHash&& merkle,
    const AbortFunction abort) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;
    static const auto now = []() {
        return static_cast<std::uint32_t>(Clock::to_time_t(Clock::now()));
    };
    static const auto checkPoW = [](const auto& pow, const auto& target) {
        return OTNumericHash{factory::NumericHash(pow)} < target;
    };
    static const auto highest = [](const std::uint32_t& nonce) {
        return std::numeric_limits<std::uint32_t>::max() == nonce;
    };
    auto serialized = ReturnType::BitcoinFormat{
        version,
        UnallocatedCString{previous.Hash().Bytes()},
        UnallocatedCString{merkle->Bytes()},
        now(),
        nBits,
        0};
    const auto chain = previous.Type();
    const auto target = serialized.Target();
    auto pow = ReturnType::calculate_pow(api, chain, serialized);

    while (true) {
        if (abort && abort()) { return {}; }

        if (checkPoW(pow, target)) { break; }

        const auto nonce = serialized.nonce_.value();

        if (highest(nonce)) {
            serialized.time_ = now();
            serialized.nonce_ = 0;
        } else {
            serialized.nonce_ = nonce + 1;
        }

        pow = ReturnType::calculate_pow(api, chain, serialized);
    }

    try {
        return std::make_unique<ReturnType>(
            api,
            chain,
            ReturnType::subversion_default_,
            ReturnType::calculate_hash(api, chain, serialized),
            std::move(pow),
            serialized.version_.value(),
            previous.Hash(),
            std::move(merkle),
            Clock::from_time_t(std::time_t(serialized.time_.value())),
            serialized.nbits_.value(),
            serialized.nonce_.value(),
            false);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinBlockHeader(
    const api::Session& api,
    const proto::BlockchainBlockHeader& serialized) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    try {
        return std::make_unique<ReturnType>(api, serialized);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinBlockHeader(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView raw) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    if (OT_BITCOIN_BLOCK_HEADER_SIZE != raw.size()) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized block size. Got: ")(raw.size())(" expected ")(
            OT_BITCOIN_BLOCK_HEADER_SIZE)
            .Flush();

        return nullptr;
    }

    static_assert(
        OT_BITCOIN_BLOCK_HEADER_SIZE == sizeof(ReturnType::BitcoinFormat));

    auto serialized = ReturnType::BitcoinFormat{};

    OT_ASSERT(sizeof(serialized) == raw.size());

    const auto result =
        std::memcpy(static_cast<void*>(&serialized), raw.data(), raw.size());

    if (nullptr == result) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to deserialize header")
            .Flush();

        return nullptr;
    }

    auto hash = ReturnType::calculate_hash(api, chain, raw);
    const auto isGenesis =
        blockchain::node::HeaderOracle::GenesisBlockHash(chain) == hash;

    try {
        return std::make_unique<ReturnType>(
            api,
            chain,
            ReturnType::subversion_default_,
            std::move(hash),
            ReturnType::calculate_pow(api, chain, raw),
            serialized.version_.value(),
            api.Factory().Data(ReadView{
                serialized.previous_.data(), serialized.previous_.size()}),
            api.Factory().Data(
                ReadView{serialized.merkle_.data(), serialized.merkle_.size()}),
            Clock::from_time_t(std::time_t(serialized.time_.value())),
            serialized.nbits_.value(),
            serialized.nonce_.value(),
            isGenesis);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinBlockHeader(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::block::Hash& merkle,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    using ReturnType = blockchain::block::bitcoin::implementation::Header;

    try {
        return std::make_unique<ReturnType>(api, chain, merkle, parent, height);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Header::local_data_version_{1};
const VersionNumber Header::subversion_default_{1};

Header::Header(
    const api::Session& api,
    const VersionNumber version,
    const blockchain::Type chain,
    block::pHash&& hash,
    block::pHash&& pow,
    block::pHash&& previous,
    const block::Height height,
    const Status status,
    const Status inheritStatus,
    const blockchain::Work& work,
    const blockchain::Work& inheritWork,
    const VersionNumber subversion,
    const std::int32_t blockVersion,
    block::pHash&& merkle,
    const Time timestamp,
    const std::uint32_t nbits,
    const std::uint32_t nonce,
    const bool validate) noexcept(false)
    : bitcoin::Header()
    , ot_super(
          api,
          version,
          chain,
          std::move(hash),
          std::move(pow),
          std::move(previous),
          height,
          status,
          inheritStatus,
          work,
          inheritWork)
    , subversion_(subversion)
    , block_version_(blockVersion)
    , merkle_root_(std::move(merkle))
    , timestamp_(timestamp)
    , nbits_(nbits)
    , nonce_(nonce)
{
    OT_ASSERT(validate || (blockchain::Type::UnitTest == type_));

    if (validate && (false == check_pow())) {
        // FIXME is it possible to verify PKT proof of work?
        if ((blockchain::Type::PKT != type_) &&
            (blockchain::Type::PKT_testnet != type_)) {
            throw std::runtime_error("Invalid proof of work");
        }
    }
}

Header::Header(
    const api::Session& api,
    const blockchain::Type chain,
    const VersionNumber subversion,
    block::pHash&& hash,
    block::pHash&& pow,
    const std::int32_t version,
    block::pHash&& previous,
    block::pHash&& merkle,
    const Time timestamp,
    const std::uint32_t nbits,
    const std::uint32_t nonce,
    const bool isGenesis) noexcept(false)
    : Header(
          api,
          default_version_,
          chain,
          std::move(hash),
          std::move(pow),
          std::move(previous),
          (isGenesis ? 0 : make_blank<block::Height>::value(api)),
          (isGenesis ? Status::Checkpoint : Status::Normal),
          Status::Normal,
          calculate_work(chain, nbits),
          minimum_work(chain),
          subversion,
          version,
          std::move(merkle),
          timestamp,
          nbits,
          nonce,
          true)
{
}

Header::Header(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::block::Hash& merkle,
    const blockchain::block::Hash& parent,
    const block::Height height) noexcept(false)
    : Header(
          api,
          default_version_,
          chain,
          OTData{BlankHash()},
          OTData{BlankHash()},
          OTData{parent},
          height,
          (0 == height) ? Status::Checkpoint : Status::Normal,
          Status::Normal,
          minimum_work(chain),
          minimum_work(chain),
          subversion_default_,
          0,
          OTData{merkle},
          make_blank<Time>::value(api),
          NumericHash::MaxTarget(chain),
          0,
          false)
{
    find_nonce();
}

Header::Header(
    const api::Session& api,
    const SerializedType& serialized) noexcept(false)
    : Header(
          api,
          serialized.version(),
          static_cast<blockchain::Type>(serialized.type()),
          calculate_hash(api, serialized),
          calculate_pow(api, serialized),
          api.Factory().Data(
              serialized.bitcoin().previous_header(),
              StringStyle::Raw),
          serialized.local().height(),
          static_cast<Status>(serialized.local().status()),
          static_cast<Status>(serialized.local().inherit_status()),
          OTWork{factory::Work(serialized.local().work())},
          OTWork{factory::Work(serialized.local().inherit_work())},
          serialized.bitcoin().version(),
          serialized.bitcoin().block_version(),
          api.Factory().Data(
              serialized.bitcoin().merkle_hash(),
              StringStyle::Raw),
          Clock::from_time_t(serialized.bitcoin().timestamp()),
          serialized.bitcoin().nbits(),
          serialized.bitcoin().nonce(),
          true)
{
}

Header::Header(const Header& rhs) noexcept
    : bitcoin::Header()
    , ot_super(rhs)
    , subversion_(rhs.subversion_)
    , block_version_(rhs.block_version_)
    , merkle_root_(rhs.merkle_root_)
    , timestamp_(rhs.timestamp_)
    , nbits_(rhs.nbits_)
    , nonce_(rhs.nonce_)
{
}

Header::BitcoinFormat::BitcoinFormat() noexcept
    : version_()
    , previous_()
    , merkle_()
    , time_()
    , nbits_()
    , nonce_()
{
    static_assert(80 == sizeof(BitcoinFormat));
}

Header::BitcoinFormat::BitcoinFormat(
    const std::int32_t version,
    const UnallocatedCString& previous,
    const UnallocatedCString& merkle,
    const std::uint32_t time,
    const std::uint32_t nbits,
    const std::uint32_t nonce) noexcept(false)
    : version_(version)
    , previous_()
    , merkle_()
    , time_(time)
    , nbits_(nbits)
    , nonce_(nonce)
{
    static_assert(80 == sizeof(BitcoinFormat));

    if (sizeof(previous_) < previous.size()) {
        throw std::invalid_argument("Invalid previous hash size");
    }

    if (sizeof(merkle_) < merkle.size()) {
        throw std::invalid_argument("Invalid merkle hash size");
    }

    std::memcpy(previous_.data(), previous.data(), previous.size());
    std::memcpy(merkle_.data(), merkle.data(), merkle.size());
}

auto Header::BitcoinFormat::Target() const noexcept -> OTNumericHash
{
    return OTNumericHash{factory::NumericHashNBits(nbits_.value())};
}

auto Header::calculate_hash(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView serialized) -> block::pHash
{
    auto output = api.Factory().Data();
    BlockHash(api, chain, serialized, output->WriteInto());

    return output;
}

auto Header::calculate_hash(
    const api::Session& api,
    const blockchain::Type chain,
    const BitcoinFormat& in) -> block::pHash
{
    return calculate_hash(
        api, chain, ReadView{reinterpret_cast<const char*>(&in), sizeof(in)});
}

auto Header::calculate_pow(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView serialized) -> block::pHash
{
    auto output = api.Factory().Data();
    ProofOfWorkHash(api, chain, serialized, output->WriteInto());

    return output;
}

auto Header::calculate_pow(
    const api::Session& api,
    const blockchain::Type chain,
    const BitcoinFormat& in) -> block::pHash
{
    return calculate_pow(
        api, chain, ReadView{reinterpret_cast<const char*>(&in), sizeof(in)});
}

auto Header::calculate_hash(
    const api::Session& api,
    const SerializedType& serialized) -> block::pHash
{
    try {
        const auto bytes = preimage(serialized);

        return calculate_hash(
            api,
            static_cast<blockchain::Type>(serialized.type()),
            ReadView(reinterpret_cast<const char*>(&bytes), sizeof(bytes)));
    } catch (const std::invalid_argument& e) {
        LogError()(OT_PRETTY_STATIC(Header))(e.what()).Flush();

        return BlankHash();
    }
}

auto Header::calculate_pow(
    const api::Session& api,
    const SerializedType& serialized) -> block::pHash
{
    try {
        const auto bytes = preimage(serialized);

        return calculate_pow(
            api,
            static_cast<blockchain::Type>(serialized.type()),
            ReadView(reinterpret_cast<const char*>(&bytes), sizeof(bytes)));
    } catch (const std::invalid_argument& e) {
        LogError()(OT_PRETTY_STATIC(Header))(e.what()).Flush();

        return BlankHash();
    }
}

auto Header::calculate_work(
    const blockchain::Type chain,
    const std::uint32_t nbits) -> OTWork
{
    const auto hash = OTNumericHash{factory::NumericHashNBits(nbits)};

    return OTWork{factory::Work(chain, hash)};
}

auto Header::check_pow() const noexcept -> bool
{
    return NumericHash() < Target();
}

auto Header::Encode() const noexcept -> OTData
{
    auto output = api_.Factory().Data();
    Serialize(output->WriteInto());

    return output;
}

auto Header::find_nonce() noexcept(false) -> void
{
    auto& hash = const_cast<OTData&>(hash_);
    auto& pow = const_cast<OTData&>(pow_);
    auto& nonce = const_cast<std::uint32_t&>(nonce_);
    auto bytes = BitcoinFormat{};
    auto view = ReadView{};

    while (true) {
        bytes = preimage([&] {
            auto out = SerializedType{};
            Serialize(out);

            return out;
        }());
        view = ReadView{reinterpret_cast<const char*>(&bytes), sizeof(bytes)};
        pow = calculate_pow(api_, type_, view);

        if (check_pow()) {
            break;
        } else if (std::numeric_limits<std::uint32_t>::max() == nonce) {
            throw std::runtime_error("Nonce not found");
        } else {
            ++nonce;
        }
    }

    hash = calculate_hash(api_, type_, view);
}

auto Header::preimage(const SerializedType& in) -> BitcoinFormat
{
    return BitcoinFormat{
        in.bitcoin().block_version(),
        in.bitcoin().previous_header(),
        in.bitcoin().merkle_hash(),
        in.bitcoin().timestamp(),
        in.bitcoin().nbits(),
        in.bitcoin().nonce()};
}

auto Header::Print() const noexcept -> UnallocatedCString
{
    const auto time = Clock::to_time_t(timestamp_);
    auto out = std::stringstream{};
    out << "  version: " << std::to_string(block_version_) << '\n';
    out << "  parent: " << parent_hash_->asHex() << '\n';
    out << "  merkle: " << merkle_root_->asHex() << '\n';
    out << "  time: " << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
    out << '\n';
    out << "  nBits: " << std::to_string(nbits_) << '\n';
    out << "  nonce: " << std::to_string(nonce_) << '\n';

    return out.str();
}

auto Header::Serialize(SerializedType& out) const noexcept -> bool
{
    const auto time = Clock::to_time_t(timestamp_);

    if (std::numeric_limits<std::uint32_t>::max() < time) { return false; }

    if (false == ot_super::Serialize(out)) { return false; }

    auto& bitcoin = *out.mutable_bitcoin();
    bitcoin.set_version(subversion_);
    bitcoin.set_block_version(block_version_);
    bitcoin.set_previous_header(parent_hash_->str());
    bitcoin.set_merkle_hash(merkle_root_->str());
    bitcoin.set_timestamp(static_cast<std::uint32_t>(time));
    bitcoin.set_nbits(nbits_);
    bitcoin.set_nonce(nonce_);

    return true;
}

auto Header::Serialize(
    const AllocateOutput destination,
    const bool bitcoinformat) const noexcept -> bool
{
    if (bitcoinformat) {
        const auto raw = BitcoinFormat{
            block_version_,
            parent_hash_->str(),
            merkle_root_->str(),
            static_cast<std::uint32_t>(Clock::to_time_t(timestamp_)),
            nbits_,
            nonce_};

        if (false == bool(destination)) {
            LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

            return false;
        }

        const auto out = destination(sizeof(raw));

        if (false == out.valid(sizeof(raw))) {
            LogError()(OT_PRETTY_CLASS())("Failed to allocate output").Flush();

            return false;
        }

        std::memcpy(out.data(), &raw, sizeof(raw));

        return true;
    } else {
        auto proto = SerializedType{};

        if (Serialize(proto)) { return write(proto, destination); }

        return false;
    }
}

auto Header::Target() const noexcept -> OTNumericHash
{
    return OTNumericHash{factory::NumericHashNBits(nbits_)};
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
