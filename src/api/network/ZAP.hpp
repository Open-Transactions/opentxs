// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/network/ZAP.hpp"

#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Handler.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Factory;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::imp
{
class ZAP final : virtual public api::network::ZAP
{
public:
    auto RegisterDomain(
        const UnallocatedCString& domain,
        const Callback& callback) const -> bool final;
    auto SetDefaultPolicy(const Policy policy) const -> bool final;

    ~ZAP() final = default;

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& context_;
    OTZMQZAPCallback callback_;
    OTZMQZAPHandler zap_;

    ZAP(const opentxs::network::zeromq::Context& context);
    ZAP() = delete;
    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    auto operator=(const ZAP&) -> ZAP& = delete;
    auto operator=(ZAP&&) -> ZAP& = delete;
};
}  // namespace opentxs::api::network::imp
