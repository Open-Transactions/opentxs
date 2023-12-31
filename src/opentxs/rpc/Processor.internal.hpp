// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class RPCCommand;
class RPCResponse;
}  // namespace protobuf

namespace rpc
{
namespace request
{
class Message;
}  // namespace request

namespace response
{
class Message;
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc
{
class Processor
{
public:
    virtual auto Process(const protobuf::RPCCommand& command) const noexcept
        -> protobuf::RPCResponse;
    virtual auto Process(const request::Message& command) const noexcept
        -> std::unique_ptr<response::Message>;

    virtual ~Processor() = default;
};
}  // namespace opentxs::rpc
