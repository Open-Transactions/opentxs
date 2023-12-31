// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/request/GetAccountActivity.hpp"  // IWYU pragma: associated
#include "opentxs/rpc/request/MessagePrivate.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/rpc/CommandType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"

namespace opentxs::rpc::request::implementation
{
struct GetAccountActivity final : public Message::Imp {
    auto asGetAccountActivity() const noexcept
        -> const request::GetAccountActivity& final
    {
        return static_cast<const request::GetAccountActivity&>(*parent_);
    }
    auto serialize(protobuf::RPCCommand& dest) const noexcept -> bool final
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
        SessionIndex session,
        const Message::Identifiers& accounts,
        const Message::AssociateNyms& nyms) noexcept(false)
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
        const protobuf::RPCCommand& in) noexcept(false)
        : Imp(parent, in)
    {
        check_session();
        check_identifiers();
    }
    GetAccountActivity() = delete;
    GetAccountActivity(const GetAccountActivity&) = delete;
    GetAccountActivity(GetAccountActivity&&) = delete;
    auto operator=(const GetAccountActivity&) -> GetAccountActivity& = delete;
    auto operator=(GetAccountActivity&&) -> GetAccountActivity& = delete;

    ~GetAccountActivity() final = default;
};
}  // namespace opentxs::rpc::request::implementation

namespace opentxs::rpc::request
{
GetAccountActivity::GetAccountActivity(
    SessionIndex session,
    const Identifiers& accounts,
    const AssociateNyms& nyms)
    : Message(std::make_unique<implementation::GetAccountActivity>(
          this,
          DefaultVersion(),
          session,
          accounts,
          nyms))
{
}

GetAccountActivity::GetAccountActivity(const protobuf::RPCCommand& in) noexcept(
    false)
    : Message(std::make_unique<implementation::GetAccountActivity>(this, in))
{
}

GetAccountActivity::GetAccountActivity() noexcept
    : Message()
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
