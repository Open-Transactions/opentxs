// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "blockchain/bitcoin/block/header/Imp.hpp"  // IWYU pragma: associated

#include <BitcoinBlockHeaderFields.pb.h>
#include <BlockchainBlockHeader.pb.h>
#include <BlockchainBlockLocalData.pb.h>
#include <boost/endian/buffers.hpp>
#include <array>
#include <compare>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/block/header/Header.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    blockchain::block::Hash&& merkle,
    const AbortFunction abort) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Header;
    static const auto now = []() {
        return static_cast<std::uint32_t>(Clock::to_time_t(Clock::now()));
    };
    static const auto checkPoW = [](const auto& pow, const auto& target) {
        return blockchain::block::NumericHash{pow} < target;
    };
    static const auto highest = [](const std::uint32_t& nonce) {
        return std::numeric_limits<std::uint32_t>::max() == nonce;
    };
    auto serialized = ReturnType::BitcoinFormat{
        version,
        UnallocatedCString{previous.Hash().Bytes()},
        UnallocatedCString{merkle.Bytes()},
        now(),
        nBits,
        0};
    const auto chain = previous.Type();
    const auto target = serialized.Target();
    auto pow = ReturnType::calculate_pow(crypto, chain, serialized);

    try {
        while (true) {
            if (abort && abort()) { throw std::runtime_error{"aborted"}; }

            if (checkPoW(pow, target)) { break; }

            const auto nonce = serialized.nonce_.value();

            if (highest(nonce)) {
                serialized.time_ = now();
                serialized.nonce_ = 0;
            } else {
                serialized.nonce_ = nonce + 1;
            }

            pow = ReturnType::calculate_pow(crypto, chain, serialized);
        }

        auto imp = std::make_unique<ReturnType>(
            chain,
            ReturnType::subversion_default_,
            ReturnType::calculate_hash(crypto, chain, serialized),
            std::move(pow),
            serialized.version_.value(),
            blockchain::block::Hash{previous.Hash()},
            std::move(merkle),
            convert_stime(std::time_t(serialized.time_.value())),
            serialized.nbits_.value(),
            serialized.nonce_.value(),
            false);

        return std::make_unique<blockchain::bitcoin::block::Header>(
            imp.release());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::bitcoin::block::Header>();
    }
}

auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const proto::BlockchainBlockHeader& serialized) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Header;

    try {
        auto imp = std::make_unique<ReturnType>(crypto, serialized);

        return std::make_unique<blockchain::bitcoin::block::Header>(
            imp.release());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::bitcoin::block::Header>();
    }
}

auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView raw) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Header;

    try {
        if (sizeof(ReturnType::BitcoinFormat) > raw.size()) {
            const auto error =
                CString{"Invalid serialized block header size. Got: "}
                    .append(std::to_string(raw.size()))
                    .append(" expected at least ")
                    .append(std::to_string(sizeof(ReturnType::BitcoinFormat)));
            throw std::runtime_error{error.c_str()};
        }

        const auto header =
            ReadView{raw.data(), sizeof(ReturnType::BitcoinFormat)};
        auto serialized = ReturnType::BitcoinFormat{};

        OT_ASSERT(sizeof(serialized) <= header.size());

        auto* const result = std::memcpy(
            static_cast<void*>(&serialized), header.data(), header.size());

        if (nullptr == result) {
            throw std::runtime_error{"failed to deserialize header"};
        }

        auto hash = ReturnType::calculate_hash(crypto, chain, header);
        const auto isGenesis =
            blockchain::params::get(chain).GenesisHash() == hash;
        auto imp = std::make_unique<ReturnType>(
            chain,
            ReturnType::subversion_default_,
            std::move(hash),
            ReturnType::calculate_pow(crypto, chain, header),
            serialized.version_.value(),
            ReadView{serialized.previous_.data(), serialized.previous_.size()},
            ReadView{serialized.merkle_.data(), serialized.merkle_.size()},
            convert_stime(std::time_t(serialized.time_.value())),
            serialized.nbits_.value(),
            serialized.nonce_.value(),
            isGenesis);

        return std::make_unique<blockchain::bitcoin::block::Header>(
            imp.release());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::bitcoin::block::Header>();
    }
}

auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Hash& merkle,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    using ReturnType = blockchain::bitcoin::block::implementation::Header;

    try {
        auto imp =
            std::make_unique<ReturnType>(crypto, chain, merkle, parent, height);

        return std::make_unique<blockchain::bitcoin::block::Header>(
            imp.release());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<blockchain::bitcoin::block::Header>();
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::bitcoin::block::implementation
{
const VersionNumber Header::local_data_version_{1};
const VersionNumber Header::subversion_default_{1};

Header::Header(
    const VersionNumber version,
    const blockchain::Type chain,
    block::Hash&& hash,
    block::Hash&& pow,
    block::Hash&& previous,
    const block::Height height,
    const Status status,
    const Status inheritStatus,
    const blockchain::Work& work,
    const blockchain::Work& inheritWork,
    const VersionNumber subversion,
    const std::int32_t blockVersion,
    block::Hash&& merkle,
    const Time timestamp,
    const std::uint32_t nbits,
    const std::uint32_t nonce,
    const bool validate) noexcept(false)
    : ot_super(
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
        if ((blockchain::Type::PKT != type_) &&
            (blockchain::Type::PKT_testnet != type_)) {
            throw std::runtime_error("Invalid proof of work");
        }
    }
}

Header::Header(
    const blockchain::Type chain,
    const VersionNumber subversion,
    block::Hash&& hash,
    block::Hash&& pow,
    const std::int32_t version,
    block::Hash&& previous,
    block::Hash&& merkle,
    const Time timestamp,
    const std::uint32_t nbits,
    const std::uint32_t nonce,
    const bool isGenesis) noexcept(false)
    : Header(
          default_version_,
          chain,
          std::move(hash),
          std::move(pow),
          std::move(previous),
          (isGenesis ? 0 : -1),
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
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const block::Hash& merkle,
    const block::Hash& parent,
    const block::Height height) noexcept(false)
    : Header(
          default_version_,
          chain,
          block::Hash{},
          block::Hash{},
          block::Hash{parent},
          height,
          (0 == height) ? Status::Checkpoint : Status::Normal,
          Status::Normal,
          minimum_work(chain),
          minimum_work(chain),
          subversion_default_,
          0,
          block::Hash{merkle},
          Time{},
          params::get(chain).Difficulty(),
          0,
          false)
{
    find_nonce(crypto);
}

Header::Header(
    const api::Crypto& crypto,
    const SerializedType& serialized) noexcept(false)
    : Header(
          serialized.version(),
          static_cast<blockchain::Type>(serialized.type()),
          calculate_hash(crypto, serialized),
          calculate_pow(crypto, serialized),
          block::Hash{serialized.bitcoin().previous_header()},
          serialized.local().height(),
          static_cast<Status>(serialized.local().status()),
          static_cast<Status>(serialized.local().inherit_status()),
          blockchain::Work{IsHex, serialized.local().work()},
          blockchain::Work{IsHex, serialized.local().inherit_work()},
          serialized.bitcoin().version(),
          serialized.bitcoin().block_version(),
          block::Hash{serialized.bitcoin().merkle_hash()},
          convert_stime(serialized.bitcoin().timestamp()),
          serialized.bitcoin().nbits(),
          serialized.bitcoin().nonce(),
          true)
{
}

Header::Header(const Header& rhs) noexcept
    : ot_super(rhs)
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
    static_assert(80_uz == sizeof(BitcoinFormat));
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

auto Header::BitcoinFormat::Target() const noexcept
    -> blockchain::block::NumericHash
{
    return {nbits_.value()};
}

auto Header::calculate_hash(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView serialized) -> block::Hash
{
    auto output = block::Hash{};
    BlockHash(crypto, chain, serialized, output.WriteInto());

    return output;
}

auto Header::calculate_hash(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const BitcoinFormat& in) -> block::Hash
{
    return calculate_hash(
        crypto,
        chain,
        ReadView{reinterpret_cast<const char*>(&in), sizeof(in)});
}

auto Header::calculate_pow(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView serialized) -> block::Hash
{
    auto output = block::Hash{};
    ProofOfWorkHash(crypto, chain, serialized, output.WriteInto());

    return output;
}

auto Header::calculate_pow(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const BitcoinFormat& in) -> block::Hash
{
    return calculate_pow(
        crypto,
        chain,
        ReadView{reinterpret_cast<const char*>(&in), sizeof(in)});
}

auto Header::calculate_hash(
    const api::Crypto& crypto,
    const SerializedType& serialized) -> block::Hash
{
    try {
        const auto bytes = preimage(serialized);

        return calculate_hash(
            crypto,
            static_cast<blockchain::Type>(serialized.type()),
            ReadView(reinterpret_cast<const char*>(&bytes), sizeof(bytes)));
    } catch (const std::invalid_argument& e) {
        LogError()(OT_PRETTY_STATIC(Header))(e.what()).Flush();

        return {};
    }
}

auto Header::calculate_pow(
    const api::Crypto& crypto,
    const SerializedType& serialized) -> block::Hash
{
    try {
        const auto bytes = preimage(serialized);

        return calculate_pow(
            crypto,
            static_cast<blockchain::Type>(serialized.type()),
            ReadView(reinterpret_cast<const char*>(&bytes), sizeof(bytes)));
    } catch (const std::invalid_argument& e) {
        LogError()(OT_PRETTY_STATIC(Header))(e.what()).Flush();

        return {};
    }
}

auto Header::calculate_work(
    const blockchain::Type chain,
    const std::uint32_t nbits) -> blockchain::Work
{
    const auto hash = blockchain::block::NumericHash{nbits};

    return {hash, chain};
}

auto Header::check_pow() const noexcept -> bool
{
    return NumericHash() < Target();
}

auto Header::clone() const noexcept
    -> std::unique_ptr<blockchain::block::Header::Imp>
{
    return std::make_unique<Header>(*this);
}

auto Header::clone_bitcoin() const noexcept -> std::unique_ptr<block::Header>
{
    return std::make_unique<block::Header>(
        std::make_unique<Header>(*this).release());
}

auto Header::Encode() const noexcept -> ByteArray
{
    auto output = ByteArray{};
    Serialize(output.WriteInto());

    return output;
}

auto Header::find_nonce(const api::Crypto& crypto) noexcept(false) -> void
{
    auto& hash = const_cast<block::Hash&>(hash_);
    auto& pow = const_cast<ByteArray&>(pow_);
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
        pow = calculate_pow(crypto, type_, view);

        if (check_pow()) {
            break;
        } else if (std::numeric_limits<std::uint32_t>::max() == nonce) {
            throw std::runtime_error("Nonce not found");
        } else {
            ++nonce;
        }
    }

    hash = calculate_hash(crypto, type_, view);
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
    out << "  parent: " << parent_hash_.asHex() << '\n';
    out << "  merkle: " << merkle_root_.asHex() << '\n';
    out << "  time: " << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
    out << '\n';
    out << "  nBits: " << std::to_string(nbits_) << '\n';
    out << "  nonce: " << std::to_string(nonce_) << '\n';

    return out.str();
}

auto Header::Serialize(SerializedType& out) const noexcept -> bool
{
    const auto time = Clock::to_time_t(timestamp_);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    if (std::numeric_limits<std::uint32_t>::max() < time) { return false; }
#pragma GCC diagnostic pop

    if (false == ot_super::Serialize(out)) { return false; }

    auto& bitcoin = *out.mutable_bitcoin();
    bitcoin.set_version(subversion_);
    bitcoin.set_block_version(block_version_);
    bitcoin.set_previous_header(UnallocatedCString{parent_hash_.Bytes()});
    bitcoin.set_merkle_hash(UnallocatedCString{merkle_root_.Bytes()});
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
            UnallocatedCString{parent_hash_.Bytes()},
            UnallocatedCString{merkle_root_.Bytes()},
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

auto Header::Target() const noexcept -> blockchain::block::NumericHash
{
    return blockchain::block::NumericHash{nbits_};
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation
