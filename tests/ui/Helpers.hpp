// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/.hpp"
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"
// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"
// IWYU pragma: no_include "opentxs/interface/ui/Blockchains.hpp"

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>

#include "Basic.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/interface/ui/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

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

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
struct User;

struct Counter {
    std::atomic_int expected_{};
    std::atomic_int updated_{};
};

struct AccountActivityRow {
    ot::StorageBox type_{};
    int polarity_{};
    ot::Amount amount_{};
    ot::UnallocatedCString display_amount_{};
    ot::UnallocatedVector<ot::UnallocatedCString> contacts_{};
    ot::UnallocatedCString memo_{};
    ot::UnallocatedCString workflow_{};
    ot::UnallocatedCString text_{};
    ot::UnallocatedCString uuid_{};
    std::optional<ot::Time> timestamp_{};
    int confirmations_{};
};

struct AccountActivityData {
    ot::AccountType type_;
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
    ot::UnitType unit_;
    ot::UnallocatedCString contract_id_{};
    ot::UnallocatedCString contract_name_{};
    ot::UnallocatedCString notary_id_{};
    ot::UnallocatedCString notary_name_{};
    int polarity_{};
    ot::Amount balance_{};
    ot::UnallocatedCString display_balance_{};
    ot::UnallocatedCString default_deposit_address_{};
    ot::UnallocatedMap<ot::blockchain::Type, ot::UnallocatedCString>
        deposit_addresses_{};
    ot::UnallocatedVector<ot::blockchain::Type> deposit_chains_{};
    double sync_{};
    std::pair<int, int> progress_{};
    ot::UnallocatedVector<std::pair<ot::UnallocatedCString, bool>>
        addresses_to_validate_{};
    ot::UnallocatedVector<
        std::pair<ot::UnallocatedCString, ot::UnallocatedCString>>
        amounts_to_validate_{};
    ot::UnallocatedVector<AccountActivityRow> rows_{};
};

struct AccountListRow {
    ot::UnallocatedCString account_id_{};
    ot::UnallocatedCString contract_id_{};
    ot::UnallocatedCString display_unit_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedCString notary_id_{};
    ot::UnallocatedCString notary_name_{};
    ot::AccountType type_{};
    ot::UnitType unit_{};
    int polarity_{};
    ot::Amount balance_{};
    ot::UnallocatedCString display_balance_{};
};

struct AccountListData {
    ot::UnallocatedVector<AccountListRow> rows_{};
};

struct AccountTreeRow {
    ot::UnallocatedCString account_id_{};
    ot::UnallocatedCString contract_id_{};
    ot::UnallocatedCString display_unit_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedCString notary_id_{};
    ot::UnallocatedCString notary_name_{};
    ot::AccountType type_{};
    ot::UnitType unit_{};
    int polarity_{};
    ot::Amount balance_{};
    ot::UnallocatedCString display_balance_{};
};

struct AccountCurrencyData {
    ot::UnitType type_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedVector<AccountTreeRow> rows_{};
};

struct AccountTreeData {
    ot::UnallocatedVector<AccountCurrencyData> rows_{};
};

struct ActivityThreadRow {
    bool loading_{};
    bool pending_{};
    bool outgoing_{};
    int polarity_{};
    ot::Amount amount_{};
    ot::UnallocatedCString display_amount_{};
    ot::UnallocatedCString from_{};
    ot::UnallocatedCString text_{};
    ot::UnallocatedCString memo_{};
    ot::StorageBox type_{};
    std::optional<ot::Time> timestamp_{};
};

struct ActivityThreadData {
    bool can_message_{};
    ot::UnallocatedCString thread_id_{};
    ot::UnallocatedCString display_name_{};
    ot::UnallocatedCString draft_{};
    ot::UnallocatedCString participants_{};
    ot::UnallocatedMap<ot::UnitType, ot::UnallocatedCString> payment_codes_{};
    ot::UnallocatedVector<ActivityThreadRow> rows_{};
};

struct BlockchainSelectionRow {
    ot::UnallocatedCString name_{};
    bool enabled_{};
    bool testnet_{};
    ot::blockchain::Type type_{};
};

struct BlockchainSelectionData {
    ot::UnallocatedVector<BlockchainSelectionRow> rows_{};
};

struct BlockchainSubchainData {
    ot::UnallocatedCString name_;
    ot::blockchain::crypto::Subchain type_;
};

struct BlockchainSubaccountData {
    ot::UnallocatedCString name_;
    ot::UnallocatedCString id_;
    ot::UnallocatedVector<BlockchainSubchainData> rows_;
};

struct BlockchainSubaccountSourceData {
    ot::UnallocatedCString name_;
    ot::UnallocatedCString id_;
    ot::blockchain::crypto::SubaccountType type_;
    ot::UnallocatedVector<BlockchainSubaccountData> rows_;
};

struct BlockchainAccountStatusData {
    ot::UnallocatedCString owner_;
    ot::blockchain::Type chain_;
    ot::UnallocatedVector<BlockchainSubaccountSourceData> rows_;
};

struct ContactListRow {
    bool check_contact_id_{};
    ot::UnallocatedCString contact_id_index_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedCString section_{};
    ot::UnallocatedCString image_{};
};

struct ContactListData {
    ot::UnallocatedVector<ContactListRow> rows_{};
};

struct NymListRow {
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
};

struct NymListData {
    ot::UnallocatedVector<NymListRow> rows_{};
};

struct SeedTreeNym {
    std::size_t index_{};
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
};

struct SeedTreeItem {
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
    ot::crypto::SeedStyle type_{};
    ot::UnallocatedVector<SeedTreeNym> rows_;
};

struct SeedTreeData {
    ot::UnallocatedVector<SeedTreeItem> rows_;
};

auto activity_thread_send_message(
    const User& user,
    const User& contact) noexcept -> bool;
auto activity_thread_send_message(
    const User& user,
    const User& contact,
    const ot::UnallocatedCString& messasge) noexcept -> bool;

auto check_account_activity(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool;
auto check_account_activity_qt(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool;

auto check_account_list(
    const User& user,
    const AccountListData& expected) noexcept -> bool;
auto check_account_list_qt(
    const User& user,
    const AccountListData& expected) noexcept -> bool;

auto check_account_tree(
    const User& user,
    const AccountTreeData& expected) noexcept -> bool;
auto check_account_tree_qt(
    const User& user,
    const AccountTreeData& expected) noexcept -> bool;

auto check_activity_thread(
    const User& user,
    const ot::Identifier& contact,
    const ActivityThreadData& expected) noexcept -> bool;
auto check_activity_thread_qt(
    const User& user,
    const ot::Identifier& contact,
    const ActivityThreadData& expected) noexcept -> bool;

auto check_blockchain_selection(
    const ot::api::session::Client& api,
    const ot::ui::Blockchains type,
    const BlockchainSelectionData& expected) noexcept -> bool;
auto check_blockchain_selection_qt(
    const ot::api::session::Client& api,
    const ot::ui::Blockchains type,
    const BlockchainSelectionData& expected) noexcept -> bool;

auto check_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;
auto check_blockchain_account_status_qt(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;

auto check_contact_list(
    const User& user,
    const ContactListData& expected) noexcept -> bool;
auto check_contact_list_qt(
    const User& user,
    const ContactListData& expected) noexcept -> bool;

auto check_messagable_list(
    const User& user,
    const ContactListData& expected) noexcept -> bool;
auto check_messagable_list_qt(
    const User& user,
    const ContactListData& expected) noexcept -> bool;

auto check_nym_list(
    const ot::api::session::Client& api,
    const NymListData& expected) noexcept -> bool;
auto check_nym_list_qt(
    const ot::api::session::Client& api,
    const NymListData& expected) noexcept -> bool;

auto check_seed_tree(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool;
auto check_seed_tree_qt(
    const ot::api::session::Client& api,
    const SeedTreeData& expected) noexcept -> bool;

auto contact_list_add_contact(
    const User& user,
    const ot::UnallocatedCString& label,
    const ot::UnallocatedCString& paymentCode,
    const ot::UnallocatedCString& nymID) noexcept -> ot::UnallocatedCString;

auto init_account_activity(
    const User& user,
    const ot::Identifier& account,
    Counter& counter) noexcept -> void;
auto init_account_list(const User& user, Counter& counter) noexcept -> void;
auto init_account_tree(const User& user, Counter& counter) noexcept -> void;
auto init_activity_thread(
    const User& user,
    const User& contact,
    Counter& counter) noexcept -> void;
auto init_contact_list(const User& user, Counter& counter) noexcept -> void;
auto init_messagable_list(const User& user, Counter& counter) noexcept -> void;
auto init_nym_list(
    const ot::api::session::Client& api,
    Counter& counter) noexcept -> void;
auto init_seed_tree(
    const ot::api::session::Client& api,
    Counter& counter) noexcept -> void;

auto make_cb(Counter& counter, const ot::UnallocatedCString name) noexcept
    -> std::function<void()>;

auto print_account_tree(const User& user) noexcept -> ot::UnallocatedCString;
auto print_seed_tree(const ot::api::session::Client& api) noexcept
    -> ot::UnallocatedCString;

auto wait_for_counter(Counter& data, const bool hard = true) noexcept -> bool;
}  // namespace ottest
