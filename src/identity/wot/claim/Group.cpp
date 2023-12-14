// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/Group.hpp"  // IWYU pragma: associated

#include <ContactSection.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/ClaimType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Item.internal.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::identity::wot::claim
{
struct Group::Imp {
    const UnallocatedCString nym_{};
    const claim::SectionType section_{claim::SectionType::Error};
    const claim::ClaimType type_{claim::ClaimType::Error};
    const identifier::Generic primary_;
    const ItemMap items_{};

    static auto get_primary_item(const ItemMap& items) -> identifier::Generic
    {
        for (const auto& it : items) {
            const auto& p = it.second;
            assert_false(nullptr == p);
            const auto& item = *p;

            if (item.HasAttribute(Attribute::Primary)) { return item.ID(); }
        }

        return {};
    }

    Imp(const UnallocatedCString& nym,
        const claim::SectionType section,
        const claim::ClaimType type,
        const ItemMap& items)
        : nym_(nym)
        , section_(section)
        , type_(type)
        , primary_(get_primary_item(items))
        , items_(normalize_items(primary_, items))
    {
    }

    Imp(const Imp& rhs)

        = default;

    Imp(Imp&& rhs)
        : nym_(std::move(const_cast<UnallocatedCString&>(rhs.nym_)))
        , section_(rhs.section_)
        , type_(rhs.type_)
        , primary_(std::move(const_cast<identifier::Generic&>(rhs.primary_)))
        , items_(std::move(const_cast<ItemMap&>(rhs.items_)))
    {
    }

    static auto normalize_items(const identifier::Generic& primary, ItemMap map)
        -> Group::ItemMap
    {
        for (auto& it : map) {
            auto& i = *it.second;

            if (i.HasAttribute(Attribute::Primary) && (i.ID() != primary)) {
                i.Remove(Attribute::Primary);
            }
        }

        return map;
    }
};

static auto create_item(const std::shared_ptr<Item>& item) -> Group::ItemMap
{
    assert_false(nullptr == item);

    Group::ItemMap output{};
    output[item->ID()] = item;

    return output;
}

Group::Group(
    const UnallocatedCString& nym,
    const claim::SectionType section,
    const claim::ClaimType type,
    const ItemMap& items)
    : imp_(std::make_unique<Imp>(nym, section, type, items))
{
    assert_false(nullptr == imp_);
}

Group::Group(
    const UnallocatedCString& nym,
    const claim::SectionType section,
    const std::shared_ptr<Item>& item)
    : Group(nym, section, item->Type(), create_item(item))
{
    assert_false(nullptr == item);
}

Group::Group(const Group& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    assert_false(nullptr == imp_);
}

Group::Group(Group&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    assert_false(nullptr == imp_);
}

auto Group::operator+(const Group& rhs) const -> Group
{
    assert_true(imp_->section_ == rhs.imp_->section_);

    const auto& primary =
        imp_->primary_.empty() ? rhs.imp_->primary_ : imp_->primary_;
    auto copy{imp_->items_};
    std::ranges::copy(rhs.imp_->items_, std::inserter(copy, copy.end()));
    const auto set_primary = [&](auto& item) {
        if (item->ID() == primary) {
            item->Add(Attribute::Primary);
        } else {
            item->Remove(Attribute::Primary);
        }
    };
    std::ranges::for_each(copy | std::views::values, set_primary);

    return {imp_->nym_, imp_->section_, imp_->type_, copy};
}

auto Group::AddItem(const std::shared_ptr<Item>& item) const -> Group
{
    assert_false(nullptr == item);

    if (item->HasAttribute(Attribute::Primary)) { return AddPrimary(item); }

    const auto& id = item->ID();
    const bool alreadyExists =
        (1 == imp_->items_.count(id)) && (*item == *imp_->items_.at(id));

    if (alreadyExists) { return *this; }

    auto map = imp_->items_;
    map[id] = item;

    return {imp_->nym_, imp_->section_, imp_->type_, map};
}

auto Group::AddPrimary(const std::shared_ptr<Item>& item) const -> Group
{
    if (false == bool(item)) { return *this; }

    const auto& incomingID = item->ID();
    const bool isExistingPrimary = (imp_->primary_ == incomingID);
    const bool haveExistingPrimary =
        ((false == imp_->primary_.empty()) && (false == isExistingPrimary));

    auto copy = imp_->items_;

    if (haveExistingPrimary) {
        copy[imp_->primary_]->Remove(Attribute::Primary);
    }

    auto [it, _] = copy.try_emplace(incomingID, item);
    it->second->Add(Attribute::Primary);

    return {imp_->nym_, imp_->section_, imp_->type_, copy};
}

auto Group::begin() const -> Group::ItemMap::const_iterator
{
    return imp_->items_.cbegin();
}

auto Group::Best() const -> std::shared_ptr<Item>
{
    if (0 == imp_->items_.size()) { return {}; }

    if (false == imp_->primary_.empty()) {
        return imp_->items_.at(imp_->primary_);
    }

    for (const auto& it : imp_->items_) {
        const auto& claim = it.second;

        assert_false(nullptr == claim);

        if (claim->HasAttribute(Attribute::Active)) { return claim; }
    }

    return imp_->items_.begin()->second;
}

auto Group::Claim(const identifier::Generic& item) const
    -> std::shared_ptr<Item>
{
    auto it = imp_->items_.find(item);

    if (imp_->items_.end() == it) { return {}; }

    return it->second;
}

auto Group::Delete(const identifier::Generic& id) const -> Group
{
    const bool exists = (1 == imp_->items_.count(id));

    if (false == exists) { return *this; }

    auto map = imp_->items_;
    map.erase(id);

    return {imp_->nym_, imp_->section_, imp_->type_, map};
}

auto Group::end() const -> Group::ItemMap::const_iterator
{
    return imp_->items_.cend();
}

auto Group::HaveClaim(const identifier::Generic& item) const -> bool
{
    return (1 == imp_->items_.count(item));
}

auto Group::Primary() const -> const identifier::Generic&
{
    return imp_->primary_;
}

auto Group::SerializeTo(proto::ContactSection& section, const bool withIDs)
    const -> bool
{
    if (translate(section.name()) != imp_->section_) {
        LogError()()("Trying to serialize to incorrect section.").Flush();

        return false;
    }

    for (const auto& it : imp_->items_) {
        const auto& item = it.second;

        assert_false(nullptr == item);

        item->Internal().Serialize(*section.add_item(), withIDs);
    }

    return true;
}

auto Group::PrimaryClaim() const -> std::shared_ptr<Item>
{
    if (imp_->primary_.empty()) { return {}; }

    return imp_->items_.at(imp_->primary_);
}

auto Group::Size() const -> std::size_t { return imp_->items_.size(); }

auto Group::Type() const -> const claim::ClaimType& { return imp_->type_; }

Group::~Group() = default;
}  // namespace opentxs::identity::wot::claim
