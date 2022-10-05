// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

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

namespace identity
{
namespace wot
{
namespace claim
{
class Group;
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
class ContactSubsection;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString final;
    auto Type() const noexcept -> identity::wot::claim::ClaimType final
    {
        return row_id_.second;
    }

    ContactSubsection(
        const ContactSectionInternalInterface& parent,
        const api::session::Client& api,
        const ContactSectionRowID& rowID,
        const ContactSectionSortKey& key,
        CustomData& custom) noexcept;
    ContactSubsection() = delete;
    ContactSubsection(const ContactSubsection&) = delete;
    ContactSubsection(ContactSubsection&&) = delete;
    auto operator=(const ContactSubsection&) -> ContactSubsection& = delete;
    auto operator=(ContactSubsection&&) -> ContactSubsection& = delete;

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
    auto process_group(const identity::wot::claim::Group& group) noexcept
        -> UnallocatedSet<ContactSubsectionRowID>;
    auto reindex(const ContactSectionSortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup(const identity::wot::claim::Group group) noexcept -> void;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ContactSubsection>;
