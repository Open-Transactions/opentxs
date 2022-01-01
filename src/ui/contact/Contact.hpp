// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/contact/SectionType.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace contact
{
class Contact;
}  // namespace contact

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
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactType = List<
    ContactExternalInterface,
    ContactInternalInterface,
    ContactRowID,
    ContactRowInterface,
    ContactRowInternal,
    ContactRowBlank,
    ContactSortKey,
    ContactPrimaryID>;

class Contact final : public ContactType
{
public:
    auto ClearCallbacks() const noexcept -> void final;
    auto ContactID() const noexcept -> std::string final;
    auto DisplayName() const noexcept -> std::string final;
    auto PaymentCode() const noexcept -> std::string final;

    auto SetCallbacks(Callbacks&& cb) noexcept -> void final;

    Contact(
        const api::session::Client& api,
        const Identifier& contactID,
        const SimpleCallback& cb) noexcept;
    ~Contact() final;

private:
    struct CallbackHolder {
        mutable std::mutex lock_{};
        Callbacks cb_{};
    };

    static const std::pmr::set<contact::SectionType> allowed_types_;
    static const std::pmr::map<contact::SectionType, int> sort_keys_;

    const ListenerDefinitions listeners_;
    mutable CallbackHolder callbacks_;
    std::string name_;
    std::string payment_code_;

    static auto sort_key(const contact::SectionType type) noexcept -> int;
    static auto check_type(const contact::SectionType type) noexcept -> bool;

    auto construct_row(
        const ContactRowID& id,
        const ContactSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ContactRowID& id) const noexcept -> bool final
    {
        return ContactType::last(id);
    }
    auto update(ContactRowInterface& row, CustomData& custom) const noexcept
        -> void;

    auto process_contact(const contact::Contact& contact) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto startup() noexcept -> void;

    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;
};
}  // namespace opentxs::ui::implementation
