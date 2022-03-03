// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "interface/rpc/request/Base.hpp"  // IWYU pragma: associated
#include "opentxs/interface/rpc/request/ListAccounts.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/interface/rpc/CommandType.hpp"

namespace opentxs::rpc::request::implementation
{
struct ListAccounts final : public Base::Imp {
    auto asListAccounts() const noexcept -> const request::ListAccounts& final
    {
        return static_cast<const request::ListAccounts&>(*parent_);
    }
    auto serialize(proto::RPCCommand& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            serialize_owner(dest);
            serialize_notary(dest);
            serialize_unit(dest);

            return true;
        }

        return false;
    }

    ListAccounts(
        const request::ListAccounts* parent,
        VersionNumber version,
        Base::SessionIndex session,
        const UnallocatedCString& filterNym,
        const UnallocatedCString& filterNotary,
        const UnallocatedCString& filterUnit,
        const Base::AssociateNyms& nyms) noexcept(false)
        : Imp(parent,
              CommandType::list_accounts,
              version,
              session,
              filterNym,
              filterNotary,
              filterUnit,
              nyms)
    {
        check_session();
    }
    ListAccounts(
        const request::ListAccounts* parent,
        const proto::RPCCommand& in) noexcept(false)
        : Imp(parent, in)
    {
        check_session();
    }

    ~ListAccounts() final = default;

private:
    ListAccounts() = delete;
    ListAccounts(const ListAccounts&) = delete;
    ListAccounts(ListAccounts&&) = delete;
    auto operator=(const ListAccounts&) -> ListAccounts& = delete;
    auto operator=(ListAccounts&&) -> ListAccounts& = delete;
};
}  // namespace opentxs::rpc::request::implementation

namespace opentxs::rpc::request
{
ListAccounts::ListAccounts(
    SessionIndex session,
    const UnallocatedCString& filterNym,
    const UnallocatedCString& filterNotary,
    const UnallocatedCString& filterUnit,
    const AssociateNyms& nyms)
    : Base(std::make_unique<implementation::ListAccounts>(
          this,
          DefaultVersion(),
          session,
          filterNym,
          filterNotary,
          filterUnit,
          nyms))
{
}

ListAccounts::ListAccounts(const proto::RPCCommand& in) noexcept(false)
    : Base(std::make_unique<implementation::ListAccounts>(this, in))
{
}

ListAccounts::ListAccounts() noexcept
    : Base()
{
}

auto ListAccounts::DefaultVersion() noexcept -> VersionNumber { return 3u; }

auto ListAccounts::FilterNotary() const noexcept -> const UnallocatedCString&
{
    return imp_->notary_;
}

auto ListAccounts::FilterNym() const noexcept -> const UnallocatedCString&
{
    return imp_->owner_;
}

auto ListAccounts::FilterUnit() const noexcept -> const UnallocatedCString&
{
    return imp_->unit_;
}

ListAccounts::~ListAccounts() = default;
}  // namespace opentxs::rpc::request
