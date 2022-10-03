// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "network/zeromq/socket/Subscribe.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace implementation
{
class Context;
}  // namespace implementation

class Context;
class PairEventCallback;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class PairEventListener final : public zeromq::socket::implementation::Subscribe
{
public:
    PairEventListener() = delete;
    PairEventListener(const PairEventListener&) = delete;
    PairEventListener(PairEventListener&&) = delete;
    auto operator=(const PairEventListener&) -> PairEventListener& = delete;
    auto operator=(PairEventListener&&) -> PairEventListener& = delete;

    ~PairEventListener() final = default;

private:
    friend zeromq::implementation::Context;
    using ot_super = socket::implementation::Subscribe;

    const int instance_;

    auto clone() const noexcept -> PairEventListener* final;

    PairEventListener(
        const zeromq::Context& context,
        const zeromq::PairEventCallback& callback,
        const int instance,
        const std::string_view threadname = "PairEventListener");
};
}  // namespace opentxs::network::zeromq::implementation
