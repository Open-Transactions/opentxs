// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/zap/ZAP.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Reply;
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::zap
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
class Reply : public zeromq::Message
{
public:
    class Imp;

    auto Code() const noexcept -> zap::Status;
    auto Debug() const noexcept -> UnallocatedCString;
    auto Metadata() const noexcept -> ReadView;
    auto RequestID() const noexcept -> ReadView;
    auto Status() const noexcept -> ReadView;
    auto UserID() const noexcept -> ReadView;
    auto Version() const noexcept -> ReadView;

    using Message::swap;
    auto swap(Message& rhs) noexcept -> void override;
    auto swap(Reply& rhs) noexcept -> void;

    Reply(Imp* imp) noexcept;
    Reply() = delete;
    Reply(const Reply&) noexcept;
    Reply(Reply&&) noexcept;
    auto operator=(const Reply&) noexcept -> Reply&;
    auto operator=(Reply&&) noexcept -> Reply&;

    ~Reply() override;

protected:
    Imp* imp_;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::network::zeromq::zap
