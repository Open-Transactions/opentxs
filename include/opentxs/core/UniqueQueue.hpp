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

#ifndef OPENTXS_CORE_UNIQUEQUEUE_HPP
#define OPENTXS_CORE_UNIQUEQUEUE_HPP

#include "opentxs/Version.hpp"

#include <deque>
#include <mutex>
#include <set>

namespace opentxs
{
template <class T>
class UniqueQueue
{
public:
    void CancelByValue(const T& in) const
    {
        Lock lock(lock_);

        if (0 == set_.count(in)) {

            return;
        }

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            /* TODO: these lines will cause a segfault in the clang-5 ast
             *  parser.
            const auto & [ key, value ] = *i;
            [[maybe_unused]] const auto& notUsed = key;
            */
            const auto& value = std::get<1>(*i);

            if (value == in) {
                set_.erase(value);
                queue_.erase(i);
                break;
            }
        }

        OT_ASSERT(set_.size() == queue_.size())
    }

    void CancelByKey(const Identifier& in) const
    {
        Lock lock(lock_);

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            /* TODO: this line will cause a segfault in the clang-5 ast parser.
            const auto & [ key, value ] = *i;
            */
            const auto& key = std::get<0>(*i);
            const auto& value = std::get<1>(*i);

            if (key == in) {
                set_.erase(value);
                queue_.erase(i);
                break;
            }
        }

        OT_ASSERT(set_.size() == queue_.size())
    }

    std::map<T, OTIdentifier> Copy() const
    {
        std::map<T, OTIdentifier> output{};
        Lock lock(lock_);

        /* TODO: this line will cause a segfault in the clang-5 ast parser.
        for (const auto & [ key, value ] : queue_) {
        */
        for (const auto& it : queue_) {
            const auto& key = it.first;
            const auto& value = it.second;
            output.emplace(value, Identifier::Factory(key->str()));
        }

        return output;
    }

    bool Push(const Identifier& key, const T& in) const
    {
        OT_ASSERT(false == key->empty())

        Lock lock(lock_);

        if (0 == set_.count(in)) {
            queue_.push_front({key, in});
            set_.emplace(in);

            OT_ASSERT(set_.size() == queue_.size())

            return true;
        }

        return false;
    }

    bool Pop(OTIdentifier& key, T& out) const
    {
        Lock lock(lock_);

        if (0 == queue_.size()) {

            return false;
        }

        /* TODO: this line will cause a segfault in the clang-5 ast parser.
        const auto & [ outKey, outValue ] = queue_.back();
        */
        const auto& outKey = queue_.back().first;
        const auto& outValue = queue_.back().second;
        set_.erase(outValue);
        out = outValue;
        key->SetString(String(outKey));
        queue_.pop_back();

        OT_ASSERT(set_.size() == queue_.size())

        return true;
    }

private:
    mutable std::mutex lock_;
    mutable std::deque<std::pair<OTIdentifier, T>> queue_;
    mutable std::set<T> set_;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_UNIQUEQUEUE_HPP
