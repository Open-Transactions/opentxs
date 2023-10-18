// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>
#include <utility>

#include "internal/blockchain/database/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
struct Elements final : public Allocated {
    Vector<std::pair<Bip32Index, std::array<std::byte, 20>>> elements_20_;
    Vector<std::pair<Bip32Index, std::array<std::byte, 32>>> elements_32_;
    Vector<std::pair<Bip32Index, std::array<std::byte, 33>>> elements_33_;
    Vector<std::pair<Bip32Index, std::array<std::byte, 64>>> elements_64_;
    Vector<std::pair<Bip32Index, std::array<std::byte, 65>>> elements_65_;
    database::TXOs txos_;

    auto get_allocator() const noexcept -> allocator_type final;
    auto size() const noexcept -> std::size_t;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    Elements(allocator_type alloc = {}) noexcept;
    Elements(const Elements& rhs, allocator_type alloc = {}) noexcept;
    Elements(Elements&& rhs) noexcept;
    Elements(Elements&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Elements& rhs) noexcept -> Elements&;
    auto operator=(Elements&& rhs) noexcept -> Elements&;

    ~Elements() final = default;
};
}  // namespace opentxs::blockchain::node::wallet
