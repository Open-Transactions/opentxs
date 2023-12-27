// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/response/GetAccountBalance.hpp"  // IWYU pragma: associated
#include "opentxs/rpc/response/MessagePrivate.hpp"  // IWYU pragma: associated

#include <RPCResponse.pb.h>
#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

#include "opentxs/rpc/AccountData.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"

namespace opentxs::rpc::response::implementation
{
struct GetAccountBalance final : public Message::Imp {
    using Data = response::GetAccountBalance::Data;

    const Data balances_;

    auto asGetAccountBalance() const noexcept
        -> const response::GetAccountBalance& final
    {
        return static_cast<const response::GetAccountBalance&>(*parent_);
    }
    auto serialize(proto::RPCResponse& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            for (const auto& balance : balances_) {
                if (false == balance.Serialize(*dest.add_balance())) {

                    return false;
                }
            }

            return true;
        }

        return false;
    }

    GetAccountBalance(
        const response::GetAccountBalance* parent,
        const request::GetAccountBalance& request,
        Message::Responses&& response,
        Data&& balances) noexcept(false)
        : Imp(parent, request, std::move(response))
        , balances_(std::move(balances))
    {
    }
    GetAccountBalance(
        const response::GetAccountBalance* parent,
        const proto::RPCResponse& in) noexcept(false)
        : Imp(parent, in)
        , balances_([&] {
            auto out = Data{};
            const auto& data = in.balance();
            std::ranges::copy(data, std::back_inserter(out));

            return out;
        }())
    {
    }
    GetAccountBalance() = delete;
    GetAccountBalance(GetAccountBalance&&) = delete;
    auto operator=(const GetAccountBalance&) -> GetAccountBalance& = delete;
    auto operator=(GetAccountBalance&&) -> GetAccountBalance& = delete;

    ~GetAccountBalance() final = default;
};
}  // namespace opentxs::rpc::response::implementation

namespace opentxs::rpc::response
{
GetAccountBalance::GetAccountBalance(
    const request::GetAccountBalance& request,
    Responses&& response,
    Data&& balances)
    : Message(std::make_unique<implementation::GetAccountBalance>(
          this,
          request,
          std::move(response),
          std::move(balances)))
{
}

GetAccountBalance::GetAccountBalance(
    const proto::RPCResponse& serialized) noexcept(false)
    : Message(
          std::make_unique<implementation::GetAccountBalance>(this, serialized))
{
}

GetAccountBalance::GetAccountBalance() noexcept
    : Message()
{
}

auto GetAccountBalance::Balances() const noexcept -> const Data&
{
    return static_cast<const implementation::GetAccountBalance&>(*imp_)
        .balances_;
}

GetAccountBalance::~GetAccountBalance() = default;
}  // namespace opentxs::rpc::response
