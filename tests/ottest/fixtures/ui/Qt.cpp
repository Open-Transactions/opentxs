// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/Qt.hpp>
#include <opentxs/opentxs.hpp>
#include <QAbstractItemModel>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QValidator>
#include <QVariant>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/util/P0330.hpp"
#include "ottest/Basic.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/AccountActivity.hpp"
#include "ottest/fixtures/ui/AccountList.hpp"
#include "ottest/fixtures/ui/AccountTree.hpp"
#include "ottest/fixtures/ui/BlockchainAccountStatus.hpp"
#include "ottest/fixtures/ui/BlockchainSelection.hpp"
#include "ottest/fixtures/ui/ContactActivity.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"
#include "ottest/fixtures/ui/NymList.hpp"
#include "ottest/fixtures/ui/SeedTree.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

constexpr auto account_activity_columns_{6};
constexpr auto account_list_columns_{4};
constexpr auto account_tree_columns_{1};
constexpr auto contact_activity_columns_{8};
constexpr auto blockchain_account_status_columns_{1};
constexpr auto blockchain_selection_columns_{1};
constexpr auto contact_list_columns_{1};
constexpr auto nym_list_columns_{1};
constexpr auto seed_tree_columns_{1};

auto check_qt_common(const QAbstractItemModel& model) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountActivityRow& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountCurrencyData& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountListRow& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountTreeRow& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const ContactActivityRow& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSelectionRow& expected,
    const int row) noexcept -> bool;
auto check_row(
    const ot::api::Crypto& crypto,
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountSourceData& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountData& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubchainData& expected,
    const int row) noexcept -> bool;
auto check_row(
    const User& user,
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const ContactListRow& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const NymListRow& expected,
    const int row) noexcept -> bool;
auto check_row(
    const ot::api::Crypto& crypto,
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const SeedTreeItem& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const SeedTreeNym& expected,
    const int row) noexcept -> bool;
}  // namespace ottest

namespace ottest
{
auto check_account_activity_qt(
    const User& user,
    const ot::identifier::Account& account,
    const AccountActivityData& expected) noexcept -> bool
{
    const auto* pModel =
        user.api_->UI().AccountActivityQt(user.nym_id_, account);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();
    const auto eDepositChains = [&] {
        const auto& input = expected.deposit_chains_;
        auto out = QVariantList{};
        std::transform(
            std::begin(input),
            std::end(input),
            std::back_inserter(out),
            [](const auto& in) -> auto { return static_cast<int>(in); });

        return out;
    }();
    const auto eProgress = [&] {
        const auto& progress = expected.progress_;
        auto out = QVariantList{};
        out.push_back(progress.first);
        out.push_back(progress.second);

        return out;
    }();

    for (const auto& [type, address] : expected.deposit_addresses_) {
        const auto value = model.getDepositAddress(static_cast<int>(type));
        output &= (value.toStdString() == address);

        EXPECT_EQ(value.toStdString(), address);
    }

    for (const auto& [input, valid] : expected.addresses_to_validate_) {
        const auto validated = model.validateAddress(input.c_str());
        output &= (validated == valid);

        EXPECT_EQ(validated, valid);
    }

    for (const auto& [input, valid] : expected.amounts_to_validate_) {
        const auto validated = model.validateAmount(input.c_str());
        output &= (validated.toStdString() == valid);

        EXPECT_EQ(validated.toStdString(), valid);
    }

    auto& amountValidator = *model.getAmountValidator();
    amountValidator.setScale(0);
    auto pos = 0;
    for (auto [input, valid] : expected.amounts_to_validate_) {
        auto inputString = QString{input.c_str()};
        const auto validated = amountValidator.validate(inputString, pos);
        output &= (validated == QValidator::State::Acceptable);

        EXPECT_EQ(validated, QValidator::State::Acceptable);
        EXPECT_EQ(inputString.toStdString(), valid);
    }

    output &= (model.accountID().toStdString() == expected.id_);
    output &=
        (model.displayBalance().toStdString() == expected.display_balance_);
    output &= (model.balancePolarity() == expected.polarity_);
    output &= (model.depositChains() == eDepositChains);
    output &=
        (std::to_string(model.syncPercentage()) ==
         std::to_string(expected.sync_));
    output &= (model.syncProgress() == eProgress);
    output &= (model.columnCount(parent) == account_activity_columns_);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.accountID().toStdString(), expected.id_);
    EXPECT_EQ(model.displayBalance().toStdString(), expected.display_balance_);
    EXPECT_EQ(model.balancePolarity(), expected.polarity_);
    EXPECT_EQ(model.depositChains(), eDepositChains);
    EXPECT_EQ(
        std::to_string(model.syncPercentage()), std::to_string(expected.sync_));
    EXPECT_EQ(model.syncProgress(), eProgress);
    EXPECT_EQ(model.columnCount(parent), account_activity_columns_);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_account_list_qt(
    const User& user,
    const AccountListData& expected) noexcept -> bool
{
    const auto* pModel = user.api_->UI().AccountListQt(user.nym_id_);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_account_tree_qt(
    const User& user,
    const AccountTreeData& expected) noexcept -> bool
{
    const auto* pModel = user.api_->UI().AccountTreeQt(user.nym_id_);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_contact_activity_qt(
    const User& user,
    const ot::identifier::Generic& contact,
    const ContactActivityData& expected) noexcept -> bool
{
    const auto* pModel =
        user.api_->UI().ContactActivityQt(user.nym_id_, contact);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();
    output &= (model.canMessage() == expected.can_message_);
    output &= (model.displayName().toStdString() == expected.display_name_);
    output &= (model.draft().toStdString() == expected.draft_);
    output &= (model.participants().toStdString() == expected.participants_);
    output &= (model.threadID().toStdString() == expected.thread_id_);
    output &= (model.columnCount(parent) == contact_activity_columns_);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.canMessage(), expected.can_message_);
    EXPECT_EQ(model.displayName().toStdString(), expected.display_name_);
    EXPECT_EQ(model.draft().toStdString(), expected.draft_);
    EXPECT_EQ(model.participants().toStdString(), expected.participants_);
    EXPECT_EQ(model.threadID().toStdString(), expected.thread_id_);
    EXPECT_EQ(model.columnCount(parent), contact_activity_columns_);
    EXPECT_EQ(model.columnCount(parent), contact_activity_columns_);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_blockchain_account_status_qt(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool
{
    const auto* pModel =
        user.api_->UI().BlockchainAccountStatusQt(user.nym_id_, chain);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();

    output &= (model.getNym().toStdString() == expected.owner_);
    output &=
        (static_cast<ot::blockchain::Type>(model.getChain()) ==
         expected.chain_);
    output &= (model.columnCount(parent) == blockchain_account_status_columns_);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.getNym().toStdString(), expected.owner_);
    EXPECT_EQ(
        static_cast<ot::blockchain::Type>(model.getChain()), expected.chain_);
    EXPECT_EQ(model.columnCount(parent), blockchain_account_status_columns_);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(
            user.api_->Crypto(), model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_blockchain_selection_qt(
    const ot::api::session::Client& api,
    const ot::ui::Blockchains type,
    const BlockchainSelectionData& expected) noexcept -> bool
{
    const auto* pModel = api.UI().BlockchainSelectionQt(type);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();
    const auto enabled = [&] {
        auto out = int{};

        for (const auto& row : expected.rows_) {
            if (row.enabled_) { ++out; }
        }

        return out;
    }();
    output &= (model.enabledCount() == enabled);
    output &= (model.columnCount(parent) == blockchain_selection_columns_);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.enabledCount(), enabled);
    EXPECT_EQ(model.columnCount(parent), blockchain_selection_columns_);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_contact_list_qt(
    const User& user,
    const ContactListData& expected) noexcept -> bool
{
    const auto* pModel = user.api_->UI().ContactListQt(user.nym_id_);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();
    output &= (model.columnCount(parent) == contact_list_columns_);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.columnCount(parent), contact_list_columns_);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(user, model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_messagable_list_qt(
    const User& user,
    const ContactListData& expected) noexcept -> bool
{
    const auto* pModel = user.api_->UI().MessagableListQt(user.nym_id_);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();
    output &= (model.columnCount(parent) == contact_list_columns_);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.columnCount(parent), contact_list_columns_);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(user, model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_nym_list_qt(
    const ot::api::session::Client& api,
    const NymListData& expected) noexcept -> bool
{
    const auto* pModel = api.UI().NymListQt();

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();
    output &= (model.columnCount(parent) == nym_list_columns_);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.columnCount(parent), nym_list_columns_);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &= check_row(model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_qt_common(const QAbstractItemModel& model) noexcept -> bool
{
    const auto* root = GetQT();

    EXPECT_NE(root, nullptr);

    if (nullptr == root) { return false; }

    auto output{true};
    output &= (model.thread() == root->thread());

    EXPECT_EQ(model.thread(), root->thread());

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountActivityRow& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::AccountActivityQt;
    auto output{true};

    for (auto column{0}; column < account_activity_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0;
        const auto rContacts = [&] {
            auto out = QStringList{};

            for (const auto& contact : expected.contacts_) {
                out.push_back(contact.c_str());
            }

            return out;
        }();
        const auto index = model.index(row, column, parent);
        const auto amount = model.data(index, Model::AmountRole);
        const auto text = model.data(index, Model::TextRole);
        const auto memo = model.data(index, Model::MemoRole);
        const auto time = model.data(index, Model::TimeRole);
        const auto uuid = model.data(index, Model::UUIDRole);
        const auto polarity = model.data(index, Model::PolarityRole);
        const auto contacts = model.data(index, Model::ContactsRole);
        const auto workflow = model.data(index, Model::WorkflowRole);
        const auto type = model.data(index, Model::TypeRole);
        const auto confirmations = model.data(index, Model::ConfirmationsRole);
        const auto display = model.data(index, Qt::DisplayRole);

        if (const auto& required = expected.timestamp_; required.has_value()) {
            const auto qt =
                ot::seconds_since_epoch(time.toDateTime().toSecsSinceEpoch())
                    .value();

            output &= (qt == required.value());

            EXPECT_EQ(qt, required.value());
        }

        switch (column) {
            case Model::TimeColumn: {
                output &= (display == time);

                EXPECT_EQ(display, time);
            } break;
            case Model::TextColumn: {
                output &= (display == text);

                EXPECT_EQ(display, text);
            } break;
            case Model::AmountColumn: {
                output &= (display == amount);

                EXPECT_EQ(display, amount);
            } break;
            case Model::UUIDColumn: {
                output &= (display == uuid);

                EXPECT_EQ(display, uuid);
            } break;
            case Model::MemoColumn: {
                output &= (display == memo);

                EXPECT_EQ(display, memo);
            } break;
            case Model::ConfirmationsColumn: {
                output &= (display == confirmations);

                EXPECT_EQ(display, confirmations);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (amount.toString().toStdString() == expected.display_amount_);
        output &= (text.toString().toStdString() == expected.text_);
        output &= (memo.toString().toStdString() == expected.memo_);
        output &= (uuid.toString().toStdString() == expected.uuid_);
        output &= (polarity.toInt() == expected.polarity_);
        output &= (confirmations.toInt() == expected.confirmations_);
        output &= (contacts.toStringList() == rContacts);
        output &= (workflow.toString().toStdString() == expected.workflow_);
        output &=
            (static_cast<ot::otx::client::StorageBox>(type.toInt()) ==
             expected.type_);
        output &= (model.columnCount(index) == account_activity_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(amount.toString().toStdString(), expected.display_amount_);
        EXPECT_EQ(text.toString().toStdString(), expected.text_);
        EXPECT_EQ(memo.toString().toStdString(), expected.memo_);
        EXPECT_EQ(uuid.toString().toStdString(), expected.uuid_);
        EXPECT_EQ(polarity.toInt(), expected.polarity_);
        EXPECT_EQ(confirmations.toInt(), expected.confirmations_);
        EXPECT_EQ(contacts.toStringList(), rContacts);
        EXPECT_EQ(workflow.toString().toStdString(), expected.workflow_);
        EXPECT_EQ(
            static_cast<ot::otx::client::StorageBox>(type.toInt()),
            expected.type_);
        EXPECT_EQ(model.columnCount(index), account_activity_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountCurrencyData& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::AccountTreeQt;
    auto output{true};

    for (auto column{0}; column < account_tree_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = expected.rows_.size();
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto unit = model.data(index, Model::UnitRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &= (static_cast<ot::UnitType>(unit.toInt()) == expected.type_);
        output &= (model.columnCount(index) == account_tree_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(static_cast<ot::UnitType>(unit.toInt()), expected.type_);
        EXPECT_EQ(model.columnCount(index), account_tree_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);

        if (0u == vCount) { return output; }

        auto it{expected.rows_.begin()};

        for (auto i = 0_uz; i < vCount; ++i, ++it) {
            output &= check_row(model, index, *it, static_cast<int>(i));
        }
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountListRow& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::AccountListQt;
    auto output{true};

    for (auto column{0}; column < account_list_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0;
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto notaryID = model.data(index, Model::NotaryIDRole);
        const auto notaryName = model.data(index, Model::NotaryNameRole);
        const auto unit = model.data(index, Model::UnitRole);
        const auto unitName = model.data(index, Model::UnitNameRole);
        const auto accountID = model.data(index, Model::AccountIDRole);
        const auto balance = model.data(index, Model::BalanceRole);
        const auto polarity = model.data(index, Model::PolarityRole);
        const auto type = model.data(index, Model::AccountTypeRole);
        const auto contract = model.data(index, Model::ContractIdRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NotaryNameColumn: {
                output &= (display == notaryName);

                EXPECT_EQ(display, notaryName);
            } break;
            case Model::DisplayUnitColumn: {
                output &= (display == unitName);

                EXPECT_EQ(display, unitName);
            } break;
            case Model::AccountNameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            case Model::DisplayBalanceColumn: {
                output &= (display == balance);

                EXPECT_EQ(display, balance);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &= (notaryID.toString().toStdString() == expected.notary_id_);
        output &=
            (notaryName.toString().toStdString() == expected.notary_name_);
        output &= (static_cast<ot::UnitType>(unit.toInt()) == expected.unit_);
        output &= (unitName.toString().toStdString() == expected.display_unit_);
        output &= (accountID.toString().toStdString() == expected.account_id_);
        output &=
            (balance.toString().toStdString() == expected.display_balance_);
        output &= (polarity.toInt() == expected.polarity_);
        output &=
            (static_cast<ot::AccountType>(type.toInt()) == expected.type_);
        output &= (contract.toString().toStdString() == expected.contract_id_);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(notaryID.toString().toStdString(), expected.notary_id_);
        EXPECT_EQ(notaryName.toString().toStdString(), expected.notary_name_);
        EXPECT_EQ(static_cast<ot::UnitType>(unit.toInt()), expected.unit_);
        EXPECT_EQ(unitName.toString().toStdString(), expected.display_unit_);
        EXPECT_EQ(accountID.toString().toStdString(), expected.account_id_);
        EXPECT_EQ(balance.toString().toStdString(), expected.display_balance_);
        EXPECT_EQ(polarity.toInt(), expected.polarity_);
        EXPECT_EQ(static_cast<ot::AccountType>(type.toInt()), expected.type_);
        EXPECT_EQ(contract.toString().toStdString(), expected.contract_id_);
        EXPECT_EQ(model.columnCount(index), account_list_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const AccountTreeRow& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::AccountTreeQt;
    auto output{true};

    for (auto column{0}; column < account_tree_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0;
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto notaryID = model.data(index, Model::NotaryIDRole);
        const auto notaryName = model.data(index, Model::NotaryNameRole);
        const auto unit = model.data(index, Model::UnitRole);
        const auto unitName = model.data(index, Model::UnitNameRole);
        const auto accountID = model.data(index, Model::AccountIDRole);
        const auto balance = model.data(index, Model::BalanceRole);
        const auto polarity = model.data(index, Model::PolarityRole);
        const auto type = model.data(index, Model::AccountTypeRole);
        const auto contract = model.data(index, Model::ContractIdRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &= (notaryID.toString().toStdString() == expected.notary_id_);
        output &=
            (notaryName.toString().toStdString() == expected.notary_name_);
        output &= (static_cast<ot::UnitType>(unit.toInt()) == expected.unit_);
        output &= (unitName.toString().toStdString() == expected.display_unit_);
        output &= (accountID.toString().toStdString() == expected.account_id_);
        output &=
            (balance.toString().toStdString() == expected.display_balance_);
        output &= (polarity.toInt() == expected.polarity_);
        output &=
            (static_cast<ot::AccountType>(type.toInt()) == expected.type_);
        output &= (contract.toString().toStdString() == expected.contract_id_);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(notaryID.toString().toStdString(), expected.notary_id_);
        EXPECT_EQ(notaryName.toString().toStdString(), expected.notary_name_);
        EXPECT_EQ(static_cast<ot::UnitType>(unit.toInt()), expected.unit_);
        EXPECT_EQ(unitName.toString().toStdString(), expected.display_unit_);
        EXPECT_EQ(accountID.toString().toStdString(), expected.account_id_);
        EXPECT_EQ(balance.toString().toStdString(), expected.display_balance_);
        EXPECT_EQ(polarity.toInt(), expected.polarity_);
        EXPECT_EQ(static_cast<ot::AccountType>(type.toInt()), expected.type_);
        EXPECT_EQ(contract.toString().toStdString(), expected.contract_id_);
        EXPECT_EQ(model.columnCount(index), account_tree_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const ContactActivityRow& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::ContactActivityQt;
    auto output{true};

    for (auto column{0}; column < contact_activity_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0;
        const auto loadingState =
            (expected.loading_) ? Qt::Checked : Qt::Unchecked;
        const auto pendingState =
            (expected.pending_) ? Qt::Checked : Qt::Unchecked;
        const auto index = model.index(row, column, parent);
        const auto amount = model.data(index, Model::AmountRole);
        const auto loading = model.data(index, Model::LoadingRole);
        const auto memo = model.data(index, Model::MemoRole);
        const auto pending = model.data(index, Model::PendingRole);
        const auto polarity = model.data(index, Model::PolarityRole);
        const auto text = model.data(index, Model::TextRole);
        const auto time = model.data(index, Model::TimeRole);
        const auto type = model.data(index, Model::TypeRole);
        const auto outgoing = model.data(index, Model::OutgoingRole);
        const auto from = model.data(index, Model::FromRole);
        const auto txid = model.data(index, Model::UUIDRole);
        const auto display = model.data(index, Qt::DisplayRole);
        const auto checked = model.data(index, Qt::CheckStateRole);

        if (const auto& required = expected.timestamp_; required.has_value()) {
            const auto qt =
                ot::seconds_since_epoch(time.toDateTime().toSecsSinceEpoch())
                    .value();

            output &= (qt == required.value());

            EXPECT_EQ(qt, required.value());
        }

        switch (column) {
            case Model::TimeColumn: {
                output &= (display == time);

                EXPECT_EQ(display, time);
            } break;
            case Model::FromColumn: {
                output &= (display == from);

                EXPECT_EQ(display, from);
            } break;
            case Model::TextColumn: {
                output &= (display == text);

                EXPECT_EQ(display, text);
            } break;
            case Model::AmountColumn: {
                output &= (display == amount);

                EXPECT_EQ(display, amount);
            } break;
            case Model::MemoColumn: {
                output &= (display == memo);

                EXPECT_EQ(display, memo);
            } break;
            case Model::LoadingColumn: {
                output &= (checked == loadingState);

                EXPECT_EQ(checked, loadingState);
            } break;
            case Model::PendingColumn: {
                output &= (checked == pendingState);

                EXPECT_EQ(checked, pendingState);
            } break;
            case Model::TxidColumn: {
                output &= (display == txid);

                EXPECT_EQ(display, txid);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (amount.toString().toStdString() == expected.display_amount_);
        output &= (loading.toBool() == expected.loading_);
        output &= (memo.toString().toStdString() == expected.memo_);
        output &= (pending.toBool() == expected.pending_);
        output &= (polarity == expected.polarity_);
        output &= (text.toString().toStdString() == expected.text_);
        output &=
            (static_cast<ot::otx::client::StorageBox>(type.toInt()) ==
             expected.type_);
        output &= (outgoing.toBool() == expected.outgoing_);
        output &= (from.toString().toStdString() == expected.from_);
        output &= (txid.toString().toStdString() == expected.txid_);
        output &= (model.columnCount(index) == contact_activity_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(amount.toString().toStdString(), expected.display_amount_);
        EXPECT_EQ(loading.toBool(), expected.loading_);
        EXPECT_EQ(memo.toString().toStdString(), expected.memo_);
        EXPECT_EQ(pending.toBool(), expected.pending_);
        EXPECT_EQ(polarity, expected.polarity_);
        EXPECT_EQ(text.toString().toStdString(), expected.text_);
        EXPECT_EQ(
            static_cast<ot::otx::client::StorageBox>(type.toInt()),
            expected.type_);
        EXPECT_EQ(outgoing.toBool(), expected.outgoing_);
        EXPECT_EQ(from.toString().toStdString(), expected.from_);
        EXPECT_EQ(txid.toString().toStdString(), expected.txid_);
        EXPECT_EQ(model.columnCount(index), contact_activity_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSelectionRow& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::BlockchainSelectionQt;
    auto output{true};

    for (auto column{0}; column < blockchain_selection_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0;
        const auto checkedState =
            (expected.enabled_) ? Qt::Checked : Qt::Unchecked;
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto type = model.data(index, Model::TypeRole);
        const auto isEnabled = model.data(index, Model::IsEnabled);
        const auto isTestnet = model.data(index, Model::IsTestnet);
        const auto display = model.data(index, Qt::DisplayRole);
        const auto checked = model.data(index, Qt::CheckStateRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &=
            (static_cast<ot::blockchain::Type>(type.toInt()) == expected.type_);
        output &= (isEnabled.toBool() == expected.enabled_);
        output &= (isTestnet.toBool() == expected.testnet_);
        output &= (checked == checkedState);
        output &= (model.columnCount(index) == blockchain_selection_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(
            static_cast<ot::blockchain::Type>(type.toInt()), expected.type_);
        EXPECT_EQ(isEnabled.toBool(), expected.enabled_);
        EXPECT_EQ(isTestnet.toBool(), expected.testnet_);
        EXPECT_EQ(checked, checkedState);
        EXPECT_EQ(model.columnCount(index), blockchain_selection_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_row(
    const ot::api::Crypto& crypto,
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountSourceData& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::BlockchainAccountStatusQt;
    auto output{true};

    for (auto column{0}; column < blockchain_account_status_columns_;
         ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = expected.rows_.size();
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto id = model.data(index, Model::SourceIDRole);
        const auto type = model.data(index, Model::SubaccountTypeRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &=
            (id.toString().toStdString() == expected.id_.asBase58(crypto));
        output &=
            (static_cast<ot::blockchain::crypto::SubaccountType>(
                 type.toInt()) == expected.type_);
        output &=
            (model.columnCount(index) == blockchain_account_status_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(id.toString().toStdString(), expected.id_.asBase58(crypto));
        EXPECT_EQ(
            static_cast<ot::blockchain::crypto::SubaccountType>(type.toInt()),
            expected.type_);
        EXPECT_EQ(model.columnCount(index), blockchain_account_status_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);

        if (0u == vCount) { return output; }

        auto it{expected.rows_.begin()};

        for (auto i = 0_uz; i < vCount; ++i, ++it) {
            output &= check_row(model, index, *it, static_cast<int>(i));
        }
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountData& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::BlockchainAccountStatusQt;
    auto output{true};

    for (auto column{0}; column < blockchain_account_status_columns_;
         ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = expected.rows_.size();
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto id = model.data(index, Model::SubaccountIDRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &= (id.toString().toStdString() == expected.id_);
        output &=
            (model.columnCount(index) == blockchain_account_status_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(id.toString().toStdString(), expected.id_);
        EXPECT_EQ(model.columnCount(index), blockchain_account_status_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);

        if (0u == vCount) { return output; }

        auto it{expected.rows_.begin()};

        for (auto i = 0_uz; i < vCount; ++i, ++it) {
            output &= check_row(model, index, *it, static_cast<int>(i));
        }
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubchainData& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::BlockchainAccountStatusQt;
    auto output{true};

    for (auto column{0}; column < blockchain_account_status_columns_;
         ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0u;
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto type = model.data(index, Model::SubchainTypeRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &=
            (static_cast<ot::blockchain::crypto::Subchain>(type.toInt()) ==
             expected.type_);
        output &=
            (model.columnCount(index) == blockchain_account_status_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(
            static_cast<ot::blockchain::crypto::Subchain>(type.toInt()),
            expected.type_);
        EXPECT_EQ(model.columnCount(index), blockchain_account_status_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_row(
    const User& user,
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const ContactListRow& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::ContactListQt;
    auto output{true};

    for (auto column{0}; column < contact_list_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0;
        const auto index = model.index(row, column, parent);
        const auto& cIndex = expected.contact_id_index_;
        const auto id = model.data(index, Model::IDRole);
        const auto name = model.data(index, Model::NameRole);
        const auto section = model.data(index, Model::SectionRole);
        const auto display = model.data(index, Qt::DisplayRole);

        if (auto required = user.Contact(cIndex).asBase58(user.api_->Crypto());
            required.empty()) {
            const auto set =
                user.SetContact(cIndex, id.toString().toStdString());
            required = user.Contact(cIndex).asBase58(user.api_->Crypto());
            output &= set;
            output &= (false == required.empty());

            EXPECT_TRUE(set);
            EXPECT_FALSE(required.empty());
        } else {
            const auto actual = id.toString().toStdString();
            output &= (required == actual);

            EXPECT_EQ(actual, required);
        }

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &= (section.toString().toStdString() == expected.section_);
        output &= (model.columnCount(index) == contact_list_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(section.toString().toStdString(), expected.section_);
        EXPECT_EQ(model.columnCount(index), contact_list_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const NymListRow& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::NymListQt;
    auto output{true};

    for (auto column{0}; column < nym_list_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0u;
        const auto index = model.index(row, column, parent);
        const auto name = model.data(index, Model::NameRole);
        const auto id = model.data(index, Model::IDRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (name.toString().toStdString() == expected.name_);
        output &= (id.toString().toStdString() == expected.id_);
        output &= (model.columnCount(index) == nym_list_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(id.toString().toStdString(), expected.id_);
        EXPECT_EQ(model.columnCount(index), nym_list_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}

auto check_seed_tree_qt(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool
{
    const auto* pModel = api.UI().SeedTreeQt();

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = 0_uz; i < vCount; ++i, ++it) {
        output &=
            check_row(api.Crypto(), model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_row(
    const ot::api::Crypto& crypto,
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const SeedTreeItem& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::SeedTreeQt;
    auto output{true};

    for (auto column{0}; column < seed_tree_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = expected.rows_.size();
        const auto index = model.index(row, column, parent);
        const auto id = model.data(index, Model::SeedIDRole);
        const auto name = model.data(index, Model::SeedNameRole);
        const auto type = model.data(index, Model::SeedTypeRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &=
                    (display.toString().toStdString() ==
                     name.toString().toStdString());

                EXPECT_EQ(
                    display.toString().toStdString(),
                    name.toString().toStdString());
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &=
            (id.toString().toStdString() == expected.id_.asBase58(crypto));
        output &= (name.toString().toStdString() == expected.name_);
        output &=
            (static_cast<ot::crypto::SeedStyle>(type.toInt()) ==
             expected.type_);
        output &= (model.columnCount(index) == seed_tree_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(id.toString().toStdString(), expected.id_.asBase58(crypto));
        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(
            static_cast<ot::crypto::SeedStyle>(type.toInt()), expected.type_);
        EXPECT_EQ(model.columnCount(index), seed_tree_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);

        if (0u == vCount) { return output; }

        auto it{expected.rows_.begin()};

        for (auto i = 0_uz; i < vCount; ++i, ++it) {
            output &= check_row(model, index, *it, static_cast<int>(i));
        }
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const SeedTreeNym& expected,
    const int row) noexcept -> bool
{
    using Model = ot::ui::SeedTreeQt;
    auto output{true};

    for (auto column{0}; column < seed_tree_columns_; ++column) {
        const auto exists = model.hasIndex(row, column, parent);
        output &= exists;

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto vCount = 0u;
        const auto index = model.index(row, column, parent);
        const auto position = model.data(index, Model::NymIndexRole);
        const auto id = model.data(index, Model::NymIDRole);
        const auto name = model.data(index, Model::NymNameRole);
        const auto display = model.data(index, Qt::DisplayRole);

        switch (column) {
            case Model::NameColumn: {
                output &= (display == name);

                EXPECT_EQ(display, name);
            } break;
            default: {
                output &= false;

                EXPECT_TRUE(false);
            }
        }

        output &= (position.toULongLong() == expected.index_);
        output &= (id.toString().toStdString() == expected.id_);
        output &= (name.toString().toStdString() == expected.name_);
        output &= (model.columnCount(index) == seed_tree_columns_);
        output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

        EXPECT_EQ(position.toULongLong(), expected.index_);
        EXPECT_EQ(id.toString().toStdString(), expected.id_);
        EXPECT_EQ(name.toString().toStdString(), expected.name_);
        EXPECT_EQ(model.columnCount(index), seed_tree_columns_);
        EXPECT_EQ(model.rowCount(index), vCount);
    }

    return output;
}
}  // namespace ottest
