// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/base/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/network/blockchain/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Header.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::blockchain::bitcoin::message::implementation
{
Message::Message(
    const api::Session& api,
    opentxs::blockchain::Type chain,
    message::Command command,
    std::optional<ByteArray> checksum,
    allocator_type alloc) noexcept
    : MessagePrivate(alloc)
    , api_(api)
    , chain_(chain)
    , command_(command)
    , description_(CString{print(chain), alloc}
                       .append(" ")
                       .append(print(command_))
                       .append(" message"))
    , checksum_(std::move(checksum))
{
}

Message::Message(
    const api::Session& api,
    opentxs::blockchain::Type chain,
    ReadView description,
    std::optional<ByteArray> checksum,
    allocator_type alloc) noexcept
    : MessagePrivate(alloc)
    , api_(api)
    , chain_(chain)
    , command_(message::Command::unknown)
    , description_(description, alloc)
    , checksum_(std::move(checksum))
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : MessagePrivate(rhs, alloc)
    , api_(rhs.api_)
    , chain_(rhs.chain_)
    , command_(rhs.command_)
    , description_(rhs.description_, alloc)
    , checksum_(rhs.checksum_)
{
}

auto Message::get_payload(Transport, WriteBuffer&) const noexcept(false) -> void
{
}

auto Message::get_size() const noexcept -> std::size_t { return 0_uz; }

auto Message::header(const ReadView payload) const noexcept -> internal::Header
{
    if (checksum_.has_value()) {

        return internal::Header{chain_, command_, payload, checksum_->Bytes()};
    } else {
        auto out = internal::Header{api_, chain_, command_, payload};
        checksum_.emplace(out.Checksum());

        return out;
    }
}

auto Message::Transmit(Transport type, zeromq::Message& out) const
    noexcept(false) -> void
{
    using enum Transport;

    switch (type) {
        case zmq: {
            transmit_zmq(type, out);
        } break;
        case ipv6:
        case ipv4:
        case onion2:
        case onion3:
        case eep:
        case cjdns:
        default: {
            transmit_asio(type, out);
        }
    }
}

auto Message::transmit_asio(Transport type, zeromq::Message& out) const
    noexcept(false) -> void
{
    // NOTE the header is serialized first but it can not be calculated until
    // the payload is calculated so we have to serialize the payload and header
    // in reverse order
    constexpr auto hSize = internal::Header::size_;
    const auto pSize = get_size();
    const auto total = hSize + pSize;
    auto buf = reserve(out.AppendBytes(), total, "p2p message");
    auto headerWriter = buf.Write(hSize);
    const auto payloadView = ReadView{buf};
    get_payload(type, buf);
    header(payloadView).Serialize(std::move(headerWriter));
    check_finished(buf);
}

auto Message::transmit_zmq(Transport type, zeromq::Message& out) const
    noexcept(false) -> void
{
    encode(chain_, out);
    SerializeCommand(command_, out);
    auto buf = reserve(out.AppendBytes(), get_size(), "p2p message");
    get_payload(type, buf);
    check_finished(buf);
}
}  // namespace opentxs::network::blockchain::bitcoin::message::implementation
