// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::SectionType

#pragma once

#include <mutex>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

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

class Contact;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ClearCallbacks() const noexcept -> void final;
    auto ContactID() const noexcept -> UnallocatedCString final;
    auto DisplayName() const noexcept -> UnallocatedCString final;
    auto PaymentCode() const noexcept -> UnallocatedCString final;

    auto SetCallbacks(Callbacks&& cb) noexcept -> void final;

    Contact(
        const api::session::Client& api,
        const identifier::Generic& contactID,
        const SimpleCallback& cb) noexcept;
    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;

    ~Contact() final;

private:
    struct CallbackHolder {
        mutable std::mutex lock_{};
        Callbacks cb_{};
    };

    static const UnallocatedSet<identity::wot::claim::SectionType>
        allowed_types_;
    static const UnallocatedMap<identity::wot::claim::SectionType, int>
        sort_keys_;

    const ListenerDefinitions listeners_;
    mutable CallbackHolder callbacks_;
    UnallocatedCString name_;
    UnallocatedCString payment_code_;

    static auto sort_key(const identity::wot::claim::SectionType type) noexcept
        -> int;
    static auto check_type(
        const identity::wot::claim::SectionType type) noexcept -> bool;

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

    auto process_contact(const opentxs::Contact& contact) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
