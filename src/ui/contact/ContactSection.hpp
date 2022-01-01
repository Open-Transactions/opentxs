// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/contact/Contact.hpp"
#include "internal/protobuf/verify/VerifyContacts.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/contact/SectionType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"
#include "ui/base/Combined.hpp"
#include "ui/base/List.hpp"
#include "ui/base/RowType.hpp"

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
class ContactSection;
}  // namespace contact

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class ContactSection;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactSectionList = List<
    ContactSectionExternalInterface,
    ContactSectionInternalInterface,
    ContactSectionRowID,
    ContactSectionRowInterface,
    ContactSectionRowInternal,
    ContactSectionRowBlank,
    ContactSectionSortKey,
    ContactSectionPrimaryID>;
using ContactSectionRow =
    RowType<ContactRowInternal, ContactInternalInterface, ContactRowID>;

class ContactSection final
    : public Combined<ContactSectionList, ContactSectionRow, ContactSortKey>
{
public:
    auto ContactID() const noexcept -> std::string final
    {
        return primary_id_->str();
    }
    auto Name(const std::string& lang) const noexcept -> std::string final
    {
        return proto::TranslateSectionName(translate(row_id_), lang);
    }
    auto Type() const noexcept -> contact::SectionType final { return row_id_; }

    ContactSection(
        const ContactInternalInterface& parent,
        const api::session::Client& api,
        const ContactRowID& rowID,
        const ContactSortKey& key,
        CustomData& custom) noexcept;
    ~ContactSection() final = default;

private:
    static const std::pmr::
        map<contact::SectionType, std::pmr::set<proto::ContactItemType>>
            allowed_types_;
    static const std::pmr::
        map<contact::SectionType, std::pmr::map<proto::ContactItemType, int>>
            sort_keys_;

    static auto sort_key(const ContactSectionRowID type) noexcept -> int;
    static auto check_type(const ContactSectionRowID type) noexcept -> bool;

    auto construct_row(
        const ContactSectionRowID& id,
        const ContactSectionSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ContactSectionRowID& id) const noexcept -> bool final
    {
        return ContactSectionList::last(id);
    }
    auto process_section(const contact::ContactSection& section) noexcept
        -> std::pmr::set<ContactSectionRowID>;
    auto reindex(
        const implementation::ContactSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;
    auto startup(const contact::ContactSection section) noexcept -> void;

    ContactSection() = delete;
    ContactSection(const ContactSection&) = delete;
    ContactSection(ContactSection&&) = delete;
    auto operator=(const ContactSection&) -> ContactSection& = delete;
    auto operator=(ContactSection&&) -> ContactSection& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactSection>;
