// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <span>

#include "opentxs/Types.hpp"
#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
class PendingWrite
{
public:
    auto Data() const noexcept -> std::span<const ReadView>;
    auto Keys() const noexcept -> std::span<const Hash>;
    auto size() const noexcept -> std::size_t;

    auto Add(const Hash& key, ReadView data) noexcept -> void;
    auto Add(const Hash& key) noexcept -> Writer;
    auto RecalculateViews() noexcept -> void;
    auto Reset() noexcept -> void;
    auto swap(PendingWrite& rhs) noexcept -> void;

    PendingWrite() noexcept;
    PendingWrite(const PendingWrite&) = delete;
    PendingWrite(PendingWrite&& rhs) noexcept = default;
    auto operator=(const PendingWrite&) -> PendingWrite& = delete;
    auto operator=(PendingWrite&&) -> PendingWrite& = delete;

private:
    Vector<Hash> key_;
    Vector<CString> data_;
    Vector<ReadView> view_;
};
}  // namespace opentxs::storage::driver
