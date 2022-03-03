// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "interface/rpc/response/Base.hpp"  // IWYU pragma: associated
#include "opentxs/interface/rpc/response/SendPayment.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/interface/rpc/request/SendPayment.hpp"

namespace opentxs::rpc::response::implementation
{
struct SendPayment final : public Base::Imp {
    auto asSendPayment() const noexcept -> const response::SendPayment& final
    {
        return static_cast<const response::SendPayment&>(*parent_);
    }
    auto serialize(proto::RPCResponse& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            serialize_tasks(dest);

            return true;
        }

        return false;
    }

    SendPayment(
        const response::SendPayment* parent,
        const request::SendPayment& request,
        Base::Responses&& response,
        Base::Tasks&& tasks) noexcept(false)
        : Imp(parent, request, std::move(response), std::move(tasks))
    {
    }
    SendPayment(
        const response::SendPayment* parent,
        const proto::RPCResponse& in) noexcept(false)
        : Imp(parent, in)
    {
    }

    ~SendPayment() final = default;

private:
    SendPayment() = delete;
    SendPayment(const SendPayment&) = delete;
    SendPayment(SendPayment&&) = delete;
    auto operator=(const SendPayment&) -> SendPayment& = delete;
    auto operator=(SendPayment&&) -> SendPayment& = delete;
};
}  // namespace opentxs::rpc::response::implementation

namespace opentxs::rpc::response
{
SendPayment::SendPayment(
    const request::SendPayment& request,
    Responses&& response,
    Tasks&& tasks)
    : Base(std::make_unique<implementation::SendPayment>(
          this,
          request,
          std::move(response),
          std::move(tasks)))
{
}

SendPayment::SendPayment(const proto::RPCResponse& serialized) noexcept(false)
    : Base(std::make_unique<implementation::SendPayment>(this, serialized))
{
}

SendPayment::SendPayment() noexcept
    : Base()
{
}

auto SendPayment::Pending() const noexcept -> const Tasks&
{
    return imp_->tasks_;
}

SendPayment::~SendPayment() = default;
}  // namespace opentxs::rpc::response
