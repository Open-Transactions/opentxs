// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/interface/rpc/response/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace rpc
{
namespace request
{
class Base;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
struct Invalid final : Base {
    Invalid(const request::Base& request) noexcept;
    Invalid() = delete;
    Invalid(const Invalid&) = delete;
    Invalid(Invalid&&) = delete;
    auto operator=(const Invalid&) -> Invalid& = delete;
    auto operator=(Invalid&&) -> Invalid& = delete;

    ~Invalid() final;
};
}  // namespace opentxs::rpc::response
