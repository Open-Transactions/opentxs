// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "opentxs/network/p2p/State.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainP2PChainState.pb.h"

namespace opentxs::network::p2p
{
struct State::Imp {
    static constexpr auto default_version_ = VersionNumber{1};

    const opentxs::blockchain::Type chain_;
    const opentxs::blockchain::block::Position position_;

    Imp(opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Position position) noexcept(false)
        : chain_(chain)
        , position_(std::move(position))
    {
        if (0 == opentxs::blockchain::DefinedChains().count(chain_)) {
            throw std::runtime_error{"invalid chain"};
        }
    }

private:
    Imp() noexcept;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

State::State(
    const api::Session& api,
    const proto::BlockchainP2PChainState& in) noexcept(false)
    : State(
          static_cast<opentxs::blockchain::Type>(in.chain()),
          {in.height(), api.Factory().Data(in.hash())})
{
}

State::State(
    opentxs::blockchain::Type chain,
    opentxs::blockchain::block::Position position) noexcept(false)
    : imp_(std::make_unique<Imp>(chain, position).release())
{
}

State::State(State&& rhs) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

auto State::Chain() const noexcept -> opentxs::blockchain::Type
{
    return imp_->chain_;
}

auto State::Position() const noexcept
    -> const opentxs::blockchain::block::Position&
{
    return imp_->position_;
}

auto State::Serialize(proto::BlockchainP2PChainState& dest) const noexcept
    -> bool
{
    const auto& pos = imp_->position_;
    const auto bytes = pos.second->Bytes();
    dest.set_version(Imp::default_version_);
    dest.set_chain(static_cast<std::uint32_t>(imp_->chain_));
    dest.set_height(pos.first);
    dest.set_hash(bytes.data(), bytes.size());

    return true;
}

State::~State() { std::unique_ptr<Imp>(imp_).reset(); }
}  // namespace opentxs::network::p2p
