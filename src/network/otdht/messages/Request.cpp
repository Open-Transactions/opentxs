// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/otdht/Request.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/network/otdht/Factory.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/network/otdht/Block.hpp"        // IWYU pragma: keep
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/State.hpp"        // IWYU pragma: keep

namespace opentxs::factory
{
auto BlockchainSyncRequest() noexcept -> network::otdht::Request
{
    using ReturnType = network::otdht::Request;

    return {std::make_unique<ReturnType::Imp>().release()};
}

auto BlockchainSyncRequest(network::otdht::StateData in) noexcept
    -> network::otdht::Request
{
    using ReturnType = network::otdht::Request;

    return {std::make_unique<ReturnType::Imp>(std::move(in)).release()};
}

auto BlockchainSyncRequest_p(network::otdht::StateData in) noexcept
    -> std::unique_ptr<network::otdht::Request>
{
    using ReturnType = network::otdht::Request;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(std::move(in)).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
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
}  // namespace opentxs::network::otdht
