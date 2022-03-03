// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "interface/rpc/response/Base.hpp"  // IWYU pragma: associated
#include "opentxs/interface/rpc/response/GetAccountActivity.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

#include "Proto.hpp"
#include "opentxs/interface/rpc/AccountEvent.hpp"
#include "opentxs/interface/rpc/request/GetAccountActivity.hpp"
#include "serialization/protobuf/RPCResponse.pb.h"

namespace opentxs::rpc::response::implementation
{
struct GetAccountActivity final : public Base::Imp {
    using Events = response::GetAccountActivity::Events;

    const Events events_;

    auto asGetAccountActivity() const noexcept
        -> const response::GetAccountActivity& final
    {
        return static_cast<const response::GetAccountActivity&>(*parent_);
    }
    auto serialize(proto::RPCResponse& dest) const noexcept -> bool final
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
        Base::Responses&& response,
        Events&& events) noexcept(false)
        : Imp(parent, request, std::move(response))
        , events_(std::move(events))
    {
    }
    GetAccountActivity(
        const response::GetAccountActivity* parent,
        const proto::RPCResponse& in) noexcept(false)
        : Imp(parent, in)
        , events_([&] {
            auto out = Events{};
            const auto& data = in.accountevent();
            std::copy(data.begin(), data.end(), std::back_inserter(out));

            return out;
        }())
    {
    }

    ~GetAccountActivity() final = default;

private:
    GetAccountActivity() = delete;
    GetAccountActivity(GetAccountActivity&&) = delete;
    auto operator=(const GetAccountActivity&) -> GetAccountActivity& = delete;
    auto operator=(GetAccountActivity&&) -> GetAccountActivity& = delete;
};
}  // namespace opentxs::rpc::response::implementation

namespace opentxs::rpc::response
{
GetAccountActivity::GetAccountActivity(
    const request::GetAccountActivity& request,
    Responses&& response,
    Events&& events)
    : Base(std::make_unique<implementation::GetAccountActivity>(
          this,
          request,
          std::move(response),
          std::move(events)))
{
}

GetAccountActivity::GetAccountActivity(
    const proto::RPCResponse& serialized) noexcept(false)
    : Base(std::make_unique<implementation::GetAccountActivity>(
          this,
          serialized))
{
}

GetAccountActivity::GetAccountActivity() noexcept
    : Base()
{
}

auto GetAccountActivity::Activity() const noexcept -> const Events&
{
    return static_cast<const implementation::GetAccountActivity&>(*imp_)
        .events_;
}

GetAccountActivity::~GetAccountActivity() = default;
}  // namespace opentxs::rpc::response
