// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Context.hpp"  // IWYU pragma: associated

#include <QQmlEngine>  // IWYU pragma: keep

#include "opentxs/Qt.hpp"
#include "opentxs/UnitType.hpp"               // IWYU pragma: keep
#include "opentxs/blockchain/Type.hpp"        // IWYU pragma: keep
#include "opentxs/otx/client/StorageBox.hpp"  // IWYU pragma: keep

namespace opentxs
{
auto RegisterQMLTypes() noexcept -> void
{
    qmlRegisterUncreatableMetaObject(
        opentxs::unittype::staticMetaObject,
        "org.opentransactions.unittype",
        VersionMajor(),
        VersionMinor(),
        "UnitType",
        "Access to opentxs::UnitType enum class");
    qmlRegisterUncreatableMetaObject(
        opentxs::blockchain::type::staticMetaObject,
        "org.opentransactions.blockchain.type",
        VersionMajor(),
        VersionMinor(),
        "Type",
        "Access to opentxs::blockchain::Type enum class");
    qmlRegisterUncreatableMetaObject(
        opentxs::otx::client::storagebox::staticMetaObject,
        "org.opentransactions.otx.client.storagebox",
        VersionMajor(),
        VersionMinor(),
        "StorageBox",
        "Access to opentxs::otx::client::StorageBox enum class");

    qmlRegisterUncreatableType<opentxs::ui::AccountActivityQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "AccountActivityQt",
        "An AccountActivityQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::AccountListQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "AccountListQt",
        "An AccountListQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::AccountSummaryQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "AccountSummaryQt",
        "An AccountSummaryQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::AccountTreeQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "AccountTreeQt",
        "An AccountTreeQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::BlockchainAccountStatusQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "BlockchainAccountStatusQt",
        "A BlockchainAccountStatusQt object cannot be created directly in "
        "QML.");
    qmlRegisterUncreatableType<opentxs::ui::BlockchainSelectionQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "BlockchainSelectionQt",
        "A BlockchainSelectionQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::BlockchainStatisticsQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "BlockchainStatisticsQt",
        "A BlockchainStatisticsQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::ContactActivityQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "ContactActivityQt",
        "A ContactActivityQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::ContactListQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "ContactListQt",
        "A ContactListQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::NymListQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "NymListQt",
        "A NymListQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::NymType>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "NymType",
        "A NymType object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::PayableListQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "PayableListQt",
        "A PayableListQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::SeedListQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "SeedListQt",
        "A SeedListQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::SeedTreeQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "SeedTreeQt",
        "A SeedTreeQt object cannot be created directly in QML.");
    qmlRegisterUncreatableType<opentxs::ui::UnitListQt>(
        "org.opentransactions.ui",
        VersionMajor(),
        VersionMinor(),
        "UnitListQt",
        "A UnitListQt object cannot be created directly in QML.");
}
}  // namespace opentxs
