// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/reply/Base.hpp"
#include "internal/core/contract/peer/reply/Acknowledgement.hpp"
#include "internal/core/contract/peer/reply/Base.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::implementation
{
class Acknowledgement final : public reply::internal::Acknowledgement,
                              public implementation::Reply
{
public:
    auto asAcknowledgement() const noexcept
        -> const internal::Acknowledgement& final
    {
        return *this;
    }

    Acknowledgement(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const RequestType type,
        const bool& ack);
    Acknowledgement(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Acknowledgement() = delete;
    Acknowledgement(const Acknowledgement&);
    Acknowledgement(Acknowledgement&&) = delete;
    auto operator=(const Acknowledgement&) -> Acknowledgement& = delete;
    auto operator=(Acknowledgement&&) -> Acknowledgement& = delete;

    ~Acknowledgement() final = default;

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    const bool ack_;

    auto IDVersion() const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::reply::implementation
