// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
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
class OPENTXS_EXPORT ListNyms final : public Base
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    /// throws std::runtime_error for invalid constructor arguments
    ListNyms(SessionIndex session, const AssociateNyms& nyms = {}) noexcept(
        false);
    OPENTXS_NO_EXPORT ListNyms(const proto::RPCCommand& serialized) noexcept(
        false);
    ListNyms() noexcept;
    ListNyms(const ListNyms&) = delete;
    ListNyms(ListNyms&&) = delete;
    auto operator=(const ListNyms&) -> ListNyms& = delete;
    auto operator=(ListNyms&&) -> ListNyms& = delete;

    ~ListNyms() final;
};
}  // namespace opentxs::rpc::request
