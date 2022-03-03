// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "interface/rpc/request/Base.hpp"  // IWYU pragma: associated
#include "opentxs/interface/rpc/request/GetAccountActivity.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/interface/rpc/CommandType.hpp"

namespace opentxs::rpc::request::implementation
{
struct GetAccountActivity final : public Base::Imp {
    auto asGetAccountActivity() const noexcept
        -> const request::GetAccountActivity& final
    {
        return static_cast<const request::GetAccountActivity&>(*parent_);
    }
    auto serialize(proto::RPCCommand& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            serialize_identifiers(dest);

            return true;
        }

        return false;
    }

    GetAccountActivity(
        const request::GetAccountActivity* parent,
        VersionNumber version,
        Base::SessionIndex session,
        const Base::Identifiers& accounts,
        const Base::AssociateNyms& nyms) noexcept(false)
        : Imp(parent,
              CommandType::get_account_activity,
              version,
              session,
              accounts,
              nyms)
    {
        check_session();
        check_identifiers();
    }
    GetAccountActivity(
        const request::GetAccountActivity* parent,
        const proto::RPCCommand& in) noexcept(false)
        : Imp(parent, in)
    {
        check_session();
        check_identifiers();
    }

    ~GetAccountActivity() final = default;

private:
    GetAccountActivity() = delete;
    GetAccountActivity(const GetAccountActivity&) = delete;
    GetAccountActivity(GetAccountActivity&&) = delete;
    auto operator=(const GetAccountActivity&) -> GetAccountActivity& = delete;
    auto operator=(GetAccountActivity&&) -> GetAccountActivity& = delete;
};
}  // namespace opentxs::rpc::request::implementation

namespace opentxs::rpc::request
{
GetAccountActivity::GetAccountActivity(
    SessionIndex session,
    const Identifiers& accounts,
    const AssociateNyms& nyms)
    : Base(std::make_unique<implementation::GetAccountActivity>(
          this,
          DefaultVersion(),
          session,
          accounts,
          nyms))
{
}

GetAccountActivity::GetAccountActivity(const proto::RPCCommand& in) noexcept(
    false)
    : Base(std::make_unique<implementation::GetAccountActivity>(this, in))
{
}

GetAccountActivity::GetAccountActivity() noexcept
    : Base()
{
}

auto GetAccountActivity::Accounts() const noexcept -> const Identifiers&
{
    return imp_->identifiers_;
}

auto GetAccountActivity::DefaultVersion() noexcept -> VersionNumber
{
    return 3u;
}

GetAccountActivity::~GetAccountActivity() = default;
}  // namespace opentxs::rpc::request
