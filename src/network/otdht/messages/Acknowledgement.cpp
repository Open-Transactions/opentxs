// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "opentxs/network/otdht/Acknowledgement.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/network/otdht/Factory.hpp"
#include "network/otdht/messages/Base.hpp"
#include "opentxs/network/otdht/MessageType.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BlockchainSyncAcknowledgement() noexcept -> network::otdht::Acknowledgement
{
    using ReturnType = network::otdht::Acknowledgement;

    return {std::make_unique<ReturnType::Imp>().release()};
}

auto BlockchainSyncAcknowledgement(
    network::otdht::StateData in,
    std::string_view endpoint) noexcept -> network::otdht::Acknowledgement
{
    using ReturnType = network::otdht::Acknowledgement;

    return {
        std::make_unique<ReturnType::Imp>(std::move(in), endpoint).release()};
}

auto BlockchainSyncAcknowledgement_p(
    network::otdht::StateData in,
    std::string_view endpoint) noexcept
    -> std::unique_ptr<network::otdht::Acknowledgement>
{
    using ReturnType = network::otdht::Acknowledgement;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(std::move(in), endpoint).release());
}
}  // namespace opentxs::factory

namespace opentxs::network::otdht
{
class Acknowledgement::Imp final : public Base::Imp
{
public:
    Acknowledgement* parent_;

    auto asAcknowledgement() const noexcept -> const Acknowledgement& final
    {
        if (nullptr != parent_) {

            return *parent_;
        } else {

            return Base::Imp::asAcknowledgement();
        }
    }

    Imp() noexcept
        : Base::Imp()
        , parent_(nullptr)
    {
    }
    Imp(StateData state, std::string_view endpoint) noexcept
        : Base::Imp(
              Base::Imp::default_version_,
              MessageType::sync_ack,
              std::move(state),
              UnallocatedCString{endpoint},
              {})
        , parent_(nullptr)
    {
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Acknowledgement::Acknowledgement(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto Acknowledgement::Endpoint() const noexcept -> std::string_view
{
    return imp_->endpoint_;
}

auto Acknowledgement::State() const noexcept -> const StateData&
{
    return imp_->state_;
}

auto Acknowledgement::State(opentxs::blockchain::Type chain) const
    noexcept(false) -> const otdht::State&
{
    for (const auto& state : imp_->state_) {
        if (state.Chain() == chain) { return state; }
    }

    throw std::out_of_range{
        "specified chain does not exist in acknowledgement"};
}

Acknowledgement::~Acknowledgement()
{
    if (nullptr != Acknowledgement::imp_) {
        delete Acknowledgement::imp_;
        Acknowledgement::imp_ = nullptr;
        Base::imp_ = nullptr;
    }
}
}  // namespace opentxs::network::otdht
