// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/ContactActivityFilterable.hpp"  // IWYU pragma: associated

#include <QDateTime>
#include <QVariant>
#include <memory>
#include <utility>

#include "opentxs/interface/qt/ContactActivity.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui
{
struct ContactActivityQtFilterable::Imp {
    enum class Type {
        Blacklist,
        Whitelist,
    };

    ContactActivityQt& parent_;
    Type type_;
    Set<int> filter_;

    Imp(ContactActivityQt& parent) noexcept
        : parent_(parent)
        , type_()
        , filter_()
    {
    }
};

ContactActivityQtFilterable::ContactActivityQtFilterable(
    ContactActivityQt& parent) noexcept
    : imp_(std::make_unique<Imp>(parent).release())
{
    auto* p = std::addressof(parent);
    setSourceModel(p);
    connect(
        p,
        &ContactActivityQt::canMessageUpdate,
        this,
        &ContactActivityQtFilterable::canMessageUpdate);
    connect(
        p,
        &ContactActivityQt::displayNameUpdate,
        this,
        &ContactActivityQtFilterable::displayNameUpdate);
    connect(
        p,
        &ContactActivityQt::draftUpdate,
        this,
        &ContactActivityQtFilterable::draftUpdate);
}

auto ContactActivityQtFilterable::blacklistType(int type) -> void
{
    using enum Imp::Type;

    if (imp_->type_ != Blacklist) {
        imp_->filter_.clear();
        imp_->type_ = Blacklist;
    }

    imp_->filter_.emplace(type);
    invalidateFilter();
}

auto ContactActivityQtFilterable::canMessage() const noexcept -> bool
{
    return imp_->parent_.canMessage();
}

auto ContactActivityQtFilterable::displayName() const noexcept -> QString
{
    return imp_->parent_.displayName();
}

auto ContactActivityQtFilterable::draft() const noexcept -> QString
{
    return imp_->parent_.draft();
}

auto ContactActivityQtFilterable::draftValidator() const noexcept -> QValidator*
{
    return imp_->parent_.draftValidator();
}

auto ContactActivityQtFilterable::filterAcceptsRow(
    int source_row,
    const QModelIndex& source_parent) const -> bool
{
    using enum Imp::Type;
    using enum ContactActivityQt::Roles;
    const auto& parent = imp_->parent_;
    const auto index = parent.index(source_row, 0, source_parent);
    const auto type = parent.data(index, TypeRole);

    if (Blacklist == imp_->type_) {

        return false == imp_->filter_.contains(type.toInt());
    } else {

        return imp_->filter_.contains(type.toInt());
    }
}

auto ContactActivityQtFilterable::lessThan(
    const QModelIndex& source_left,
    const QModelIndex& source_right) const -> bool
{
    using enum ContactActivityQt::Roles;
    const auto& parent = imp_->parent_;
    const auto lhs = parent.data(source_left, TimeRole).toDateTime();
    const auto rhs = parent.data(source_right, TimeRole).toDateTime();

    return lhs < rhs;
}

auto ContactActivityQtFilterable::participants() const noexcept -> QString
{
    return imp_->parent_.participants();
}

auto ContactActivityQtFilterable::pay(
    const QString& amount,
    const QString& sourceAccount,
    const QString& memo) const noexcept -> bool
{
    return imp_->parent_.pay(amount, sourceAccount, memo);
}

auto ContactActivityQtFilterable::paymentCode(const int currency) const noexcept
    -> QString
{
    return imp_->parent_.paymentCode(currency);
}

auto ContactActivityQtFilterable::sendDraft() const noexcept -> bool
{
    return imp_->parent_.sendDraft();
}

auto ContactActivityQtFilterable::sendFaucetRequest(
    const int currency) const noexcept -> bool
{
    return imp_->parent_.sendFaucetRequest(currency);
}

auto ContactActivityQtFilterable::setDraft(QString value) -> void
{
    return imp_->parent_.setDraft(value);
}

auto ContactActivityQtFilterable::threadID() const noexcept -> QString
{
    return imp_->parent_.threadID();
}

auto ContactActivityQtFilterable::whitelistType(int type) -> void
{
    using enum Imp::Type;

    if (imp_->type_ != Whitelist) {
        imp_->filter_.clear();
        imp_->type_ = Whitelist;
    }

    imp_->filter_.emplace(type);
    invalidateFilter();
}

ContactActivityQtFilterable::~ContactActivityQtFilterable()
{
    if (nullptr != imp_) {
        setSourceModel(nullptr);
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui
