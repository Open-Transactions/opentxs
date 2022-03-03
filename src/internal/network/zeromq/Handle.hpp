// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Context;
}  // namespace internal
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Handle
{
public:
    internal::Batch& batch_;

    auto Release() noexcept -> void;

    Handle(const internal::Context& context, internal::Batch& batch) noexcept;
    Handle(const Handle&) = delete;
    Handle(Handle&& rhs) noexcept;
    auto operator=(const Handle&) -> Handle& = delete;
    auto operator=(Handle&&) -> Handle& = delete;

    ~Handle();

private:
    const internal::Context* context_;
};
}  // namespace opentxs::network::zeromq::internal
