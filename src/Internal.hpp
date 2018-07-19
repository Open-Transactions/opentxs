// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_FORWARD_INTERNAL_HPP
#define OPENTXS_FORWARD_INTERNAL_HPP

#include "opentxs/Forward.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace implementation
{
class Wallet;
}  // namespace implementation
}  // namespace client

namespace implementation
{
class Api;
class Crypto;
class Native;
class Storage;
class UI;
}  // namespace implementation

namespace network
{
namespace implementation
{
class Context;
class Dht;
class ZMQ;
}  // namespace implementation
}  // namespace network

namespace storage
{
class StorageInternal;
}  // namespace storage

class NativeInternal;
}  // namespace api

namespace crypto
{
namespace implementation
{
class OpenSSL;
}  // namespace implementation
}  // namespace crypto

namespace network
{
namespace zeromq
{
namespace implementation
{
class Context;
class Proxy;
}  // namespace implementation
}  // namespace zeromq
}  // namespace network

namespace storage
{
class Accounts;
class BlockchainTransactions;
class Contacts;
class Contexts;
class Credentials;
class Issuers;
class Mailbox;
class Nym;
class Nyms;
class PaymentWorkflows;
class PeerReplies;
class PeerRequests;
class Root;
class Seeds;
class Servers;
class Thread;
class Threads;
class Tree;
class Units;
}  // namespace storage

namespace ui
{
namespace implementation
{
class AccountActivity;
class AccountActivityParent;
class AccountSummary;
class AccountSummaryItemBlank;
class AccountSummaryParent;
class ActivitySummary;
class ActivitySummaryItemBlank;
class ActivitySummaryParent;
class ActivityThread;
class ActivityThreadItemBlank;
class ActivityThreadParent;
class BalanceItemBlank;
class Contact;
class ContactItemBlank;
class ContactListItemBlank;
class ContactListParent;
class ContactParent;
class ContactSection;
class ContactSectionBlank;
class ContactSectionParent;
class ContactSubsection;
class ContactSubsectionBlank;
class ContactSubsectionParent;
class IssuerItemBlank;
class IssuerItemParent;
class MessagableList;
class PayableList;
class PayableListItemBlank;
class Profile;
class ProfileItemBlank;
class ProfileParent;
class ProfileSection;
class ProfileSectionBlank;
class ProfileSectionParent;
class ProfileSubsection;
class ProfileSubsectionBlank;
class ProfileSubsectionParent;

// Account activity
using AccountActivityExternalInterface = opentxs::ui::AccountActivity;
using AccountActivityInternalInterface = AccountActivityParent;
using AccountActivityRowID = std::pair<OTIdentifier, proto::PaymentEventType>;
using AccountActivityRowInterface = opentxs::ui::BalanceItem;
using AccountActivityRowBlank = BalanceItemBlank;
/** WorkflowID, state */
using AccountActivitySortKey = std::chrono::system_clock::time_point;

// Account summary
using AccountSummaryExternalInterface = opentxs::ui::AccountSummary;
using AccountSummaryInternalInterface = AccountSummaryParent;
using AccountSummaryRowID = OTIdentifier;
using AccountSummaryRowInterface = opentxs::ui::IssuerItem;
using AccountSummaryRowBlank = IssuerItemBlank;
using AccountSummarySortKey = std::pair<bool, std::string>;

using IssuerItemExternalInterface = AccountSummaryRowInterface;
using IssuerItemInternalInterface = IssuerItemParent;
using IssuerItemRowID = std::pair<OTIdentifier, proto::ContactItemType>;
using IssuerItemRowInterface = opentxs::ui::AccountSummaryItem;
using IssuerItemRowBlank = AccountSummaryItemBlank;
using IssuerItemSortKey = std::string;

// Activity summary
using ActivitySummaryExternalInterface = opentxs::ui::ActivitySummary;
using ActivitySummaryInternalInterface = ActivitySummaryParent;
using ActivitySummaryRowID = OTIdentifier;
using ActivitySummaryRowInterface = opentxs::ui::ActivitySummaryItem;
using ActivitySummaryRowBlank = ActivitySummaryItemBlank;
using ActivitySummarySortKey =
    std::pair<std::chrono::system_clock::time_point, std::string>;

// Activity thread
using ActivityThreadExternalInterface = opentxs::ui::ActivityThread;
using ActivityThreadInternalInterface = ActivityThreadParent;
/** item id, box, accountID */
using ActivityThreadRowID = std::tuple<OTIdentifier, StorageBox, OTIdentifier>;
using ActivityThreadRowInterface = opentxs::ui::ActivityThreadItem;
using ActivityThreadRowBlank = ActivityThreadItemBlank;
/** timestamp, index */
using ActivityThreadSortKey =
    std::pair<std::chrono::system_clock::time_point, std::uint64_t>;

// Contact
using ContactExternalInterface = opentxs::ui::Contact;
using ContactInternalInterface = ContactParent;
using ContactRowID = proto::ContactSectionName;
using ContactRowInterface = opentxs::ui::ContactSection;
using ContactRowBlank = ContactSectionBlank;
using ContactSortKey = int;

using ContactSectionExternalInterface = ContactRowInterface;
using ContactSectionInternalInterface = ContactSectionParent;
using ContactSectionRowID =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ContactSectionRowInterface = opentxs::ui::ContactSubsection;
using ContactSectionRowBlank = ContactSubsectionBlank;
using ContactSectionSortKey = int;

using ContactSubsectionExternalInterface = ContactSectionRowInterface;
using ContactSubsectionInternalInterface = ContactSubsectionParent;
using ContactSubsectionRowID = OTIdentifier;
using ContactSubsectionRowInterface = opentxs::ui::ContactItem;
using ContactSubsectionRowBlank = ContactItemBlank;
using ContactSubsectionSortKey = int;

// Contact list
using ContactListExternalInterface = opentxs::ui::ContactList;
using ContactListInternalInterface = ContactListParent;
using ContactListRowID = OTIdentifier;
using ContactListRowInterface = opentxs::ui::ContactListItem;
using ContactListRowBlank = ContactListItemBlank;
using ContactListSortKey = std::string;

// Messagable list
using MessagableExternalInterface = opentxs::ui::MessagableList;
using MessagableInternalInterface = ContactListInternalInterface;
using MessagableListRowID = ContactListRowID;
using MessagableListRowInterface = ContactListRowInterface;
using MessagableListRowBlank = ContactListRowBlank;
using MessagableListSortKey = std::string;

// Payable list
using PayableExternalInterface = opentxs::ui::PayableList;
using PayableInternalInterface = ContactListInternalInterface;
using PayableListRowID = ContactListRowID;
using PayableListRowInterface = opentxs::ui::PayableListItem;
using PayableListRowBlank = PayableListItemBlank;
using PayableListSortKey = std::string;

// Profile
using ProfileExternalInterface = opentxs::ui::Profile;
using ProfileInternalInterface = ProfileParent;
using ProfileRowID = proto::ContactSectionName;
using ProfileRowInterface = opentxs::ui::ProfileSection;
using ProfileRowBlank = ProfileSectionBlank;
using ProfileSortKey = int;

using ProfileSectionExternalInterface = ProfileRowInterface;
using ProfileSectionInternalInterface = ProfileSectionParent;
using ProfileSectionRowID =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ProfileSectionRowInterface = opentxs::ui::ProfileSubsection;
using ProfileSectionRowBlank = ProfileSubsectionBlank;
using ProfileSectionSortKey = int;

using ProfileSubsectionExternalInterface = ProfileSectionRowInterface;
using ProfileSubsectionInternalInterface = ProfileSubsectionParent;
using ProfileSubsectionRowID = OTIdentifier;
using ProfileSubsectionRowInterface = opentxs::ui::ProfileItem;
using ProfileSubsectionRowBlank = ProfileItemBlank;
using ProfileSubsectionSortKey = int;
}  // namespace implementation
}  // namespace ui

class DhtConfig;
#if OT_CRYPTO_USING_LIBSECP256K1
class Libsecp256k1;
#endif
class Libsodium;
class LowLevelKeyGenerator;
#if OT_CRYPTO_USING_OPENSSL
class OpenSSL;
#endif
class StorageConfig;
class StorageMultiplex;
#if OT_CRYPTO_USING_TREZOR
class Trezor;
#endif
}  // namespace opentxs

extern template class std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

#include "Factory.hpp"

#endif  // OPENTXS_FORWARD_INTERNAL_HPP
