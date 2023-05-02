// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/OTX.hpp"

#include "internal/util/UniqueQueue.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

class OTPayment;
template <class T>
class UniqueQueue;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class OTX : virtual public api::session::OTX
{
public:
    virtual void associate_message_id(
        const identifier::Generic& messageID,
        const TaskID taskID) const = 0;
    virtual auto can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const identifier::Account& accountIDHint,
        identifier::Notary& depositServer,
        identifier::UnitDefinition& unitID,
        identifier::Account& depositAccount) const
        -> otx::client::Depositability = 0;
    virtual auto finish_task(
        const TaskID taskID,
        const bool success,
        Result&& result) const -> bool = 0;
    auto Internal() const noexcept -> const internal::OTX& final
    {
        return *this;
    }
    virtual auto get_nym_fetch(const identifier::Notary& serverID) const
        -> UniqueQueue<identifier::Nym>& = 0;
    virtual auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask = 0;

    auto Internal() noexcept -> internal::OTX& final { return *this; }
};
}  // namespace opentxs::api::session::internal
