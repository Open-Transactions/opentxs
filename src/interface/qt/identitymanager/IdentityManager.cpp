// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/qt/identitymanager/IdentityManager.hpp"  // IWYU pragma: associated
#include "internal/interface/qt/Factory.hpp"         // IWYU pragma: associated
#include "opentxs/interface/qt/IdentityManager.hpp"  // IWYU pragma: associated

#include <QAbstractItemModel>
#include <memory>
#include <string>
#include <utility>

#include "interface/qt/identitymanager/NymType.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/interface/qt/AccountActivity.hpp"
#include "opentxs/interface/qt/AccountList.hpp"
#include "opentxs/interface/qt/AccountTree.hpp"
#include "opentxs/interface/qt/BlockchainAccountStatus.hpp"
#include "opentxs/interface/qt/ContactActivity.hpp"
#include "opentxs/interface/qt/ContactActivityFilterable.hpp"
#include "opentxs/interface/qt/ContactList.hpp"
#include "opentxs/interface/qt/NymList.hpp"
#include "opentxs/interface/qt/Profile.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto IdentityManagerQt(const api::session::Client& api) noexcept
    -> ui::IdentityManagerQt
{
    using ReturnType = ui::IdentityManagerQt;

    return std::make_unique<ReturnType::Imp>(api).release();
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
IdentityManagerQt::Imp::Imp(const api::session::Client& api) noexcept
    : api_(api)
    , parent_(nullptr)
    , active_nym_()
{
}

auto IdentityManagerQt::Imp::getAccountActivity(
    const QString& accountID) const noexcept -> AccountActivityQt*
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    if (id.empty()) {
        parent_->needNym();

        return nullptr;
    }

    return api_.UI().AccountActivityQt(
        id, api_.Factory().AccountIDFromBase58(accountID.toStdString()));
}

auto IdentityManagerQt::Imp::getAccountList() const noexcept -> AccountListQt*
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    if (id.empty()) {
        parent_->needNym();

        return nullptr;
    }

    return api_.UI().AccountListQt(id);
}

auto IdentityManagerQt::Imp::getAccountStatus(
    const QString& accountID) const noexcept -> BlockchainAccountStatusQt*
{
    const auto id = api_.Factory().AccountIDFromBase58(accountID.toStdString());
    const auto [chain, nymID] = api_.Crypto().Blockchain().LookupAccount(id);

    if (blockchain::Type::UnknownBlockchain == chain) { return nullptr; }

    return api_.UI().BlockchainAccountStatusQt(nymID, chain);
}

auto IdentityManagerQt::Imp::getAccountTree() const noexcept -> AccountTreeQt*
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    if (id.empty()) {
        parent_->needNym();

        return nullptr;
    }

    return api_.UI().AccountTreeQt(id);
}

auto IdentityManagerQt::Imp::getActiveNym() const noexcept -> QString
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    return QString::fromStdString(id.asBase58(api_.Crypto()));
}

auto IdentityManagerQt::Imp::getContactActivity(
    const QString& contactID) const noexcept -> ContactActivityQt*
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    if (id.empty()) {
        parent_->needNym();

        return nullptr;
    }

    return api_.UI().ContactActivityQt(
        id, api_.Factory().IdentifierFromBase58(contactID.toStdString()));
}

auto IdentityManagerQt::Imp::getContactActivityFilterable(
    const QString& contactID) const noexcept -> ContactActivityQtFilterable*
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    if (id.empty()) {
        parent_->needNym();

        return nullptr;
    }

    return api_.UI().ContactActivityQtFilterable(
        id, api_.Factory().IdentifierFromBase58(contactID.toStdString()));
}

auto IdentityManagerQt::Imp::getContactList() const noexcept -> ContactListQt*
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    if (id.empty()) {
        parent_->needNym();

        return nullptr;
    }

    return api_.UI().ContactListQt(id);
}

auto IdentityManagerQt::Imp::getNymList() const noexcept -> NymListQt*
{
    return api_.UI().NymListQt();
}

auto IdentityManagerQt::Imp::getNymType() const noexcept -> QAbstractListModel*
{
    static auto model = identitymanager::NymType{};

    return &model;
}

auto IdentityManagerQt::Imp::getProfile() const noexcept -> ProfileQt*
{
    auto handle = active_nym_.lock_shared();
    const auto& id = *handle;

    if (id.empty()) {
        parent_->needNym();

        return nullptr;
    }

    return api_.UI().ProfileQt(id);
}

auto IdentityManagerQt::Imp::init(IdentityManagerQt* parent) noexcept -> void
{
    parent_ = parent;
}

auto IdentityManagerQt::Imp::setActiveNym(QString id) noexcept -> void
{
    auto newID = api_.Factory().NymIDFromBase58(id.toStdString());
    auto changed{false};
    active_nym_.modify([&](auto& value) {
        changed = (newID != value);
        value = std::move(newID);
        id = QString::fromStdString(value.asBase58(api_.Crypto()));
    });

    if (changed) { parent_->activeNymChanged(std::move(id)); }
}
}  // namespace opentxs::ui

namespace opentxs::ui
{
IdentityManagerQt::IdentityManagerQt(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp);

    imp_->init(this);
}

IdentityManagerQt::IdentityManagerQt(IdentityManagerQt&& rhs) noexcept
    : imp_(std::exchange(rhs.imp_, nullptr))
{
}

auto IdentityManagerQt::getAccountActivity(
    const QString& accountID) const noexcept -> AccountActivityQt*
{
    return imp_->getAccountActivity(accountID);
}

auto IdentityManagerQt::getAccountActivityQML(
    const QString& accountID) const noexcept -> QObject*
{
    return getAccountActivity(accountID);
}

auto IdentityManagerQt::getAccountList() const noexcept -> AccountListQt*
{
    return imp_->getAccountList();
}

auto IdentityManagerQt::getAccountListQML() const noexcept -> QObject*
{
    return getAccountList();
}

auto IdentityManagerQt::getAccountStatus(
    const QString& accountID) const noexcept -> BlockchainAccountStatusQt*
{
    return imp_->getAccountStatus(accountID);
}

auto IdentityManagerQt::getAccountStatusQML(
    const QString& accountID) const noexcept -> QObject*
{
    return getAccountStatus(accountID);
}

auto IdentityManagerQt::getAccountTree() const noexcept -> AccountTreeQt*
{
    return imp_->getAccountTree();
}

auto IdentityManagerQt::getAccountTreeQML() const noexcept -> QObject*
{
    return imp_->getAccountTree();
}

auto IdentityManagerQt::getActiveNym() const noexcept -> QString
{
    return imp_->getActiveNym();
}

auto IdentityManagerQt::getContactActivity(
    const QString& contactID) const noexcept -> ContactActivityQt*
{
    return imp_->getContactActivity(contactID);
}

auto IdentityManagerQt::getContactActivityFilterable(
    const QString& contactID) const noexcept -> ContactActivityQtFilterable*
{
    return imp_->getContactActivityFilterable(contactID);
}

auto IdentityManagerQt::getContactActivityQML(
    const QString& contactID) const noexcept -> QObject*
{
    return getContactActivity(contactID);
}

auto IdentityManagerQt::getContactActivityFilterableQML(
    const QString& contactID) const noexcept -> QObject*
{
    return getContactActivityFilterable(contactID);
}

auto IdentityManagerQt::getContactList() const noexcept -> ContactListQt*
{
    return imp_->getContactList();
}

auto IdentityManagerQt::getContactListQML() const noexcept -> QObject*
{
    return getContactList();
}

auto IdentityManagerQt::getNymList() const noexcept -> NymListQt*
{
    return imp_->getNymList();
}

auto IdentityManagerQt::getNymListQML() const noexcept -> QObject*
{
    return getNymList();
}

auto IdentityManagerQt::getNymType() const noexcept -> QAbstractListModel*
{
    return imp_->getNymType();
}

auto IdentityManagerQt::getNymTypeQML() const noexcept -> QObject*
{
    return getNymType();
}

auto IdentityManagerQt::getProfile() const noexcept -> ProfileQt*
{
    return imp_->getProfile();
}

auto IdentityManagerQt::getProfileQML() const noexcept -> QObject*
{
    return getProfile();
}

auto IdentityManagerQt::setActiveNym(QString id) noexcept -> void
{
    imp_->setActiveNym(std::move(id));
}

IdentityManagerQt::~IdentityManagerQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui
