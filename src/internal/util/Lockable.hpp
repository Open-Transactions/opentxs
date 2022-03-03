// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <shared_mutex>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <typename L, typename M>
auto CheckLock(const L& lock, const M& mutex) noexcept -> bool
{
    if (lock.mutex() != &mutex) {
        LogError()(": Lock is on incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(": Lock is unlocked.").Flush();

        return false;
    }

    return true;
}

class Lockable
{
public:
    Lockable() noexcept
        : lock_()
        , shared_lock_()
    {
    }

    virtual ~Lockable() = default;

protected:
    mutable std::mutex lock_;
    mutable std::shared_mutex shared_lock_;

    auto verify_lock(const Lock& lock) const noexcept -> bool
    {
        return CheckLock(lock, lock_);
    }

    auto verify_lock(const sLock& lock) const noexcept -> bool
    {
        return CheckLock(lock, shared_lock_);
    }

    auto verify_lock(const eLock& lock) const noexcept -> bool
    {
        return CheckLock(lock, shared_lock_);
    }

private:
    Lockable(const Lockable&) = delete;
    Lockable(Lockable&&) = delete;
    auto operator=(const Lockable&) -> Lockable& = delete;
    auto operator=(Lockable&&) -> Lockable& = delete;
};
}  // namespace opentxs
