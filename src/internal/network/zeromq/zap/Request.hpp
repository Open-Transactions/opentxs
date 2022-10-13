// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <tuple>

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
class Request : public zeromq::Message
{
public:
    class Imp;

    auto Address() const noexcept -> ReadView;
    auto Credentials() const noexcept -> const FrameSection;
    auto Debug() const noexcept -> UnallocatedCString;
    auto Domain() const noexcept -> ReadView;
    auto Identity() const noexcept -> ReadView;
    auto Mechanism() const noexcept -> zap::Mechanism;
    auto RequestID() const noexcept -> ReadView;
    auto Validate() const noexcept -> std::pair<bool, UnallocatedCString>;
    auto Version() const noexcept -> ReadView;

    using Message::swap;
    auto swap(Message& rhs) noexcept -> void override;
    auto swap(Request& rhs) noexcept -> void;

    Request() noexcept;
    Request(Imp* imp) noexcept;
    Request(const Request&) noexcept;
    Request(Request&&) noexcept;
    auto operator=(const Request&) noexcept -> Request&;
    auto operator=(Request&&) noexcept -> Request&;

    ~Request() override;

protected:
    Imp* imp_;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::network::zeromq::zap
