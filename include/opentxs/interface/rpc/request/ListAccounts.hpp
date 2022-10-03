// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
#include "opentxs/util/Container.hpp"
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
class OPENTXS_EXPORT ListAccounts final : public Base
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    auto FilterNotary() const noexcept -> const UnallocatedCString&;
    auto FilterNym() const noexcept -> const UnallocatedCString&;
    auto FilterUnit() const noexcept -> const UnallocatedCString&;

    /// throws std::runtime_error for invalid constructor arguments
    ListAccounts(
        SessionIndex session,
        const UnallocatedCString& filterNym = {},
        const UnallocatedCString& filterNotary = {},
        const UnallocatedCString& filterUnit = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    ListAccounts(const proto::RPCCommand& serialized) noexcept(false);
    ListAccounts() noexcept;
    ListAccounts(const ListAccounts&) = delete;
    ListAccounts(ListAccounts&&) = delete;
    auto operator=(const ListAccounts&) -> ListAccounts& = delete;
    auto operator=(ListAccounts&&) -> ListAccounts& = delete;

    ~ListAccounts() final;
};
}  // namespace opentxs::rpc::request
