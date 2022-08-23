// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>

#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class Node final : public Allocated
{
public:
    class Actor;
    class Shared;

    auto get_allocator() const noexcept -> allocator_type final;

    auto Init(std::shared_ptr<const api::Session> api) noexcept -> void;

    Node(const api::Session& api) noexcept;
    Node() = delete;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    auto operator=(const Node&) -> Node& = delete;
    auto operator=(Node&&) -> Node& = delete;

    ~Node() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::network::otdht
