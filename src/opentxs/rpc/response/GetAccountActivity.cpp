// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/response/GetAccountActivity.hpp"  // IWYU pragma: associated
#include "opentxs/rpc/response/MessagePrivate.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCResponse.pb.h>
#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

#include "opentxs/rpc/AccountEvent.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"

namespace opentxs::rpc::response::implementation
{
struct GetAccountActivity final : public Message::Imp {
    using Events = response::GetAccountActivity::Events;

    const Events events_;

    auto asGetAccountActivity() const noexcept
        -> const response::GetAccountActivity& final
    {
        return static_cast<const response::GetAccountActivity&>(*parent_);
    }
    auto serialize(protobuf::RPCResponse& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            for (const auto& event : events_) {
                if (false == event.Serialize(*dest.add_accountevent())) {

                    return false;
                }
            }

            return true;
        }

        return false;
    }

    GetAccountActivity(
        const response::GetAccountActivity* parent,
        const request::GetAccountActivity& request,
        Message::Responses&& response,
        Events&& events) noexcept(false)
        : Imp(parent, request, std::move(response))
        , events_(std::move(events))
    {
    }
    GetAccountActivity(
        const response::GetAccountActivity* parent,
        const protobuf::RPCResponse& in) noexcept(false)
        : Imp(parent, in)
        , events_([&] {
            auto out = Events{};
            const auto& data = in.accountevent();
            std::ranges::copy(data, std::back_inserter(out));

            return out;
        }())
    {
    }
    GetAccountActivity() = delete;
    GetAccountActivity(GetAccountActivity&&) = delete;
    auto operator=(const GetAccountActivity&) -> GetAccountActivity& = delete;
    auto operator=(GetAccountActivity&&) -> GetAccountActivity& = delete;

    ~GetAccountActivity() final = default;
};
}  // namespace opentxs::rpc::response::implementation

namespace opentxs::rpc::response
{
GetAccountActivity::GetAccountActivity(
    const request::GetAccountActivity& request,
    Responses&& response,
    Events&& events)
    : Message(std::make_unique<implementation::GetAccountActivity>(
          this,
          request,
          std::move(response),
          std::move(events)))
{
}

GetAccountActivity::GetAccountActivity(
    const protobuf::RPCResponse& serialized) noexcept(false)
    : Message(std::make_unique<implementation::GetAccountActivity>(
          this,
          serialized))
{
}

GetAccountActivity::GetAccountActivity() noexcept
    : Message()
{
}

auto GetAccountActivity::Activity() const noexcept -> const Events&
{
    return static_cast<const implementation::GetAccountActivity&>(*imp_)
        .events_;
}

GetAccountActivity::~GetAccountActivity() = default;
}  // namespace opentxs::rpc::response
