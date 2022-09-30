// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>

#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <class ChildType, class MutexType = std::mutex>
class Editor
{
public:
    using OptionalCallback = std::function<void(const ChildType&)>;
    using Lock = std::unique_lock<MutexType>;
    using LockedSave = std::function<void(ChildType*, Lock&)>;
    using UnlockedSave = std::function<void(ChildType*)>;

    auto get() -> ChildType& { return *object_; }

    Editor(
        MutexType& objectMutex,
        ChildType* object,
        LockedSave save,
        OptionalCallback callback = nullptr)
        : object_(object)
        , locked_(true)
        , object_lock_(new Lock(objectMutex))
        , locked_save_callback_(new LockedSave(save))
        , unlocked_save_callback_(nullptr)
        , optional_callback_(callback)
    {
        assert(nullptr != object);
        assert(object_lock_);
        assert(locked_save_callback_);
    }
    Editor(
        ChildType* object,
        UnlockedSave save,
        OptionalCallback callback = nullptr)
        : object_(object)
        , locked_(false)
        , object_lock_(nullptr)
        , locked_save_callback_(nullptr)
        , unlocked_save_callback_(new UnlockedSave(save))
        , optional_callback_(callback)
    {
        assert(nullptr != object);
        assert(unlocked_save_callback_);
    }
    Editor() = delete;
    Editor(const Editor&) = delete;
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
    auto operator=(const Editor&) -> Editor& = delete;
    auto operator=(Editor&&) -> Editor& = delete;

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
    ChildType* object_{nullptr};
    bool locked_{true};
    std::unique_ptr<Lock> object_lock_{nullptr};
    std::unique_ptr<LockedSave> locked_save_callback_{nullptr};
    std::unique_ptr<UnlockedSave> unlocked_save_callback_{nullptr};
    const OptionalCallback optional_callback_{nullptr};
};  // class Editor
}  // namespace opentxs
