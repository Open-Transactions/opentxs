// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/reply/Bailment.hpp"  // IWYU pragma: associated

#include <BailmentReply.pb.h>
#include <PeerReply.pb.h>

#include "core/contract/peer/reply/Base.hpp"
#include "opentxs/api/session/Crypto.hpp"              // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::reply::implementation
{
Bailment::Bailment(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const UnallocatedCString& terms)
    : Reply(
          api,
          nym,
          current_version_,
          initiator,
          server,
          RequestType::Bailment,
          request,
          terms)
{
    first_time_init(set_name_from_id_);
}

Bailment::Bailment(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Reply(api, nym, serialized, serialized.bailment().instructions())
{
    init_serialized();
}

Bailment::Bailment(const Bailment& rhs)
    : Reply(rhs)
{
}

auto Bailment::IDVersion() const -> SerializedType
{
    auto contract = Reply::IDVersion();
    auto& bailment = *contract.mutable_bailment();
    bailment.set_version(Version());
    bailment.set_instructions(UnallocatedCString{Terms()});

    return contract;
}
}  // namespace opentxs::contract::peer::reply::implementation
