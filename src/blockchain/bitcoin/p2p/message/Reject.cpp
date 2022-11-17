// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

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
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinP2PReject(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Reject*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::Reject;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        const auto msgSize = decode_compact_size(bytes, "message size");
        auto message = UnallocatedCString{};
        deserialize(bytes, writer(message), msgSize, "message");
        auto code = be::little_uint8_buf_t{};
        deserialize_object(bytes, code, "reject code");
        const auto reasonSize = decode_compact_size(bytes, "reason size");
        auto reason = UnallocatedCString{};
        deserialize(bytes, writer(reason), reasonSize, "reason");
        // This next field is "sometimes there". Sometimes it's there (or not)
        // for a single code! Because the code means different things depending
        // on what got rejected.
        auto extra = ByteArray{extract_prefix(bytes, bytes.size(), "")};
        check_finished(bytes);

        return new ReturnType(
            api,
            std::move(pHeader),
            std::move(message),
            static_cast<bitcoin::RejectCode>(code.value()),
            std::move(reason),
            std::move(extra));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
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
