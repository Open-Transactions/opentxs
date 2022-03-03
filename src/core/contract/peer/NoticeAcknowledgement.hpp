// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Proto.hpp"
#include "core/contract/peer/PeerReply.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
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
class Acknowledgement final : public reply::Acknowledgement,
                              public peer::implementation::Reply
{
public:
    Acknowledgement(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Notary& server,
        const PeerRequestType type,
        const bool& ack);
    Acknowledgement(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized);

    ~Acknowledgement() final = default;

    auto asAcknowledgement() const noexcept
        -> const reply::Acknowledgement& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    const bool ack_;

    auto clone() const noexcept -> Acknowledgement* final
    {
        return new Acknowledgement(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    Acknowledgement() = delete;
    Acknowledgement(const Acknowledgement&);
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
