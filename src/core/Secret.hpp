// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <sodium.h>
}

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <optional>

#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Allocator.hpp"

namespace opentxs::implementation
{
class Secret final : virtual public opentxs::Secret
{
public:
    auto operator==(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator==(rhs.Bytes());
    }
    auto operator==(const ReadView rhs) const noexcept -> bool final;
    auto operator!=(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator!=(rhs.Bytes());
    }
    auto operator!=(const ReadView rhs) const noexcept -> bool final;
    auto operator<(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator<(rhs.Bytes());
    }
    auto operator<(const ReadView rhs) const noexcept -> bool final;
    auto operator>(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator>(rhs.Bytes());
    }
    auto operator>(const ReadView rhs) const noexcept -> bool final;
    auto operator<=(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator<=(rhs.Bytes());
    }
    auto operator<=(const ReadView& rhs) const noexcept -> bool final;
    auto operator>=(const opentxs::Secret& rhs) const noexcept -> bool final
    {
        return operator>=(rhs.Bytes());
    }
    auto operator>=(const ReadView& rhs) const noexcept -> bool final;
    auto Bytes() const noexcept -> ReadView final;
    auto data() const noexcept -> const std::byte* final
    {
        return data_.data();
    }
    auto empty() const noexcept -> bool final { return data_.empty(); }
    auto size() const noexcept -> std::size_t final;

    auto operator+=(const opentxs::Secret& rhs) noexcept -> Secret& final;
    auto operator+=(const ReadView rhs) noexcept -> Secret& final;

    auto Assign(const opentxs::Secret& rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Assign(const ReadView rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Assign(const void* data, const std::size_t& size) noexcept
        -> void final;
    auto AssignText(const ReadView source) noexcept -> void final;
    auto clear() noexcept -> void final { data_.clear(); }
    auto Concatenate(const opentxs::Secret& rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Concatenate(const ReadView rhs) noexcept -> void final
    {
        return Assign(rhs.data(), rhs.size());
    }
    auto Concatenate(const void* data, const std::size_t size) noexcept
        -> void final;
    auto data() noexcept -> std::byte* final { return data_.data(); }
    auto Randomize(const std::size_t bytes) noexcept -> std::size_t final;
    auto Resize(const std::size_t size) noexcept -> std::size_t final;
    auto WriteInto(const std::optional<Mode> = {}) noexcept
        -> AllocateOutput final;

    Secret(const std::size_t bytes) noexcept;
    Secret(const ReadView bytes, const Mode mode) noexcept;
    ~Secret() final = default;

private:
    friend opentxs::Secret;

    using ByteAllocator = SecureAllocator<std::byte>;

    static const ByteAllocator allocator_;

    Mode mode_;
    std::vector<std::byte, ByteAllocator> data_;

    auto clone() const noexcept -> Secret* final { return new Secret(*this); }
    auto mem() const noexcept -> bool { return Mode::Mem == mode_; }
    auto spaceship(const ReadView rhs) const noexcept -> int;

    Secret() = delete;
    Secret(const Secret& rhs) noexcept;
    Secret(Secret&& rhs) = delete;
    auto operator=(const Secret& rhs) -> Secret& = delete;
    auto operator=(Secret&& rhs) -> Secret& = delete;
};
}  // namespace opentxs::implementation
