// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"

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
}  // namespace api

namespace identity
{
namespace wot
{
namespace claim
{
class Section;
}  // namespace claim
}  // namespace wot
}  // namespace identity

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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    auto ContactID() const noexcept -> UnallocatedCString final
    {
        return primary_id_->str();
    }
    auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString final
    {
        return proto::TranslateSectionName(translate(row_id_), lang);
    }
    auto Type() const noexcept -> identity::wot::claim::SectionType final
    {
        return row_id_;
    }

    ContactSection(
        const ContactInternalInterface& parent,
        const api::session::Client& api,
        const ContactRowID& rowID,
        const ContactSortKey& key,
        CustomData& custom) noexcept;
    ~ContactSection() final = default;

private:
    static const std::map<
        identity::wot::claim::SectionType,
        UnallocatedSet<proto::ContactItemType>>
        allowed_types_;
    static const UnallocatedMap<
        identity::wot::claim::SectionType,
        UnallocatedMap<proto::ContactItemType, int>>
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
    auto process_section(const identity::wot::claim::Section& section) noexcept
        -> UnallocatedSet<ContactSectionRowID>;
    auto reindex(
        const implementation::ContactSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;
    auto startup(const identity::wot::claim::Section section) noexcept -> void;

    ContactSection() = delete;
    ContactSection(const ContactSection&) = delete;
    ContactSection(ContactSection&&) = delete;
    auto operator=(const ContactSection&) -> ContactSection& = delete;
    auto operator=(ContactSection&&) -> ContactSection& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactSection>;
