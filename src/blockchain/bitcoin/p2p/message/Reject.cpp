// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Reject.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinP2PReject(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Reject*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Reject;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Reject 1")
            .Flush();

        return nullptr;
    }

    const auto* it{static_cast<const std::byte*>(payload)};
    // -----------------------------------------------
    auto messageSize = 0_uz;
    const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, messageSize);

    if (!decodedSize) {
        LogError()(__func__)(": CompactSize incomplete for message field")
            .Flush();

        return nullptr;
    }
    // -----------------------------------------------
    expectedSize += messageSize;

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for message field")
            .Flush();

        return nullptr;
    }
    const UnallocatedCString message{
        reinterpret_cast<const char*>(it), messageSize};
    it += messageSize;
    // -----------------------------------------------
    expectedSize += sizeof(std::uint8_t);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for code field")
            .Flush();

        return nullptr;
    }

    auto raw_code = be::little_uint8_buf_t{};
    std::memcpy(static_cast<void*>(&raw_code), it, sizeof(raw_code));
    it += sizeof(raw_code);
    const std::uint8_t code = raw_code.value();
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Reject 1")
            .Flush();

        return nullptr;
    }
    // -----------------------------------------------
    std::size_t reasonSize{0};
    const bool decodedReasonSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, reasonSize);

    if (!decodedReasonSize) {
        LogError()(__func__)(": CompactSize incomplete for reason field")
            .Flush();

        return nullptr;
    }
    // -----------------------------------------------
    expectedSize += reasonSize;

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for reason field")
            .Flush();

        return nullptr;
    }
    const UnallocatedCString reason{
        reinterpret_cast<const char*>(it), reasonSize};
    it += reasonSize;
    // -----------------------------------------------
    // This next field is "sometimes there".
    // Sometimes it's there (or not) for a single code!
    // Because the code means different things depending on
    // what got rejected.
    //
    auto extra = ByteArray{};

    // better than hardcoding "32"
    expectedSize += sizeof(bitcoin::BlockHeaderHashField);

    if (expectedSize <= size) {
        extra.Concatenate(it, sizeof(bitcoin::BlockHeaderHashField));
        std::advance(it, sizeof(bitcoin::BlockHeaderHashField));
    }
    // -----------------------------------------------
    try {
        return new ReturnType(
            api,
            std::move(pHeader),
            message,
            static_cast<bitcoin::RejectCode>(code),
            reason,
            extra);
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

auto BitcoinP2PReject(
    const api::Session& api,
    const blockchain::Type network,
    const UnallocatedCString& message,
    const std::uint8_t code,
    const UnallocatedCString& reason,
    const Data& extra) -> blockchain::p2p::bitcoin::message::Reject*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Reject;

    return new ReturnType(
        api,
        network,
        message,
        static_cast<bitcoin::RejectCode>(code),
        reason,
        extra);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{

Reject::Reject(
    const api::Session& api,
    const blockchain::Type network,
    const UnallocatedCString& message,
    const bitcoin::RejectCode code,
    const UnallocatedCString& reason,
    const Data& extra) noexcept
    : Message(api, network, bitcoin::Command::reject)
    , message_(message)
    , code_(code)
    , reason_(reason)
    , extra_(extra)
{
    init_hash();
}

Reject::Reject(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const UnallocatedCString& message,
    const bitcoin::RejectCode code,
    const UnallocatedCString& reason,
    const Data& extra) noexcept(false)
    : Message(api, std::move(header))
    , message_(message)
    , code_(code)
    , reason_(reason)
    , extra_(extra)
{
    verify_checksum();
}

auto Reject::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto message = BitcoinString(message_);
        const auto reason = BitcoinString(reason_);
        const auto code =
            be::little_uint8_buf_t{static_cast<std::uint8_t>(code_)};
        const auto bytes =
            message.size() + reason.size() + sizeof(code) + extra_.size();
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, message.data(), message.size());
        std::advance(i, message.size());
        std::memcpy(i, static_cast<const void*>(&code), sizeof(code));
        std::advance(i, sizeof(code));
        std::memcpy(i, reason.data(), reason.size());
        std::advance(i, reason.size());

        if (false == extra_.empty()) {
            std::memcpy(i, extra_.data(), extra_.size());
            std::advance(i, extra_.size());
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
