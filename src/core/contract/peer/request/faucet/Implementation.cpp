// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/faucet/Implementation.hpp"  // IWYU pragma: associated

#include <Faucet.pb.h>
#include <PeerRequest.pb.h>
#include <utility>

#include "internal/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

namespace opentxs::contract::peer::request::faucet
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    opentxs::UnitType currency,
    std::string_view instructions,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , FaucetPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          alloc)
    , currency_(std::move(currency))
    , instructions_(instructions, alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , FaucetPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , currency_(ClaimToUnit(translate(proto.faucet().type())))
    , instructions_(proto.faucet().address(), alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , FaucetPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , currency_(rhs.currency_)
    , instructions_(rhs.instructions_, alloc)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& faucet = *out.mutable_faucet();
    faucet.set_version(Version());
    faucet.set_type(translate(UnitToClaim(currency_)));
    faucet.set_address(instructions_.data(), instructions_.size());

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::request::faucet
