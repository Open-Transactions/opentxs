// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "BoostAsio.hpp"

namespace opentxs::api::network::asio
{
class Buffers
{
public:
    using AsioBuffer = decltype(boost::asio::buffer(
        std::declval<void*>(),
        std::declval<std::size_t>()));
    using Index = std::int64_t;
    using Handle = std::pair<Index, AsioBuffer>;

    auto clear(Index id) noexcept -> void;
    auto get(const std::size_t bytes) noexcept -> Handle;

    Buffers() noexcept;
    Buffers(const Buffers&) = delete;
    Buffers(Buffers&&) = delete;
    auto operator=(const Buffers&) -> Buffers& = delete;
    auto operator=(Buffers&&) -> Buffers& = delete;

    ~Buffers();

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::api::network::asio
