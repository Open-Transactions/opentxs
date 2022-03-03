// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/zeromq/socket/Socket.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket::implementation
{
template <typename Interface, typename ImplementationParent = Socket>
class Sender : virtual public Interface, virtual public ImplementationParent
{
protected:
    auto Send(zeromq::Message&& data) const noexcept -> bool override;

    Sender() noexcept;

    ~Sender() override = default;

private:
    Sender(const Sender&) = delete;
    Sender(Sender&&) = delete;
    auto operator=(const Sender&) -> Sender& = delete;
    auto operator=(Sender&&) -> Sender& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
