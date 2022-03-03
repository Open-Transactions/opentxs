// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/OTX.hpp"

#include "internal/util/UniqueQueue.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identifier
{
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

class Identifier;
class OTPayment;
template <class T>
class UniqueQueue;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class OTX : virtual public api::session::OTX
{
public:
    virtual void associate_message_id(
        const Identifier& messageID,
        const TaskID taskID) const = 0;
    virtual auto can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Notary& depositServer,
        identifier::UnitDefinition& unitID,
        Identifier& depositAccount) const -> Depositability = 0;
    virtual auto finish_task(
        const TaskID taskID,
        const bool success,
        Result&& result) const -> bool = 0;
    auto Internal() const noexcept -> const internal::OTX& final
    {
        return *this;
    }
    virtual auto get_nym_fetch(const identifier::Notary& serverID) const
        -> UniqueQueue<OTNymID>& = 0;
    virtual auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask = 0;

    auto Internal() noexcept -> internal::OTX& final { return *this; }
};
}  // namespace opentxs::api::session::internal
