// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include "internal/network/zeromq/message/Message.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class Message::Imp : virtual public internal::Message
{
public:
    zeromq::Message* parent_;

    auto Envelope() const noexcept -> std::span<const Frame> final;
    auto get() const noexcept -> std::span<const Frame> { return frames_; }
    auto Payload() const noexcept -> std::span<const Frame>;
    auto size() const noexcept -> std::size_t;

    auto AddFrame() noexcept -> Frame&;
    auto AddFrame(const Amount& amount) noexcept -> Frame&;
    auto AddFrame(Frame&& frame) noexcept -> Frame&;
    auto AddFrame(const char* in) noexcept -> Frame&;
    auto AddFrame(const ProtobufType& input) noexcept -> Frame& final;
    auto AddFrame(const ReadView bytes) noexcept -> Frame&;
    auto AddFrame(const void* input, const std::size_t size) noexcept -> Frame&;
    auto AppendBytes() noexcept -> Writer;
    auto EnsureDelimiter() noexcept -> void final;
    auto ExtractFront() noexcept -> zeromq::Frame;
    auto get() noexcept -> std::span<Frame> { return frames_; }
    auto Envelope() noexcept -> std::span<Frame> final;
    auto Payload() noexcept -> std::span<Frame>;
    auto Prepend(ReadView frame) noexcept -> zeromq::Frame&;
    auto Prepend(zeromq::Frame frame) noexcept -> zeromq::Frame&;
    auto Prepend(SocketID id) noexcept -> zeromq::Frame& final;
    auto StartBody() noexcept -> void;

    Imp() noexcept;
    Imp(const Imp& rhs) noexcept;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() override = default;

protected:
    UnallocatedVector<Frame> frames_;

    auto set_field(
        const std::size_t position,
        const zeromq::Frame& input) noexcept -> bool;

private:
    std::optional<std::size_t> delimiter_;
};
}  // namespace opentxs::network::zeromq
