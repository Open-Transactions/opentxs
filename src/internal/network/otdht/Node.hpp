// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/util/PMR.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Secret;
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

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Init(std::shared_ptr<const api::Session> api) noexcept -> void;

    Node(
        const api::Session& api,
        const ReadView publicKey,
        const Secret& secretKey) noexcept;
    Node() = delete;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    auto operator=(const Node&) -> Node& = delete;
    auto operator=(Node&&) -> Node& = delete;

    ~Node() final;

private:
    std::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::network::otdht
