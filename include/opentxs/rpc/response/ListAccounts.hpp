// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/rpc/response/Message.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class RPCResponse;
}  // namespace protobuf

namespace rpc
{
namespace request
{
class ListAccounts;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
class OPENTXS_EXPORT ListAccounts final : public Message
{
public:
    auto AccountIDs() const noexcept -> const Identifiers&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT ListAccounts(
        const request::ListAccounts& request,
        Responses&& response,
        Identifiers&& accounts) noexcept(false);
    OPENTXS_NO_EXPORT ListAccounts(
        const protobuf::RPCResponse& serialized) noexcept(false);
    ListAccounts() noexcept;
    ListAccounts(const ListAccounts&) = delete;
    ListAccounts(ListAccounts&&) = delete;
    auto operator=(const ListAccounts&) -> ListAccounts& = delete;
    auto operator=(ListAccounts&&) -> ListAccounts& = delete;

    ~ListAccounts() final;
};
}  // namespace opentxs::rpc::response
