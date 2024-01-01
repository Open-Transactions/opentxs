// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class RPCCommand;
}  // namespace protobuf

namespace rpc
{
namespace request
{
class Message;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto RPCRequest(ReadView protobuf) noexcept
    -> std::unique_ptr<rpc::request::Message>;
auto RPCRequest(const protobuf::RPCCommand& serialized) noexcept
    -> std::unique_ptr<rpc::request::Message>;
}  // namespace opentxs::factory
