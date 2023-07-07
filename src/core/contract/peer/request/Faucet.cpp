// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/request/Faucet.hpp"  // IWYU pragma: associated

#include <Faucet.pb.h>
#include <PeerRequest.pb.h>

#include "core/contract/peer/request/Base.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "opentxs/api/session/Crypto.hpp"              // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

namespace opentxs::contract::peer::request::implementation
{
Faucet::Faucet(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    opentxs::UnitType unit,
    std::string_view address)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          identifier::Notary{},
          RequestType::Faucet)
    , unit_(unit)
    , address_(address)
{
    first_time_init(set_name_from_id_);
}

Faucet::Faucet(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , unit_(ClaimToUnit(translate(serialized.faucet().type())))
    , address_(serialized.faucet().address())
{
    init_serialized();
}

Faucet::Faucet(const Faucet& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , address_(rhs.address_)
{
}

auto Faucet::IDVersion() const -> SerializedType
{
    auto contract = Request::IDVersion();
    auto& faucet = *contract.mutable_faucet();
    faucet.set_version(Version());
    faucet.set_type(translate(UnitToClaim(unit_)));
    faucet.set_address(address_.data(), address_.size());

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
