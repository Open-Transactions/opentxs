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

#ifndef OPENTXS_API_EDITOR_HPP
#define OPENTXS_API_EDITOR_HPP

#include "opentxs/core/Log.hpp"

#include <functional>
#include <memory>
#include <mutex>

namespace opentxs
{

template<class C>
class Editor
{
private:
    typedef std::unique_lock<std::mutex> Lock;
    typedef std::function<void(C*, Lock&)> LockedSave;
    typedef std::function<void(C*)> UnlockedSave;

    C* object_;
    bool locked_{true};
    std::unique_ptr<Lock> object_lock_;
    std::unique_ptr<LockedSave> locked_save_callback_;
    std::unique_ptr<UnlockedSave> unlocked_save_callback_;

    Editor() = delete;
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

public:
    Editor(std::mutex& objectMutex, C* object, LockedSave save)
        : object_(object)
        , locked_(true)
    {
        OT_ASSERT(nullptr != object);

        object_lock_.reset(new Lock(objectMutex));

        OT_ASSERT(object_lock_);

        locked_save_callback_.reset(new LockedSave(save));

        OT_ASSERT(locked_save_callback_);
    }

    Editor(C* object, UnlockedSave save)
        : object_(object)
        , locked_(false)
    {
        OT_ASSERT(nullptr != object);

        unlocked_save_callback_.reset(new UnlockedSave(save));

        OT_ASSERT(unlocked_save_callback_);
    }

    Editor(Editor&& rhs)
        : object_(rhs.object_)
        , locked_(rhs.locked_)
        , object_lock_(rhs.object_lock_.release())
        , locked_save_callback_(rhs.locked_save_callback_.release())
        , unlocked_save_callback_(rhs.unlocked_save_callback_.release())
    {
        rhs.object_ = nullptr;
    }

    C& It() { return *object_; }

    ~Editor()
    {
        if (locked_) {
            auto& callback = *locked_save_callback_;
            callback(object_, *object_lock_);

            object_lock_->unlock();
        } else {
            auto& callback = *unlocked_save_callback_;
            callback(object_);
        }
    }

}; // class Editor
} // namespace opentxs

#endif // OPENTXS_API_EDITOR_HPP
