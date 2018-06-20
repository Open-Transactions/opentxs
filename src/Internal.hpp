/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

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
}  // namespace api

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
class Bip47Context;
class Bip47Contexts;
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
class ActivitySummary;
class ActivitySummaryParent;
class ActivityThread;
class ActivityThreadParent;
class Contact;
class ContactListParent;
class ContactParent;
class ContactSection;
class ContactSectionParent;
class ContactSubsection;
class ContactSubsectionParent;
class MessagableList;
class PayableList;
class Profile;
class ProfileParent;
class ProfileSection;
class ProfileSectionParent;
class ProfileSubsection;
class ProfileSubsectionParent;

using AccountActivityPimpl = opentxs::ui::BalanceItem;
/** WorkflowID, state */
using AccountActivityID = std::pair<OTIdentifier, proto::PaymentEventType>;
using AccountActivitySortKey = std::chrono::system_clock::time_point;
using AccountActivityInner =
    std::map<AccountActivityID, std::shared_ptr<AccountActivityPimpl>>;
using AccountActivityOuter =
    std::map<AccountActivitySortKey, AccountActivityInner>;
using ActivitySummaryPimpl = opentxs::ui::ActivitySummaryItem;
using ActivitySummaryID = OTIdentifier;
using ActivitySummarySortKey =
    std::pair<std::chrono::system_clock::time_point, std::string>;
using ActivitySummaryInner =
    std::map<ActivitySummaryID, std::shared_ptr<ActivitySummaryPimpl>>;
using ActivitySummaryOuter =
    std::map<ActivitySummarySortKey, ActivitySummaryInner>;
using ActivityThreadPimpl = opentxs::ui::ActivityThreadItem;
/** item id, box, accountID */
using ActivityThreadID = std::tuple<OTIdentifier, StorageBox, OTIdentifier>;
/** timestamp, index */
using ActivityThreadSortKey =
    std::pair<std::chrono::system_clock::time_point, std::uint64_t>;
using ActivityThreadInner =
    std::map<ActivityThreadID, std::shared_ptr<ActivityThreadPimpl>>;
using ActivityThreadOuter =
    std::map<ActivityThreadSortKey, ActivityThreadInner>;
using ContactPimpl = opentxs::ui::ContactSection;
using ContactIDType = proto::ContactSectionName;
using ContactSortKey = int;
using ContactInner = std::map<ContactIDType, std::shared_ptr<ContactPimpl>>;
using ContactOuter = std::map<ContactSortKey, ContactInner>;
using ContactListPimpl = opentxs::ui::ContactListItem;
using ContactListID = OTIdentifier;
using ContactListSortKey = std::string;
using ContactListInner =
    std::map<ContactListID, std::shared_ptr<ContactListPimpl>>;
using ContactListOuter = std::map<ContactListSortKey, ContactListInner>;
using ContactSectionPimpl = opentxs::ui::ContactSubsection;
using ContactSectionIDType =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ContactSectionSortKey = int;
using ContactSectionInner =
    std::map<ContactSectionIDType, std::shared_ptr<ContactSectionPimpl>>;
using ContactSectionOuter =
    std::map<ContactSectionSortKey, ContactSectionInner>;
using ContactSubsectionPimpl = opentxs::ui::ContactItem;
using ContactSubsectionIDType = OTIdentifier;
using ContactSubsectionSortKey = int;
using ContactSubsectionInner =
    std::map<ContactSubsectionIDType, std::shared_ptr<ContactSubsectionPimpl>>;
using ContactSubsectionOuter =
    std::map<ContactSubsectionSortKey, ContactSubsectionInner>;
using MessagableListPimpl = opentxs::ui::ContactListItem;
using MessagableListID = OTIdentifier;
using MessagableListSortKey = std::string;
using MessagableListInner =
    std::map<MessagableListID, std::shared_ptr<MessagableListPimpl>>;
using MessagableListOuter =
    std::map<MessagableListSortKey, MessagableListInner>;
using PayableListPimpl = opentxs::ui::PayableListItem;
using PayableListID = OTIdentifier;
using PayableListSortKey = std::string;
using PayableListInner =
    std::map<PayableListID, std::shared_ptr<PayableListPimpl>>;
using PayableListOuter = std::map<PayableListSortKey, PayableListInner>;
using ProfilePimpl = opentxs::ui::ProfileSection;
using ProfileIDType = proto::ContactSectionName;
using ProfileSortKey = int;
using ProfileInner = std::map<ProfileIDType, std::shared_ptr<ProfilePimpl>>;
using ProfileOuter = std::map<ProfileSortKey, ProfileInner>;
using ProfileSectionPimpl = opentxs::ui::ProfileSubsection;
using ProfileSectionIDType =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ProfileSectionSortKey = int;
using ProfileSectionInner =
    std::map<ProfileSectionIDType, std::shared_ptr<ProfileSectionPimpl>>;
using ProfileSectionOuter =
    std::map<ProfileSectionSortKey, ProfileSectionInner>;
using ProfileSubsectionPimpl = opentxs::ui::ProfileItem;
using ProfileSubsectionIDType = OTIdentifier;
using ProfileSubsectionSortKey = int;
using ProfileSubsectionInner =
    std::map<ProfileSubsectionIDType, std::shared_ptr<ProfileSubsectionPimpl>>;
using ProfileSubsectionOuter =
    std::map<ProfileSubsectionSortKey, ProfileSubsectionInner>;
}  // namespace implementation
}  // namespace ui

class DhtConfig;
#if OT_CRYPTO_USING_LIBSECP256K1
class Libsecp256k1;
#endif
class Libsodium;
#if OT_CRYPTO_USING_OPENSSL
class OpenSSL;
#endif
class StorageConfig;
class StorageMultiplex;
#if OT_CRYPTO_USING_TREZOR
class TrezorCrypto;
#endif
}  // namespace opentxs

extern template class std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

#include "Factory.hpp"

#endif  // OPENTXS_FORWARD_INTERNAL_HPP
