// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/response/ListNyms.hpp"        // IWYU pragma: associated
#include "opentxs/rpc/response/MessagePrivate.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/rpc/request/ListNyms.hpp"

namespace opentxs::rpc::response::implementation
{
struct ListNyms final : public Message::Imp {
    auto asListNyms() const noexcept -> const response::ListNyms& final
    {
        return static_cast<const response::ListNyms&>(*parent_);
    }
    auto serialize(proto::RPCResponse& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            serialize_identifiers(dest);

            return true;
        }

        return false;
    }

    ListNyms(
        const response::ListNyms* parent,
        const request::ListNyms& request,
        Message::Responses&& response,
        Message::Identifiers&& accounts) noexcept(false)
        : Imp(parent, request, std::move(response), std::move(accounts))
    {
    }
    ListNyms(
        const response::ListNyms* parent,
        const proto::RPCResponse& in) noexcept(false)
        : Imp(parent, in)
    {
    }
    ListNyms() = delete;
    ListNyms(const ListNyms&) = delete;
    ListNyms(ListNyms&&) = delete;
    auto operator=(const ListNyms&) -> ListNyms& = delete;
    auto operator=(ListNyms&&) -> ListNyms& = delete;

    ~ListNyms() final = default;
};
}  // namespace opentxs::rpc::response::implementation

namespace opentxs::rpc::response
{
ListNyms::ListNyms(
    const request::ListNyms& request,
    Responses&& response,
    Identifiers&& accounts)
    : Message(std::make_unique<implementation::ListNyms>(
          this,
          request,
          std::move(response),
          std::move(accounts)))
{
}

ListNyms::ListNyms(const proto::RPCResponse& serialized) noexcept(false)
    : Message(std::make_unique<implementation::ListNyms>(this, serialized))
{
}

ListNyms::ListNyms() noexcept
    : Message()
{
}

auto ListNyms::NymIDs() const noexcept -> const Identifiers&
{
    return imp_->identifiers_;
}

ListNyms::~ListNyms() = default;
}  // namespace opentxs::rpc::response
