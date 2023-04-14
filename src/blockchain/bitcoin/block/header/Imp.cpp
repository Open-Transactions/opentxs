// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "blockchain/bitcoin/block/header/Imp.hpp"  // IWYU pragma: associated

#include <BitcoinBlockHeaderFields.pb.h>
#include <BlockchainBlockHeader.pb.h>
#include <BlockchainBlockLocalData.pb.h>
#include <array>
#include <compare>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "blockchain/block/header/HeaderPrivate.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

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
    const bool validate,
    allocator_type alloc) noexcept(false)
    : blockchain::block::HeaderPrivate(alloc)
    , blockchain::block::implementation::Header(
          version,
          chain,
          std::move(hash),
          std::move(pow),
          std::move(previous),
          height,
          status,
          inheritStatus,
          work,
          inheritWork,
          alloc)
    , HeaderPrivate(alloc)
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
    const bool isGenesis,
    allocator_type alloc) noexcept(false)
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
          true,
          alloc)
{
}

Header::Header(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const block::Hash& merkle,
    const block::Hash& parent,
    const block::Height height,
    allocator_type alloc) noexcept(false)
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
          false,
          alloc)
{
    find_nonce(crypto);
}

Header::Header(
    const api::Crypto& crypto,
    const SerializedType& serialized,
    allocator_type alloc) noexcept(false)
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
          true,
          alloc)
{
}

Header::Header(const Header& rhs, allocator_type alloc) noexcept
    : blockchain::block::HeaderPrivate(rhs, alloc)
    , blockchain::block::implementation::Header(rhs, alloc)
    , HeaderPrivate(rhs, alloc)
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
    return Print({}).c_str();
}

auto Header::Print(allocator_type alloc) const noexcept -> CString
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
    // NOTE someday once LLVM and Apple pull their heads out of their asses and
    // finish c++20 support we can pass an allocator to std::stringstream. Until
    // then there is this.

    return {out.str().c_str(), alloc};
}

auto Header::Serialize(SerializedType& out) const noexcept -> bool
{
    const auto time = Clock::to_time_t(timestamp_);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    if (std::numeric_limits<std::uint32_t>::max() < time) { return false; }
#pragma GCC diagnostic pop

    if (false == blockchain::block::implementation::Header::Serialize(out)) {
        return false;
    }

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

auto Header::Serialize(Writer&& destination, const bool bitcoinformat)
    const noexcept -> bool
{
    if (bitcoinformat) {
        const auto raw = BitcoinFormat{
            block_version_,
            UnallocatedCString{parent_hash_.Bytes()},
            UnallocatedCString{merkle_root_.Bytes()},
            static_cast<std::uint32_t>(Clock::to_time_t(timestamp_)),
            nbits_,
            nonce_};

        return copy(
            reader(std::addressof(raw), sizeof(raw)), std::move(destination));
    } else {
        auto proto = SerializedType{};

        if (Serialize(proto)) { return write(proto, std::move(destination)); }

        return false;
    }
}

auto Header::Target() const noexcept -> blockchain::block::NumericHash
{
    return blockchain::block::NumericHash{nbits_};
}

Header::~Header() = default;
}  // namespace opentxs::blockchain::bitcoin::block::implementation
