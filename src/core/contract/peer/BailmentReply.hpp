// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/BailmentReply.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
class Nym;
}  // namespace identifier

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::implementation
{
class Bailment final : public reply::Bailment,
                       public peer::implementation::Reply
{
public:
    auto asBailment() const noexcept -> const reply::Bailment& final
    {
        return *this;
    }

    Bailment(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Bailment(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms);

    ~Bailment() final = default;
    Bailment() = delete;
    Bailment(const Bailment&);
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    auto clone() const noexcept -> Bailment* final
    {
        return new Bailment(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::reply::implementation
