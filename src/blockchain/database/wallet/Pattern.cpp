// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/wallet/Pattern.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>

#include "internal/util/P0330.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::database::wallet::db
{
auto operator==(const Pattern& lhs, const Pattern& rhs) noexcept -> bool
{
    return lhs.data_ == rhs.data_;
}

Pattern::Pattern(const crypto::Bip32Index index, const ReadView data) noexcept
    : data_([&] {
        auto out = space(fixed_ + data.size());
        auto* it = reinterpret_cast<std::byte*>(out.data());
        std::memcpy(it, &index, sizeof(index));
        std::advance(it, sizeof(index));
        std::memcpy(it, data.data(), data.size());

        return out;
    }())
{
    static_assert(4u == fixed_);
}

Pattern::Pattern(const ReadView bytes) noexcept(false)
    : data_(space(bytes))
{
    if (data_.size() <= fixed_) {
        throw std::runtime_error{"Input byte range too small for Pattern"};
    }
}

Pattern::Pattern(Pattern&& rhs) noexcept
    : data_()
{
    const_cast<Space&>(data_).swap(const_cast<Space&>(rhs.data_));
}

auto Pattern::Data() const noexcept -> ReadView
{
    static constexpr auto offset = fixed_;
    const auto size = data_.size() - offset;
    const auto* const start = std::next(data_.data(), offset);

    return {reinterpret_cast<const char*>(start), size};
}

auto Pattern::Index() const noexcept -> crypto::Bip32Index
{
    auto out = crypto::Bip32Index{};
    static constexpr auto offset = 0_uz;
    static constexpr auto size = sizeof(out);
    const auto* const start = std::next(data_.data(), offset);
    std::memcpy(&out, start, size);

    return out;
}
}  // namespace opentxs::blockchain::database::wallet::db
