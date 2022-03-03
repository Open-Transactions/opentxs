// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "opentxs/network/p2p/Acknowledgement.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/network/p2p/Factory.hpp"
#include "network/p2p/Base.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BlockchainSyncAcknowledgement() noexcept -> network::p2p::Acknowledgement
{
    using ReturnType = network::p2p::Acknowledgement;

    return {std::make_unique<ReturnType::Imp>().release()};
}

auto BlockchainSyncAcknowledgement(
    network::p2p::StateData in,
    UnallocatedCString endpoint) noexcept -> network::p2p::Acknowledgement
{
    using ReturnType = network::p2p::Acknowledgement;

    return {
        std::make_unique<ReturnType::Imp>(std::move(in), std::move(endpoint))
            .release()};
}

auto BlockchainSyncAcknowledgement_p(
    network::p2p::StateData in,
    UnallocatedCString endpoint) noexcept
    -> std::unique_ptr<network::p2p::Acknowledgement>
{
    using ReturnType = network::p2p::Acknowledgement;

    return std::make_unique<ReturnType>(
        std::make_unique<ReturnType::Imp>(std::move(in), std::move(endpoint))
            .release());
}
}  // namespace opentxs::factory

namespace opentxs::network::p2p
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
    Imp(StateData state, UnallocatedCString endpoint) noexcept
        : Base::Imp(
              Base::Imp::default_version_,
              MessageType::sync_ack,
              std::move(state),
              std::move(endpoint),
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

Acknowledgement::Acknowledgement(Imp* imp) noexcept
    : Base(imp)
    , imp_(imp)
{
    imp_->parent_ = this;
}

auto Acknowledgement::Endpoint() const noexcept -> const UnallocatedCString&
{
    return imp_->endpoint_;
}

auto Acknowledgement::State() const noexcept -> const StateData&
{
    return imp_->state_;
}

auto Acknowledgement::State(opentxs::blockchain::Type chain) const
    noexcept(false) -> const p2p::State&
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
}  // namespace opentxs::network::p2p
