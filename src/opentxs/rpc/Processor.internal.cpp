// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/Processor.internal.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCResponse.pb.h>

#include "opentxs/rpc/response/Message.hpp"  // IWYU pragma: keep

namespace opentxs::rpc
{
auto Processor::Process(const protobuf::RPCCommand& command) const noexcept
    -> protobuf::RPCResponse
{
    return {};
}

auto Processor::Process(const request::Message& command) const noexcept
    -> std::unique_ptr<response::Message>
{
    return {};
}
}  // namespace opentxs::rpc
