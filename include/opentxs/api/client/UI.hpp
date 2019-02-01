// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_UI_HPP
#define OPENTXS_API_CLIENT_UI_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#ifdef SWIG
// clang-format off
%extend opentxs::api::client::UI {
    const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const int currency) const
    {
        return $self->AccountSummary(
            nymID,
            static_cast<opentxs::proto::ContactItemType>(currency));
    }
    const opentxs::ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        const int currency) const
    {
        return $self->PayableList(
            nymID,
            static_cast<opentxs::proto::ContactItemType>(currency));
    }
}
%ignore opentxs::api::client::UI::AccountSummary;
%ignore opentxs::api::client::UI::PayableList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace api
{
namespace client
{
class UI
{
public:
    EXPORT virtual const ui::AccountActivity& AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID) const = 0;
    EXPORT virtual const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const = 0;
    EXPORT virtual const ui::ActivitySummary& ActivitySummary(
        const identifier::Nym& nymID) const = 0;
    EXPORT virtual const ui::ActivityThread& ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const = 0;
    EXPORT virtual const ui::Contact& Contact(
        const Identifier& contactID) const = 0;
    EXPORT virtual const ui::ContactList& ContactList(
        const identifier::Nym& nymID) const = 0;
    EXPORT virtual const ui::MessagableList& MessagableList(
        const identifier::Nym& nymID) const = 0;
    EXPORT virtual const ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        proto::ContactItemType currency) const = 0;
    EXPORT virtual const ui::Profile& Profile(
        const identifier::Nym& contactID) const = 0;

    virtual ~UI() = default;

protected:
    UI() = default;

private:
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
