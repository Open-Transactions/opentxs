// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/request/Message.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::request
{
class OPENTXS_EXPORT GetAccountActivity final : public Message
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    auto Accounts() const noexcept -> const Identifiers&;

    /// throws std::runtime_error for invalid constructor arguments
    GetAccountActivity(
        SessionIndex session,
        const Identifiers& accounts,
        const AssociateNyms& nyms = {}) noexcept(false);
    OPENTXS_NO_EXPORT GetAccountActivity(
        const proto::RPCCommand& serialized) noexcept(false);
    GetAccountActivity() noexcept;
    GetAccountActivity(const GetAccountActivity&) = delete;
    GetAccountActivity(GetAccountActivity&&) = delete;
    auto operator=(const GetAccountActivity&) -> GetAccountActivity& = delete;
    auto operator=(GetAccountActivity&&) -> GetAccountActivity& = delete;

    ~GetAccountActivity() final;
};
}  // namespace opentxs::rpc::request
