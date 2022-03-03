// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <array>
#include <cstdint>
#include <memory>

#include "blockchain/block/Header.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Header final : virtual public internal::Header,
                     public block::implementation::Header
{
public:
    struct BitcoinFormat {
        be::little_int32_buf_t version_;
        std::array<char, 32> previous_;
        std::array<char, 32> merkle_;
        be::little_uint32_buf_t time_;
        be::little_uint32_buf_t nbits_;
        be::little_uint32_buf_t nonce_;

        auto Target() const noexcept -> OTNumericHash;

        BitcoinFormat(
            const std::int32_t version,
            const UnallocatedCString& previous,
            const UnallocatedCString& merkle,
            const std::uint32_t time,
            const std::uint32_t nbits,
            const std::uint32_t nonce) noexcept(false);
        BitcoinFormat() noexcept;
    };

    static const VersionNumber local_data_version_;
    static const VersionNumber subversion_default_;

    static auto calculate_hash(
        const api::Session& api,
        const blockchain::Type chain,
        const ReadView serialized) -> block::pHash;
    static auto calculate_hash(
        const api::Session& api,
        const blockchain::Type chain,
        const BitcoinFormat& serialized) -> block::pHash;
    static auto calculate_pow(
        const api::Session& api,
        const blockchain::Type chain,
        const ReadView serialized) -> block::pHash;
    static auto calculate_pow(
        const api::Session& api,
        const blockchain::Type chain,
        const BitcoinFormat& serialized) -> block::pHash;

    auto as_Bitcoin() const noexcept -> std::unique_ptr<bitcoin::Header> final
    {
        return std::make_unique<Header>(*this);
    }
    auto clone() const noexcept -> std::unique_ptr<block::Header> final
    {
        return std::make_unique<Header>(*this);
    }
    auto Encode() const noexcept -> OTData final;
    auto MerkleRoot() const noexcept -> const block::Hash& final
    {
        return merkle_root_;
    }
    auto Nonce() const noexcept -> std::uint32_t final { return nonce_; }
    auto nBits() const noexcept -> std::uint32_t final { return nbits_; }
    auto Print() const noexcept -> UnallocatedCString final;
    auto Serialize(SerializedType& out) const noexcept -> bool final;
    auto Serialize(
        const AllocateOutput destination,
        const bool bitcoinformat = true) const noexcept -> bool final;
    auto Target() const noexcept -> OTNumericHash final;
    auto Timestamp() const noexcept -> Time final { return timestamp_; }
    auto Version() const noexcept -> std::uint32_t final
    {
        return block_version_;
    }

    Header(
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
        const bool isGenesis) noexcept(false);
    Header(
        const api::Session& api,
        const blockchain::Type chain,
        const blockchain::block::Hash& merkle,
        const blockchain::block::Hash& parent,
        const block::Height height) noexcept(false);
    Header(const api::Session& api, const SerializedType& serialized) noexcept(
        false);
    Header(const Header& rhs) noexcept;

    ~Header() final = default;

private:
    using ot_super = block::implementation::Header;

    const VersionNumber subversion_;
    const std::int32_t block_version_;
    const OTData merkle_root_;
    const Time timestamp_;
    const std::uint32_t nbits_;
    const std::uint32_t nonce_;

    static auto calculate_hash(
        const api::Session& api,
        const SerializedType& serialized) -> block::pHash;
    static auto calculate_pow(
        const api::Session& api,
        const SerializedType& serialized) -> block::pHash;
    static auto calculate_work(
        const blockchain::Type chain,
        const std::uint32_t nbits) -> OTWork;
    static auto preimage(const SerializedType& in) -> BitcoinFormat;

    auto check_pow() const noexcept -> bool;

    auto find_nonce() noexcept(false) -> void;

    Header(
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
        const bool validate) noexcept(false);
    Header() = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
