// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "opentxs/util/Bytes.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto copy(const ReadView in, const AllocateOutput out) noexcept -> bool
{
    return copy(in, out, in.size());
}

auto copy(
    const ReadView in,
    const AllocateOutput out,
    const std::size_t limit) noexcept -> bool
{
    if ((nullptr == in.data()) || (0 == in.size())) {
        LogError()(__func__)(": invalid input").Flush();

        return false;
    }

    if (false == bool(out)) {
        LogError()(__func__)(": invalid allocator").Flush();

        return false;
    }

    const auto size = std::min(in.size(), limit);
    auto write = out(size);

    if (false == write.valid(size)) {
        LogError()(__func__)(": failed to allocate space").Flush();

        return false;
    }

    OT_ASSERT(size == write.size());

    std::memcpy(write.data(), in.data(), size);

    return true;
}

auto preallocated(const std::size_t size, void* out) noexcept -> AllocateOutput
{
    return [=](const auto in) -> WritableView {
        if (in <= size) {

            return {out, in};
        } else {
            LogError()("preallocated(): Requested ")(in)(" bytes but only ")(
                size)(" are available")
                .Flush();

            return {nullptr, 0};
        }
    };
}
auto reader(const Space& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
auto reader(const WritableView& in) noexcept -> ReadView
{
    return {in.as<const char>(), in.size()};
}
auto reader(const std::pmr::vector<std::uint8_t>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
auto reader(const std::vector<std::uint8_t>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
auto space(const std::size_t size) noexcept -> Space
{
    auto output = Space{};
    output.assign(size, std::byte{51});

    return output;
}
auto space(const ReadView bytes) noexcept -> Space
{
    if ((nullptr == bytes.data()) || (0 == bytes.size())) { return {}; }

    auto it = reinterpret_cast<const std::byte*>(bytes.data());

    return {it, it + bytes.size()};
}
auto valid(const ReadView view) noexcept -> bool
{
    return (nullptr != view.data()) && (0 < view.size());
}
auto writer(std::string& in) noexcept -> AllocateOutput
{
    return [&in](const auto size) -> WritableView {
        in.resize(size, 51);

        return {in.data(), in.size()};
    };
}
auto writer(std::string* protobuf) noexcept -> AllocateOutput
{
    if (nullptr == protobuf) { return {}; }

    return writer(*protobuf);
}
auto writer(Space& in) noexcept -> AllocateOutput
{
    return [&in](const auto size) -> WritableView {
        in.resize(size, std::byte{51});

        return {in.data(), in.size()};
    };
}
}  // namespace opentxs
