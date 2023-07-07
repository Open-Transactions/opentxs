// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/request/StoreSecret.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <StoreSecret.pb.h>

#include "core/contract/peer/request/Base.hpp"
#include "internal/core/contract/peer/Types.hpp"
#include "opentxs/api/session/Crypto.hpp"              // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::request::implementation
{
StoreSecret::StoreSecret(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const SecretType type,
    const UnallocatedCString& primary,
    const UnallocatedCString& secondary,
    const identifier::Notary& serverID)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          serverID,
          contract::peer::RequestType::StoreSecret)
    , secret_type_(type)
    , primary_(primary)
    , secondary_(secondary)
{
    first_time_init(set_name_from_id_);
}

StoreSecret::StoreSecret(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , secret_type_(translate(serialized.storesecret().type()))
    , primary_(serialized.storesecret().primary())
    , secondary_(serialized.storesecret().secondary())
{
    init_serialized();
}

StoreSecret::StoreSecret(const StoreSecret& rhs)
    : Request(rhs)
    , secret_type_(rhs.secret_type_)
    , primary_(rhs.primary_)
    , secondary_(rhs.secondary_)
{
}

auto StoreSecret::IDVersion() const -> SerializedType
{
    auto contract = Request::IDVersion();
    auto& storesecret = *contract.mutable_storesecret();
    storesecret.set_version(Version());
    storesecret.set_type(translate(secret_type_));
    storesecret.set_primary(primary_);
    storesecret.set_secondary(secondary_);

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
