// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ContactListList = List<
    ContactListExternalInterface,
    ContactListInternalInterface,
    ContactListRowID,
    ContactListRowInterface,
    ContactListRowInternal,
    ContactListRowBlank,
    ContactListSortKey,
    ContactListPrimaryID>;

class ContactList final : virtual public internal::ContactList,
                          public ContactListList,
                          Worker<ContactList>
{
public:
    auto AddContact(
        const UnallocatedCString& label,
        const UnallocatedCString& paymentCode,
        const UnallocatedCString& nymID) const noexcept
        -> UnallocatedCString final;
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ID() const noexcept -> const identifier::Generic& final
    {
        return owner_contact_id_;
    }
    auto SetContactName(
        const UnallocatedCString& contactID,
        const UnallocatedCString& name) const noexcept -> bool final;

    ContactList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    auto operator=(const ContactList&) -> ContactList& = delete;
    auto operator=(ContactList&&) -> ContactList& = delete;

    ~ContactList() final;

private:
    friend Worker<ContactList>;

    enum class Work : OTZMQWorkType {
        contact = value(WorkType::ContactUpdated),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    struct ParsedArgs {
        identifier::Nym nym_id_;
        PaymentCode payment_code_;

        ParsedArgs(
            const api::Session& api,
            const UnallocatedCString& purportedID,
            const UnallocatedCString& purportedPaymentCode) noexcept;

    private:
        static auto extract_nymid(
            const api::Session& api,
            const UnallocatedCString& purportedID,
            const UnallocatedCString& purportedPaymentCode) noexcept
            -> identifier::Nym;
        static auto extract_paymentcode(
            const api::Session& api,
            const UnallocatedCString& purportedID,
            const UnallocatedCString& purportedPaymentCode) noexcept
            -> PaymentCode;
    };

    const ContactListRowID owner_contact_id_;

    auto construct_row(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto default_id() const noexcept -> ContactListRowID final
    {
        return owner_contact_id_;
    }

    auto pipeline(const Message& in) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto process_contact(const identifier::Generic& contactID) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
