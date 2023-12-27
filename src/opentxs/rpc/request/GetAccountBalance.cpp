// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/request/GetAccountBalance.hpp"  // IWYU pragma: associated
#include "opentxs/rpc/request/MessagePrivate.hpp"     // IWYU pragma: associated

#include <memory>

#include "opentxs/rpc/CommandType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"

namespace opentxs::rpc::request::implementation
{
struct GetAccountBalance final : public Message::Imp {
    auto asGetAccountBalance() const noexcept
        -> const request::GetAccountBalance& final
    {
        return static_cast<const request::GetAccountBalance&>(*parent_);
    }
    auto serialize(proto::RPCCommand& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            serialize_identifiers(dest);

            return true;
        }

        return false;
    }

    GetAccountBalance(
        const request::GetAccountBalance* parent,
        VersionNumber version,
        SessionIndex session,
        const Message::Identifiers& accounts,
        const Message::AssociateNyms& nyms) noexcept(false)
        : Imp(parent,
              CommandType::get_account_balance,
              version,
              session,
              accounts,
              nyms)
    {
        check_session();
        check_identifiers();
    }
    GetAccountBalance(
        const request::GetAccountBalance* parent,
        const proto::RPCCommand& in) noexcept(false)
        : Imp(parent, in)
    {
        check_session();
        check_identifiers();
    }
    GetAccountBalance() = delete;
    GetAccountBalance(const GetAccountBalance&) = delete;
    GetAccountBalance(GetAccountBalance&&) = delete;
    auto operator=(const GetAccountBalance&) -> GetAccountBalance& = delete;
    auto operator=(GetAccountBalance&&) -> GetAccountBalance& = delete;

    ~GetAccountBalance() final = default;
};
}  // namespace opentxs::rpc::request::implementation

namespace opentxs::rpc::request
{
GetAccountBalance::GetAccountBalance(
    SessionIndex session,
    const Identifiers& accounts,
    const AssociateNyms& nyms)
    : Message(std::make_unique<implementation::GetAccountBalance>(
          this,
          DefaultVersion(),
          session,
          accounts,
          nyms))
{
}

GetAccountBalance::GetAccountBalance(const proto::RPCCommand& in) noexcept(
    false)
    : Message(std::make_unique<implementation::GetAccountBalance>(this, in))
{
}

GetAccountBalance::GetAccountBalance() noexcept
    : Message()
{
}

auto GetAccountBalance::Accounts() const noexcept -> const Identifiers&
{
    return imp_->identifiers_;
}

auto GetAccountBalance::DefaultVersion() noexcept -> VersionNumber
{
    return 3u;
}

GetAccountBalance::~GetAccountBalance() = default;
}  // namespace opentxs::rpc::request
