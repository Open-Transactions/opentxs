// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_EDITOR_HPP
#define OPENTXS_API_EDITOR_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Log.hpp"

#include <functional>
#include <memory>
#include <mutex>

namespace opentxs
{

template <class C>
class Editor
{
public:
    using OptionalCallback = std::function<void(const C&)>;
    using Lock = std::unique_lock<std::mutex>;
    using LockedSave = std::function<void(C*, Lock&)>;
    using UnlockedSave = std::function<void(C*)>;

    Editor(
        std::mutex& objectMutex,
        C* object,
        LockedSave save,
        OptionalCallback callback = nullptr)
        : object_(object)
        , locked_(true)
        , object_lock_(new Lock(objectMutex))
        , locked_save_callback_(new LockedSave(save))
        , unlocked_save_callback_(nullptr)
        , optional_callback_(callback)
    {
        OT_ASSERT(nullptr != object);
        OT_ASSERT(object_lock_);
        OT_ASSERT(locked_save_callback_);
    }

    Editor(C* object, UnlockedSave save, OptionalCallback callback = nullptr)
        : object_(object)
        , locked_(false)
        , object_lock_(nullptr)
        , locked_save_callback_(nullptr)
        , unlocked_save_callback_(new UnlockedSave(save))
        , optional_callback_(callback)
    {
        OT_ASSERT(nullptr != object);
        OT_ASSERT(unlocked_save_callback_);
    }

    Editor(Editor&& rhs)
        : object_(rhs.object_)
        , locked_(rhs.locked_)
        , object_lock_(rhs.object_lock_.release())
        , locked_save_callback_(rhs.locked_save_callback_.release())
        , unlocked_save_callback_(rhs.unlocked_save_callback_.release())
        , optional_callback_(std::move(rhs.optional_callback_))
    {
        rhs.object_ = nullptr;
    }

    C& It() { return *object_; }

    ~Editor()
    {
        if (nullptr == object_) {
            // A move constructor must have been called on this object

            return;
        }

        if (locked_) {
            auto& callback = *locked_save_callback_;
            callback(object_, *object_lock_);
            object_lock_->unlock();
        } else {
            auto& callback = *unlocked_save_callback_;
            callback(object_);
        }

        if (optional_callback_) { optional_callback_(*object_); }
    }

private:
    C* object_{nullptr};
    bool locked_{true};
    std::unique_ptr<Lock> object_lock_{nullptr};
    std::unique_ptr<LockedSave> locked_save_callback_{nullptr};
    std::unique_ptr<UnlockedSave> unlocked_save_callback_{nullptr};
    const OptionalCallback optional_callback_{nullptr};

    Editor() = delete;
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
};  // class Editor
}  // namespace opentxs

#endif
