// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/wallet/Position.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::database::wallet::db
{
Position::Position(const block::Position& position) noexcept
    : data_([&] {
        const auto& [height, hash] = position;
        auto out = space(fixed_);
        auto* it = reinterpret_cast<std::byte*>(out.data());
        std::memcpy(it, &height, sizeof(height));
        std::advance(it, sizeof(height));
        std::memcpy(it, hash->data(), hash->size());

        return out;
    }())
    , lock_()
    , position_(position)
{
    static_assert(40u == fixed_);
}

Position::Position(const ReadView bytes) noexcept(false)
    : data_(space(bytes))
    , lock_()
    , position_()
{
    if (data_.size() != fixed_) {
        throw std::runtime_error{"Input byte range incorrect for Position"};
    }
}

auto Position::Decode(const api::Session& api) const noexcept
    -> const block::Position&
{
    auto lock = Lock{lock_};

    if (false == position_.has_value()) {
        position_.emplace(Height(), api.Factory().Data(Hash()));
    }

    return position_.value();
}

auto Position::Hash() const noexcept -> ReadView
{
    static constexpr auto offset = sizeof(block::Height);
    static constexpr auto size = fixed_ - offset;
    const auto start = std::next(data_.data(), offset);

    return {reinterpret_cast<const char*>(start), size};
}

auto Position::Height() const noexcept -> block::Height
{
    auto out = block::Height{};
    static constexpr auto offset = std::size_t{0};
    static constexpr auto size = sizeof(out);
    const auto start = std::next(data_.data(), offset);
    std::memcpy(&out, start, size);

    return out;
}
}  // namespace opentxs::blockchain::database::wallet::db
