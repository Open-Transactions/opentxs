// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "interface/rpc/request/Base.hpp"  // IWYU pragma: associated
#include "opentxs/interface/rpc/request/ListNyms.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/interface/rpc/CommandType.hpp"

namespace opentxs::rpc::request::implementation
{
struct ListNyms final : public Base::Imp {
    auto asListNyms() const noexcept -> const request::ListNyms& final
    {
        return static_cast<const request::ListNyms&>(*parent_);
    }
    ListNyms(
        const request::ListNyms* parent,
        VersionNumber version,
        Base::SessionIndex session,
        const Base::AssociateNyms& nyms) noexcept(false)
        : Imp(parent, CommandType::list_nyms, version, session, nyms)
    {
        check_session();
    }
    ListNyms(
        const request::ListNyms* parent,
        const proto::RPCCommand& in) noexcept(false)
        : Imp(parent, in)
    {
        check_session();
    }
    ListNyms() = delete;
    ListNyms(const ListNyms&) = delete;
    ListNyms(ListNyms&&) = delete;
    auto operator=(const ListNyms&) -> ListNyms& = delete;
    auto operator=(ListNyms&&) -> ListNyms& = delete;

    ~ListNyms() final = default;
};
}  // namespace opentxs::rpc::request::implementation

namespace opentxs::rpc::request
{
ListNyms::ListNyms(SessionIndex session, const AssociateNyms& nyms)
    : Base(std::make_unique<implementation::ListNyms>(
          this,
          DefaultVersion(),
          session,
          nyms))
{
}

ListNyms::ListNyms(const proto::RPCCommand& in) noexcept(false)
    : Base(std::make_unique<implementation::ListNyms>(this, in))
{
}

ListNyms::ListNyms() noexcept
    : Base()
{
}

auto ListNyms::DefaultVersion() noexcept -> VersionNumber { return 3u; }

ListNyms::~ListNyms() = default;
}  // namespace opentxs::rpc::request
