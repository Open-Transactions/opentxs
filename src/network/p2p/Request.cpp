// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/network/p2p/Request.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/network/p2p/Factory.hpp"
#include "network/p2p/Base.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/State.hpp"

namespace opentxs::factory
{
auto BlockchainSyncRequest() noexcept -> network::p2p::Request
{
    using ReturnType = network::p2p::Request;

    return {std::make_unique<ReturnType::Imp>().release()};
}

auto BlockchainSyncRequest(network::p2p::StateData in) noexcept
    -> network::p2p::Request
{
    using ReturnType = network::p2p::Request;

    return {std::make_unique<ReturnType::Imp>(std::move(in)).release()};
}

auto BlockchainSyncRequest_p(network::p2p::StateData in) noexcept
    -> std::unique_ptr<network::p2p::Request>
{
    using ReturnType = network::p2p::Request;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(std::move(in)).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::p2p
{
class Request::Imp final : public Base::Imp
{
public:
    Request* parent_;

    auto asRequest() const noexcept -> const Request& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asRequest();
        }
    }

    Imp() noexcept
        : Base::Imp()
        , parent_(nullptr)
    {
    }
    Imp(StateData state) noexcept
        : Base::Imp(
              Base::Imp::default_version_,
              MessageType::sync_request,
              std::move(state),
              {},
              {})
        , parent_(nullptr)
    {
    }

private:
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Request::Request(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto Request::State() const noexcept -> const StateData&
{
    return imp_->state_;
}

Request::~Request()
{
    if (nullptr != Request::imp_) {
        delete Request::imp_;
        Request::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::p2p
