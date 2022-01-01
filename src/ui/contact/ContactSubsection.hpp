// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/SharedPimpl.hpp"
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
class ContactGroup;
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
class ContactSubsection;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ContactSubsectionList = List<
    ContactSubsectionExternalInterface,
    ContactSubsectionInternalInterface,
    ContactSubsectionRowID,
    ContactSubsectionRowInterface,
    ContactSubsectionRowInternal,
    ContactSubsectionRowBlank,
    ContactSubsectionSortKey,
    ContactSubsectionPrimaryID>;
using ContactSubsectionRow = RowType<
    ContactSectionRowInternal,
    ContactSectionInternalInterface,
    ContactSectionRowID>;

class ContactSubsection final : public Combined<
                                    ContactSubsectionList,
                                    ContactSubsectionRow,
                                    ContactSectionSortKey>
{
public:
    auto Name(const std::string& lang) const noexcept -> std::string final;
    auto Type() const noexcept -> contact::ClaimType final
    {
        return row_id_.second;
    }

    ContactSubsection(
        const ContactSectionInternalInterface& parent,
        const api::session::Client& api,
        const ContactSectionRowID& rowID,
        const ContactSectionSortKey& key,
        CustomData& custom) noexcept;
    ~ContactSubsection() final = default;

private:
    ContactSectionSortKey sequence_;

    static auto check_type(const ContactSubsectionRowID type) noexcept -> bool;

    auto construct_row(
        const ContactSubsectionRowID& id,
        const ContactSubsectionSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ContactSubsectionRowID& id) const noexcept -> bool final
    {
        return ContactSubsectionList::last(id);
    }
    auto process_group(const contact::ContactGroup& group) noexcept
        -> std::pmr::set<ContactSubsectionRowID>;
    auto reindex(const ContactSectionSortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup(const contact::ContactGroup group) noexcept -> void;

    ContactSubsection() = delete;
    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    auto operator=(const ContactSubsection&) -> ContactSubsection& = delete;
    auto operator=(ContactSubsection&&) -> ContactSubsection& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactSubsection>;
