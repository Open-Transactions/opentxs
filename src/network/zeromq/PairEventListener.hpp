// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/zeromq/socket/Subscribe.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class PairEventListener final : public zeromq::socket::implementation::Subscribe
{
public:
    ~PairEventListener() final = default;

private:
    friend zeromq::implementation::Context;
    using ot_super = socket::implementation::Subscribe;

    const int instance_;

    auto clone() const noexcept -> PairEventListener* final;

    PairEventListener(
        const zeromq::Context& context,
        const zeromq::PairEventCallback& callback,
        const int instance);
    PairEventListener() = delete;
    PairEventListener(const PairEventListener&) = delete;
    PairEventListener(PairEventListener&&) = delete;
    auto operator=(const PairEventListener&) -> PairEventListener& = delete;
    auto operator=(PairEventListener&&) -> PairEventListener& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
