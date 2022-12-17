// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/OutBailmentReply.hpp"
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::implementation
{
class Outbailment final : public reply::Outbailment,
                          public peer::implementation::Reply
{
public:
    auto asOutbailment() const noexcept -> const reply::Outbailment& final
    {
        return *this;
    }

    Outbailment(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms);
    Outbailment(
        const api::Session& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Outbailment() = delete;
    Outbailment(const Outbailment&);
    Outbailment(Outbailment&&) = delete;
    auto operator=(const Outbailment&) -> Outbailment& = delete;
    auto operator=(Outbailment&&) -> Outbailment& = delete;

    ~Outbailment() final = default;

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    auto clone() const noexcept -> Outbailment* final
    {
        return new Outbailment(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::reply::implementation
