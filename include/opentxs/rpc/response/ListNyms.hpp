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
class ListNyms;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
class OPENTXS_EXPORT ListNyms final : public Message
{
public:
    auto NymIDs() const noexcept -> const Identifiers&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT ListNyms(
        const request::ListNyms& request,
        Responses&& response,
        Identifiers&& accounts) noexcept(false);
    OPENTXS_NO_EXPORT ListNyms(
        const protobuf::RPCResponse& serialized) noexcept(false);
    ListNyms() noexcept;
    ListNyms(const ListNyms&) = delete;
    ListNyms(ListNyms&&) = delete;
    auto operator=(const ListNyms&) -> ListNyms& = delete;
    auto operator=(ListNyms&&) -> ListNyms& = delete;

    ~ListNyms() final;
};
}  // namespace opentxs::rpc::response
