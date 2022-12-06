// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/rpc/response/Base.hpp"  // IWYU pragma: associated
#include "opentxs/interface/rpc/response/ListAccounts.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/interface/rpc/request/ListAccounts.hpp"

namespace opentxs::rpc::response::implementation
{
struct ListAccounts final : public Base::Imp {
    auto asListAccounts() const noexcept -> const response::ListAccounts& final
    {
        return static_cast<const response::ListAccounts&>(*parent_);
    }
    auto serialize(proto::RPCResponse& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            serialize_identifiers(dest);

            return true;
        }

        return false;
    }

    ListAccounts(
        const response::ListAccounts* parent,
        const request::ListAccounts& request,
        Base::Responses&& response,
        Base::Identifiers&& accounts) noexcept(false)
        : Imp(parent, request, std::move(response), std::move(accounts))
    {
    }
    ListAccounts(
        const response::ListAccounts* parent,
        const proto::RPCResponse& in) noexcept(false)
        : Imp(parent, in)
    {
    }
    ListAccounts() = delete;
    ListAccounts(const ListAccounts&) = delete;
    ListAccounts(ListAccounts&&) = delete;
    auto operator=(const ListAccounts&) -> ListAccounts& = delete;
    auto operator=(ListAccounts&&) -> ListAccounts& = delete;

    ~ListAccounts() final = default;
};
}  // namespace opentxs::rpc::response::implementation

namespace opentxs::rpc::response
{
ListAccounts::ListAccounts(
    const request::ListAccounts& request,
    Responses&& response,
    Identifiers&& accounts)
    : Base(std::make_unique<implementation::ListAccounts>(
          this,
          request,
          std::move(response),
          std::move(accounts)))
{
}

ListAccounts::ListAccounts(const proto::RPCResponse& serialized) noexcept(false)
    : Base(std::make_unique<implementation::ListAccounts>(this, serialized))
{
}

ListAccounts::ListAccounts() noexcept
    : Base()
{
}

auto ListAccounts::AccountIDs() const noexcept -> const Identifiers&
{
    return imp_->identifiers_;
}

ListAccounts::~ListAccounts() = default;
}  // namespace opentxs::rpc::response
