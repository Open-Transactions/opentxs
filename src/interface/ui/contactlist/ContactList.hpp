// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <memory>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/interface/ui/ContactList.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
using CONTACTLISTID = std::pair<bool, opentxs::UnallocatedCString>;

template <>
struct less<CONTACTLISTID> {
    auto operator()(const CONTACTLISTID& lhs, const CONTACTLISTID& rhs) const
        -> bool
    {
        const auto& [lSelf, lText] = lhs;
        const auto& [rSelf, rText] = rhs;

        if (lSelf && (!rSelf)) { return true; }

        if (rSelf && (!lSelf)) { return false; }

        if (lText < rText) { return true; }

        return false;
    }
};
}  // namespace std

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
    auto ID() const noexcept -> const Identifier& final
    {
        return owner_contact_id_;
    }

    ContactList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
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
        OTNymID nym_id_;
        PaymentCode payment_code_;

        ParsedArgs(
            const api::Session& api,
            const UnallocatedCString& purportedID,
            const UnallocatedCString& purportedPaymentCode) noexcept;

    private:
        static auto extract_nymid(
            const api::Session& api,
            const UnallocatedCString& purportedID,
            const UnallocatedCString& purportedPaymentCode) noexcept -> OTNymID;
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
    auto process_contact(const Identifier& contactID) noexcept -> void;
    auto startup() noexcept -> void;

    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    auto operator=(const ContactList&) -> ContactList& = delete;
    auto operator=(ContactList&&) -> ContactList& = delete;
};
}  // namespace opentxs::ui::implementation
