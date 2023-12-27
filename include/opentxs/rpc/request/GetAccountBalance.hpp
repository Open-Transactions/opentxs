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
class OPENTXS_EXPORT GetAccountBalance final : public Message
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    auto Accounts() const noexcept -> const Identifiers&;

    /// throws std::runtime_error for invalid constructor arguments
    GetAccountBalance(
        SessionIndex session,
        const Identifiers& accounts,
        const AssociateNyms& nyms = {}) noexcept(false);
    OPENTXS_NO_EXPORT GetAccountBalance(
        const proto::RPCCommand& serialized) noexcept(false);
    GetAccountBalance() noexcept;
    GetAccountBalance(const GetAccountBalance&) = delete;
    GetAccountBalance(GetAccountBalance&&) = delete;
    auto operator=(const GetAccountBalance&) -> GetAccountBalance& = delete;
    auto operator=(GetAccountBalance&&) -> GetAccountBalance& = delete;

    ~GetAccountBalance() final;
};
}  // namespace opentxs::rpc::request
