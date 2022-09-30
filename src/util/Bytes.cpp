// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "opentxs/util/Bytes.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
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
    if ((nullptr == in.data()) || (0_uz == in.size()) || (0_uz == limit)) {
        return true;
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

            return {nullptr, 0_uz};
        }
    };
}
auto reader(const Space& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
auto reader(const Vector<std::byte>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
auto reader(const WritableView& in) noexcept -> ReadView
{
    return {in.as<const char>(), in.size()};
}
auto reader(const UnallocatedVector<std::uint8_t>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}
auto space(const std::size_t size) noexcept -> Space
{
    auto output = Space{};
    output.assign(size, std::byte{51});

    return output;
}
auto space(const std::size_t size, alloc::Default alloc) noexcept
    -> Vector<std::byte>
{
    auto output = Vector<std::byte>{alloc};
    output.assign(size, std::byte{51});

    return output;
}
auto space(const ReadView bytes) noexcept -> Space
{
    if ((nullptr == bytes.data()) || (0_uz == bytes.size())) { return {}; }

    const auto* it = reinterpret_cast<const std::byte*>(bytes.data());

    return {it, it + bytes.size()};
}
auto space(const ReadView bytes, alloc::Default alloc) noexcept
    -> Vector<std::byte>
{
    using Out = Vector<std::byte>;

    if ((nullptr == bytes.data()) || (0_uz == bytes.size())) {
        return Out{alloc};
    }

    const auto* it = reinterpret_cast<const std::byte*>(bytes.data());

    return Out{it, it + bytes.size(), alloc};
}
auto valid(const ReadView view) noexcept -> bool
{
    return (nullptr != view.data()) && (0_uz < view.size());
}
auto writer(UnallocatedCString& in) noexcept -> AllocateOutput
{
    return [&in](const auto size) -> WritableView {
        in.resize(size, 51);

        return {in.data(), in.size()};
    };
}
auto writer(UnallocatedCString* protobuf) noexcept -> AllocateOutput
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
auto writer(Vector<std::byte>& in) noexcept -> AllocateOutput
{
    return [&in](const auto size) -> WritableView {
        in.resize(size, std::byte{51});

        return {in.data(), in.size()};
    };
}
}  // namespace opentxs
