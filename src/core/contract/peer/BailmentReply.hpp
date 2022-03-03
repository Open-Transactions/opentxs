// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/PeerReply.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

class Factory;
class Identifier;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::implementation
{
class Bailment final : public reply::Bailment,
                       public peer::implementation::Reply
{
public:
    Bailment(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Bailment(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms);

    ~Bailment() final = default;

    auto asBailment() const noexcept -> const reply::Bailment& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    auto clone() const noexcept -> Bailment* final
    {
        return new Bailment(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    Bailment() = delete;
    Bailment(const Bailment&);
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
