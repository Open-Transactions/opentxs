// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/plugin/PendingWrite.hpp"  // IWYU pragma: associated

#include <string_view>

#include "internal/util/P0330.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::storage::driver
{
PendingWrite::PendingWrite() noexcept
    : key_()
    , data_()
    , view_()
{
}

auto PendingWrite::Add(const Hash& key, ReadView data) noexcept -> void
{
    key_.emplace_back(key);
    const auto& value = data_.emplace_back(data);
    view_.emplace_back(value);
}

auto PendingWrite::Add(const Hash& key) noexcept -> Writer
{
    key_.emplace_back(key);
    auto& value = data_.emplace_back();
    view_.emplace_back(value);

    return writer(value);
}

auto PendingWrite::Data() const noexcept -> std::span<const ReadView>
{
    return view_;
}

auto PendingWrite::Keys() const noexcept -> std::span<const Hash>
{
    return key_;
}

auto PendingWrite::RecalculateViews() noexcept -> void
{
    for (auto n = 0_uz; n < data_.size(); ++n) { view_[n] = data_[n]; }
}

auto PendingWrite::Reset() noexcept -> void
{
    key_.clear();
    data_.clear();
    view_.clear();
}

auto PendingWrite::swap(PendingWrite& rhs) noexcept -> void
{
    using std::swap;
    swap(key_, rhs.key_);
    swap(data_, rhs.data_);
    swap(view_, rhs.view_);
}

auto PendingWrite::size() const noexcept -> std::size_t { return key_.size(); }
}  // namespace opentxs::storage::driver
