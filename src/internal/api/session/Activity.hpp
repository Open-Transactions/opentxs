// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/otx/common/Cheque.hpp"
// IWYU pragma: no_include "internal/otx/common/Item.hpp"
// IWYU pragma: no_include "internal/otx/common/Message.hpp"

#pragma once

#include "opentxs/api/session/Activity.hpp"

#include "internal/core/contract/Unit.hpp"
#include "opentxs/otx/client/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

class Cheque;
class Item;
class Message;
class PeerObject;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Activity : virtual public session::Activity
{
public:
    using ChequeData =
        std::pair<std::unique_ptr<const opentxs::Cheque>, OTUnitDefinition>;
    using TransferData =
        std::pair<std::unique_ptr<const opentxs::Item>, OTUnitDefinition>;

    virtual auto Cheque(
        const identifier::Nym& nym,
        const UnallocatedCString& id,
        const UnallocatedCString& workflow) const noexcept -> ChequeData = 0;
    auto Internal() const noexcept -> const Activity& final { return *this; }
    using session::Activity::Mail;
    /**   Load a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto Mail(
        const identifier::Nym& nym,
        const identifier::Generic& id,
        const otx::client::StorageBox& box) const noexcept
        -> std::unique_ptr<Message> = 0;
    /**   Store a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    virtual auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const otx::client::StorageBox box,
        const PeerObject& text) const noexcept -> UnallocatedCString = 0;
    virtual auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const otx::client::StorageBox box,
        const UnallocatedCString& text) const noexcept
        -> UnallocatedCString = 0;
    virtual auto Transfer(
        const identifier::Nym& nym,
        const UnallocatedCString& id,
        const UnallocatedCString& workflow) const noexcept -> TransferData = 0;

    auto Internal() noexcept -> Activity& final { return *this; }

    ~Activity() override = default;
};
}  // namespace opentxs::api::session::internal
