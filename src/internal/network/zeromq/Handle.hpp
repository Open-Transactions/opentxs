// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
}  // namespace internal

class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Handle
{
private:
    std::shared_ptr<internal::Batch> batch_p_;

public:
    internal::Batch& batch_;

    auto Release() noexcept -> void;

    Handle(
        std::shared_ptr<const zeromq::Context> context,
        std::shared_ptr<internal::Batch> batch) noexcept;
    Handle(const Handle&) = delete;
    Handle(Handle&& rhs) noexcept;
    auto operator=(const Handle&) -> Handle& = delete;
    auto operator=(Handle&&) -> Handle& = delete;

    ~Handle();

private:
    std::shared_ptr<const zeromq::Context> context_;
};
}  // namespace opentxs::network::zeromq::internal
