// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"  // IWYU pragma: associated

#include <array>
#include <compare>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/header/HeaderPrivate.hpp"
#include "blockchain/protocol/bitcoin/base/block/header/Imp.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    blockchain::block::Hash&& merkle,
    const AbortFunction abort,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Header;
    using BlankType = blockchain::protocol::bitcoin::base::block::HeaderPrivate;

    try {
        static const auto now = []() {
            return shorten(uint64_to_size(
                seconds_since_epoch_unsigned(Clock::now()).value()));
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

        return pmr::construct<ReturnType>(
            alloc.result_,
            chain,
            ReturnType::subversion_default_,
            ReturnType::calculate_hash(crypto, chain, serialized),
            std::move(pow),
            serialized.version_.value(),
            blockchain::block::Hash{previous.Hash()},
            std::move(merkle),
            seconds_since_epoch_unsigned(serialized.time_.value()).value(),
            serialized.nbits_.value(),
            serialized.nonce_.value(),
            false);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const protobuf::BlockchainBlockHeader& serialized,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Header;
    using BlankType = blockchain::protocol::bitcoin::base::block::HeaderPrivate;

    try {

        return pmr::construct<ReturnType>(alloc.result_, crypto, serialized);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView raw,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Header;
    using BlankType = blockchain::protocol::bitcoin::base::block::HeaderPrivate;

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

        assert_true(sizeof(serialized) <= header.size());

        auto* const result = std::memcpy(
            static_cast<void*>(&serialized), header.data(), header.size());

        if (nullptr == result) {
            throw std::runtime_error{"failed to deserialize header"};
        }

        auto hash = ReturnType::calculate_hash(crypto, chain, header);
        const auto isGenesis =
            blockchain::params::get(chain).GenesisHash() == hash;

        return pmr::construct<ReturnType>(
            alloc.result_,
            chain,
            ReturnType::subversion_default_,
            std::move(hash),
            ReturnType::calculate_pow(crypto, chain, header),
            serialized.version_.value(),
            ReadView{serialized.previous_.data(), serialized.previous_.size()},
            ReadView{serialized.merkle_.data(), serialized.merkle_.size()},
            seconds_since_epoch_unsigned(serialized.time_.value()).value(),
            serialized.nbits_.value(),
            serialized.nonce_.value(),
            isGenesis);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Hash& merkle,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Header;
    using BlankType = blockchain::protocol::bitcoin::base::block::HeaderPrivate;

    try {

        return pmr::construct<ReturnType>(
            alloc.result_, crypto, chain, merkle, parent, height);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
