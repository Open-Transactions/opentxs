// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/session/ui/Imp-base.hpp"  // IWYU pragma: associated

#include <memory>
#include <tuple>

#include "internal/core/Core.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/AccountType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::api::session::imp
{
UI::Imp::Imp(
    const api::session::Client& api,
    const api::crypto::Blockchain& blockchain,
    const Flag& running) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , running_(running)
    , accounts_()
    , account_lists_()
    , account_summaries_()
    , account_trees_()
    , activity_summaries_()
    , activity_threads_()
    , blockchain_account_status_()
    , blockchain_selection_()
    , blockchain_statistics_()
    , contact_lists_()
    , contacts_()
    , messagable_lists_()
    , nym_list_()
    , payable_lists_()
    , profiles_()
    , seed_list_()
    , seed_tree_()
    , unit_lists_()
    , update_manager_(api_)
{
    // WARNING: do not access api_.Wallet() during construction
}

auto UI::Imp::account_activity(
    const Lock& lock,
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    const SimpleCallback& cb) const noexcept -> AccountActivityMap::mapped_type&
{
    auto key = AccountActivityKey{nymID, accountID};
    auto it = accounts_.find(key);

    if (accounts_.end() == it) {
        it = accounts_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple((
                         is_blockchain_account(accountID)
                             ? opentxs::factory::BlockchainAccountActivityModel
                             : opentxs::factory::CustodialAccountActivityModel)(
                         api_, nymID, accountID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::AccountActivity(
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::AccountActivity&
{
    auto lock = Lock{lock_};

    return *account_activity(lock, nymID, accountID, cb);
}

auto UI::Imp::account_list(
    const Lock& lock,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) const noexcept -> AccountListMap::mapped_type&
{
    auto key = AccountListKey{nymID};
    auto it = account_lists_.find(key);

    if (account_lists_.end() == it) {
        it = account_lists_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::AccountListModel(api_, nymID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::AccountList(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::AccountList&
{
    auto lock = Lock{lock_};

    return *account_list(lock, nymID, cb);
}

auto UI::Imp::account_summary(
    const Lock& lock,
    const identifier::Nym& nymID,
    const UnitType currency,
    const SimpleCallback& cb) const noexcept -> AccountSummaryMap::mapped_type&
{
    auto key = AccountSummaryKey{nymID, currency};
    auto it = account_summaries_.find(key);

    if (account_summaries_.end() == it) {
        it =
            account_summaries_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(opentxs::factory::AccountSummaryModel(
                        api_, nymID, currency, cb)))
                .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::AccountSummary(
    const identifier::Nym& nymID,
    const UnitType currency,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::AccountSummary&
{
    auto lock = Lock{lock_};

    return *account_summary(lock, nymID, currency, cb);
}

auto UI::Imp::ActivateUICallback(
    const identifier::Generic& widget) const noexcept -> void
{
    update_manager_.ActivateUICallback(widget);
}

auto UI::Imp::activity_summary(
    const Lock& lock,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) const noexcept -> ActivitySummaryMap::mapped_type&
{
    auto key = ActivitySummaryKey{nymID};
    auto it = activity_summaries_.find(key);

    if (activity_summaries_.end() == it) {
        it = activity_summaries_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::ActivitySummaryModel(
                             api_, running_, nymID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::AccountTree(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::AccountTree&
{
    auto lock = Lock{lock_};

    return *account_tree(lock, nymID, cb);
}

auto UI::Imp::account_tree(
    const Lock& lock,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) const noexcept -> AccountTreeMap::mapped_type&
{
    auto key = AccountTreeKey{nymID};
    auto it = account_trees_.find(key);

    if (account_trees_.end() == it) {
        it = account_trees_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::AccountTreeModel(api_, nymID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::ActivitySummary(
    const identifier::Nym& nymID,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::ActivitySummary&
{
    auto lock = Lock{lock_};

    return *activity_summary(lock, nymID, cb);
}

auto UI::Imp::activity_thread(
    const Lock& lock,
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    const SimpleCallback& cb) const noexcept -> ActivityThreadMap::mapped_type&
{
    auto key = ActivityThreadKey{nymID, threadID};
    auto it = activity_threads_.find(key);

    if (activity_threads_.end() == it) {
        it =
            activity_threads_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(opentxs::factory::ActivityThreadModel(
                        api_, nymID, threadID, cb)))
                .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::ActivityThread(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::ActivityThread&
{
    auto lock = Lock{lock_};

    return *activity_thread(lock, nymID, threadID, cb);
}

auto UI::Imp::blockchain_account_status(
    const Lock& lock,
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain,
    const SimpleCallback& cb) const noexcept
    -> BlockchainAccountStatusMap::mapped_type&
{
    auto key = BlockchainAccountStatusKey{nymID, chain};
    auto it = blockchain_account_status_.find(key);

    if (blockchain_account_status_.end() == it) {
        it = blockchain_account_status_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::BlockchainAccountStatusModel(
                             api_, nymID, chain, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::BlockchainAccountStatus(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::BlockchainAccountStatus&
{
    auto lock = Lock{lock_};

    return *blockchain_account_status(lock, nymID, chain, cb);
}

auto UI::Imp::BlockchainIssuerID(const opentxs::blockchain::Type chain)
    const noexcept -> const identifier::Nym&
{
    return opentxs::blockchain::IssuerID(api_, chain);
}

auto UI::Imp::BlockchainNotaryID(const opentxs::blockchain::Type chain)
    const noexcept -> const identifier::Notary&
{
    return opentxs::blockchain::NotaryID(api_, chain);
}

auto UI::Imp::blockchain_selection(
    const Lock& lock,
    const opentxs::ui::Blockchains key,
    const SimpleCallback cb) const noexcept
    -> BlockchainSelectionMap::mapped_type&
{
    auto it = blockchain_selection_.find(key);

    if (blockchain_selection_.end() == it) {
        it = blockchain_selection_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(key),
                     std::forward_as_tuple(
                         opentxs::factory::BlockchainSelectionModel(
                             api_, key, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::BlockchainSelection(
    const opentxs::ui::Blockchains type,
    const SimpleCallback updateCB) const noexcept
    -> const opentxs::ui::BlockchainSelection&
{
    auto lock = Lock{lock_};

    return *blockchain_selection(lock, type, updateCB);
}

auto UI::Imp::blockchain_statistics(const Lock& lock, const SimpleCallback cb)
    const noexcept -> BlockchainStatisticsPointer&
{
    if (false == bool(blockchain_statistics_)) {
        blockchain_statistics_ =
            opentxs::factory::BlockchainStatisticsModel(api_, cb);
    }

    OT_ASSERT(blockchain_statistics_);

    return blockchain_statistics_;
}

auto UI::Imp::BlockchainStatistics(const SimpleCallback updateCB) const noexcept
    -> const opentxs::ui::BlockchainStatistics&
{
    auto lock = Lock{lock_};

    return *blockchain_statistics(lock, updateCB);
}

auto UI::Imp::BlockchainUnitID(const opentxs::blockchain::Type chain)
    const noexcept -> const identifier::UnitDefinition&
{
    return opentxs::blockchain::UnitID(api_, chain);
}

auto UI::Imp::ClearUICallbacks(const identifier::Generic& widget) const noexcept
    -> void
{
    update_manager_.ClearUICallbacks(widget);
}

auto UI::Imp::contact(
    const Lock& lock,
    const identifier::Generic& contactID,
    const SimpleCallback& cb) const noexcept -> ContactMap::mapped_type&
{
    auto key = ContactKey{contactID};
    auto it = contacts_.find(key);

    if (contacts_.end() == it) {
        it = contacts_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::ContactModel(api_, contactID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::Contact(
    const identifier::Generic& contactID,
    const SimpleCallback cb) const noexcept -> const opentxs::ui::Contact&
{
    auto lock = Lock{lock_};

    return *contact(lock, contactID, cb);
}

auto UI::Imp::contact_list(
    const Lock& lock,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) const noexcept -> ContactListMap::mapped_type&
{
    auto key = ContactListKey{nymID};
    auto it = contact_lists_.find(key);

    if (contact_lists_.end() == it) {
        it = contact_lists_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::ContactListModel(api_, nymID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::ContactList(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::ContactList&
{
    auto lock = Lock{lock_};

    return *contact_list(lock, nymID, cb);
}

auto UI::Imp::is_blockchain_account(
    const identifier::Account& id) const noexcept -> bool
{
    blockchain_.LookupAccount(id);

    return AccountType::Blockchain == id.AccountType();
}

auto UI::Imp::messagable_list(
    const Lock& lock,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) const noexcept -> MessagableListMap::mapped_type&
{
    auto key = MessagableListKey{nymID};
    auto it = messagable_lists_.find(key);

    if (messagable_lists_.end() == it) {
        it =
            messagable_lists_
                .emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(std::move(key)),
                    std::forward_as_tuple(
                        opentxs::factory::MessagableListModel(api_, nymID, cb)))
                .first;
    }

    return it->second;
}

auto UI::Imp::MessagableList(
    const identifier::Nym& nymID,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::MessagableList&
{
    auto lock = Lock{lock_};

    return *messagable_list(lock, nymID, cb);
}

auto UI::Imp::NymList(const SimpleCallback cb) const noexcept
    -> const opentxs::ui::NymList&
{
    auto lock = Lock{lock_};

    return nym_list(lock, cb);
}

auto UI::Imp::nym_list(const Lock& lock, const SimpleCallback& cb)
    const noexcept -> opentxs::ui::internal::NymList&
{
    if (!nym_list_) {
        nym_list_ = opentxs::factory::NymListModel(api_, cb);

        OT_ASSERT(nym_list_);
    }

    return *nym_list_;
}

auto UI::Imp::payable_list(
    const Lock& lock,
    const identifier::Nym& nymID,
    const UnitType currency,
    const SimpleCallback& cb) const noexcept -> PayableListMap::mapped_type&
{
    auto key = PayableListKey{nymID, currency};
    auto it = payable_lists_.find(key);

    if (payable_lists_.end() == it) {
        it = payable_lists_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(opentxs::factory::PayableListModel(
                         api_, nymID, currency, cb)))
                 .first;
    }

    return it->second;
}

auto UI::Imp::PayableList(
    const identifier::Nym& nymID,
    UnitType currency,
    const SimpleCallback cb) const noexcept -> const opentxs::ui::PayableList&
{
    auto lock = Lock{lock_};

    return *payable_list(lock, nymID, currency, cb);
}

auto UI::Imp::profile(
    const Lock& lock,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) const noexcept -> ProfileMap::mapped_type&
{
    auto key = ProfileKey{nymID};
    auto it = profiles_.find(key);

    if (profiles_.end() == it) {
        it = profiles_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::ProfileModel(api_, nymID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::Profile(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::Profile&
{
    auto lock = Lock{lock_};

    return *profile(lock, nymID, cb);
}

auto UI::Imp::RegisterUICallback(
    const identifier::Generic& widget,
    const SimpleCallback& cb) const noexcept -> void
{
    update_manager_.RegisterUICallback(widget, cb);
}

auto UI::Imp::SeedList(const SimpleCallback cb) const noexcept
    -> const opentxs::ui::SeedList&
{
    auto lock = Lock{lock_};

    return seed_list(lock, cb);
}

auto UI::Imp::seed_list(const Lock& lock, const SimpleCallback& cb)
    const noexcept -> opentxs::ui::internal::SeedList&
{
    if (!seed_list_) {
        seed_list_ = opentxs::factory::SeedListModel(api_, cb);

        OT_ASSERT(seed_list_);
    }

    return *seed_list_;
}

auto UI::Imp::SeedTree(const SimpleCallback cb) const noexcept
    -> const opentxs::ui::SeedTree&
{
    auto lock = Lock{lock_};

    return seed_tree(lock, cb);
}

auto UI::Imp::seed_tree(const Lock& lock, const SimpleCallback& cb)
    const noexcept -> opentxs::ui::internal::SeedTree&
{
    if (!seed_tree_) {
        seed_tree_ = opentxs::factory::SeedTreeModel(api_, cb);

        OT_ASSERT(seed_tree_);
    }

    return *seed_tree_;
}

auto UI::Imp::Shutdown() noexcept -> void
{
    ShutdownCallbacks();
    ShutdownModels();
}

auto UI::Imp::ShutdownCallbacks() noexcept -> void
{
    const auto clearCallbacks = [](auto& map) {
        for (auto& [key, widget] : map) {
            if (widget) { widget->ClearCallbacks(); }
        }
    };
    auto lock = Lock{lock_};

    if (blockchain_statistics_) { blockchain_statistics_->ClearCallbacks(); }

    clearCallbacks(unit_lists_);
    clearCallbacks(profiles_);

    if (seed_tree_) { seed_tree_->ClearCallbacks(); }
    if (seed_list_) { seed_list_->ClearCallbacks(); }

    clearCallbacks(payable_lists_);

    if (nym_list_) { nym_list_->ClearCallbacks(); }

    clearCallbacks(messagable_lists_);
    clearCallbacks(contacts_);
    clearCallbacks(contact_lists_);
    clearCallbacks(blockchain_selection_);
    clearCallbacks(blockchain_account_status_);
    clearCallbacks(activity_threads_);
    clearCallbacks(activity_summaries_);
    clearCallbacks(account_trees_);
    clearCallbacks(account_summaries_);
    clearCallbacks(account_lists_);
    clearCallbacks(accounts_);
}

auto UI::Imp::ShutdownModels() noexcept -> void
{
    unit_lists_.clear();
    seed_tree_.reset();
    seed_list_.reset();
    profiles_.clear();
    payable_lists_.clear();
    nym_list_.reset();
    messagable_lists_.clear();
    contacts_.clear();
    contact_lists_.clear();
    blockchain_statistics_.reset();
    blockchain_selection_.clear();
    blockchain_account_status_.clear();
    activity_threads_.clear();
    activity_summaries_.clear();
    account_trees_.clear();
    account_summaries_.clear();
    account_lists_.clear();
    accounts_.clear();
}

auto UI::Imp::unit_list(
    const Lock& lock,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) const noexcept -> UnitListMap::mapped_type&
{
    auto key = UnitListKey{nymID};
    auto it = unit_lists_.find(key);

    if (unit_lists_.end() == it) {
        it = unit_lists_
                 .emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(std::move(key)),
                     std::forward_as_tuple(
                         opentxs::factory::UnitListModel(api_, nymID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second;
}

auto UI::Imp::UnitList(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::UnitList&
{
    auto lock = Lock{lock_};

    return *unit_list(lock, nymID, cb);
}

UI::Imp::~Imp()
{
    ShutdownCallbacks();
    Imp::ShutdownModels();
}
}  // namespace opentxs::api::session::imp
