// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "interface/rpc/Types.internal.hpp"
#include "opentxs/interface/rpc/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class RPCCommand;
class RPCResponse;
}  // namespace proto

namespace rpc
{
namespace request
{
class Base;
}  // namespace request

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{

}  // namespace opentxs

namespace opentxs::rpc::internal
{
struct RPC {
    virtual auto Process(const proto::RPCCommand& command) const
        -> proto::RPCResponse = 0;
    virtual auto Process(const request::Base& command) const
        -> std::unique_ptr<response::Base> = 0;

    virtual ~RPC() = default;
};
}  // namespace opentxs::rpc::internal
