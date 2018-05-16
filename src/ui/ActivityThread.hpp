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

#ifndef OPENTXS_UI_ACTIVITYTHREAD_IMPLEMENTATION_HPP
#define OPENTXS_UI_ACTIVITYTHREAD_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace std
{
using STORAGEID = std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

template <>
struct less<STORAGEID> {
    bool operator()(const STORAGEID& lhs, const STORAGEID& rhs) const
    {
        /* TODO: these lines will cause a segfault in the clang-5 ast parser.
                const auto & [ lID, lBox, lAccount ] = lhs;
                const auto & [ rID, rBox, rAccount ] = rhs;
        */
        const auto& lID = std::get<0>(lhs);
        const auto& lBox = std::get<1>(lhs);
        const auto& lAccount = std::get<2>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rBox = std::get<1>(rhs);
        const auto& rAccount = std::get<2>(rhs);

        if (lID->str() < rID->str()) {

            return true;
        }

        if (rID->str() < lID->str()) {

            return false;
        }

        if (lBox < rBox) {

            return true;
        }

        if (rBox < lBox) {

            return false;
        }

        if (lAccount->str() < rAccount->str()) {

            return true;
        }

        return false;
    }
};
}  // namespace std

namespace opentxs::ui::implementation
{
using ActivityThreadPimpl = std::unique_ptr<opentxs::ui::ActivityThreadItem>;
/** timestamp, index */
using ActivityThreadSortKey =
    std::pair<std::chrono::system_clock::time_point, std::uint64_t>;
using ActivityThreadInner = std::map<ActivityThreadID, ActivityThreadPimpl>;
using ActivityThreadOuter =
    std::map<ActivityThreadSortKey, ActivityThreadInner>;
using ActivityThreadReverse = std::map<ActivityThreadID, ActivityThreadSortKey>;
using ActivityThreadType = List<
    opentxs::ui::ActivityThread,
    ActivityThreadParent,
    opentxs::ui::ActivityThreadItem,
    ActivityThreadID,
    ActivityThreadPimpl,
    ActivityThreadInner,
    ActivityThreadSortKey,
    ActivityThreadOuter,
    ActivityThreadOuter::const_iterator,
    ActivityThreadReverse>;

class ActivityThread : virtual public ActivityThreadType
{
public:
    std::string DisplayName() const override;
    std::string GetDraft() const override;
    std::string Participants() const override;
    std::string PaymentCode(
        const proto::ContactItemType currency) const override;
    bool same(const ActivityThreadID& lhs, const ActivityThreadID& rhs)
        const override;
    bool SendDraft() const override;
    bool SetDraft(const std::string& draft) const override;
    std::string ThreadID() const override;

    ~ActivityThread();

private:
    friend Factory;

    const api::Activity& activity_;
    const api::client::Sync& sync_;
    const Identifier& threadID_;
    std::set<OTIdentifier> participants_;
    OTZMQListenCallback activity_subscriber_callback_;
    OTZMQSubscribeSocket activity_subscriber_;
    mutable std::mutex contact_lock_;
    mutable std::shared_mutex draft_lock_;
    mutable std::string draft_{""};
    mutable std::set<ActivityThreadID> draft_tasks_;
    std::shared_ptr<const opentxs::Contact> contact_;
    std::unique_ptr<std::thread> contact_thread_{nullptr};

    ActivityThreadID blank_id() const override;
    bool check_draft(const ActivityThreadID& id) const;
    void check_drafts() const;
    std::string comma(const std::set<std::string>& list) const;
    void construct_item(
        const ActivityThreadID& id,
        const ActivityThreadSortKey& index,
        void* custom = nullptr) const override;
    ActivityThreadOuter::const_iterator outer_first() const override;
    ActivityThreadOuter::const_iterator outer_end() const override;

    void init_contact();
    void load_thread(const proto::StorageThread& thread);
    void new_thread();
    ActivityThreadID process_item(const proto::StorageThreadItem& item);
    void process_thread(const network::zeromq::Message& message);
    void startup();

    ActivityThread(
        const network::zeromq::Context& zmq,
        const api::client::Sync& sync,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const Identifier& threadID);

    ActivityThread() = delete;
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    ActivityThread& operator=(const ActivityThread&) = delete;
    ActivityThread& operator=(ActivityThread&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITYTHREAD_IMPLEMENTATION_HPP
