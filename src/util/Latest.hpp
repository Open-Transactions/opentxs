// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "internal/util/Mutex.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
/// So maybe you have a member variable that you want to return by const
/// reference, but also you need to be thread safe and, rarely, this member
/// variable might be changed out for new object. This class keeps the old
/// versions around to maintain the validity of any references which might exist
/// to the old version.
///
/// Only use if you don't care about consuming memory for out-of-date copies.
/// Really it's best if in practice updates will probably never actually happen
/// but you can't prove it so you need something to account for the possibility.
template <typename Store, typename Return, typename Compare>
class LatestVersion
{
public:
    operator const Return&() const noexcept { return get(); }
    auto get() const noexcept -> const Return&
    {
        return const_cast<LatestVersion&>(*this).get();
    }

    operator Return&() noexcept { return get(); }
    auto get() noexcept -> Return&
    {
        auto lock = Lock{lock_};

        return versions_.back();
    }
    auto revise(const Return& rhs) noexcept -> bool
    {
        auto lock = Lock{lock_};

        if (false == compare_(versions_.back(), rhs)) { return false; }

        versions_.emplace_back(rhs);

        return true;
    }

    LatestVersion(const Return& value, Compare compare) noexcept
        : compare_(std::move(compare))
        , lock_()
        , versions_({value})
    {
        assert_false(versions_.empty());
    }

private:
    const Compare compare_;
    mutable std::mutex lock_;
    UnallocatedList<Store> versions_;
};
}  // namespace opentxs
