// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/zeromq/Types.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>

#include "internal/core/Core.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::zeromq
{
using namespace std::literals;

constexpr auto inproc_prefix_{"inproc://opentxs/"sv};
constexpr auto path_seperator_{"/"sv};
constexpr auto z85_size_ = 41_uz;

auto CurveKeypair(Writer&& sec, Writer&& pub) noexcept -> bool
{
    try {
        constexpr auto decodedSize = 32_uz;
        static_assert(1_uz + decodedSize * 5_uz / 4_uz == z85_size_);
        auto pubkey = FixedByteArray<z85_size_>{};
        auto seckey = factory::Secret(z85_size_);

        if (false == CurveKeypairZ85(seckey.WriteInto(), pubkey.WriteInto())) {
            throw std::runtime_error{"failed to derive keys"};
        }

        {
            auto buf = sec.Reserve(decodedSize);

            if (false == buf.IsValid(decodedSize)) {
                throw std::runtime_error{
                    "failed to reserve space for secret key"};
            }

            const auto* result = ::zmq_z85_decode(
                buf.as<std::uint8_t>(),
                static_cast<const char*>(seckey.data()));

            if (nullptr == result) {
                throw std::runtime_error{"failed to decode secret key"};
            }
        }
        {
            auto buf = pub.Reserve(decodedSize);

            if (false == buf.IsValid(decodedSize)) {
                throw std::runtime_error{
                    "failed to reserve space for public key"};
            }

            const auto* result = ::zmq_z85_decode(
                buf.as<std::uint8_t>(),
                static_cast<const char*>(pubkey.data()));

            if (nullptr == result) {
                throw std::runtime_error{"failed to decode public key"};
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(e.what()).Flush();

        return false;
    }
}

auto CurveKeypairZ85(Writer&& sec, Writer&& pub) noexcept -> bool
{
    try {
        auto pubkey = pub.Reserve(z85_size_);
        auto seckey = sec.Reserve(z85_size_);

        if (false == pubkey.IsValid(z85_size_)) {
            throw std::runtime_error{"failed to reserve space for public key"};
        }

        if (false == seckey.IsValid(z85_size_)) {
            throw std::runtime_error{"failed to reserve space for secret key"};
        }

        const auto rc =
            ::zmq_curve_keypair(pubkey.as<char>(), seckey.as<char>());

        if (0 != rc) { throw std::runtime_error{"failed to derive key"}; }

        return true;
    } catch (const std::exception& e) {
        LogError()(e.what()).Flush();

        return false;
    }
}

auto DefaultProcessor() noexcept -> actor::Processor
{
    return [](auto, auto, auto&&, auto alloc) {
        return actor::Replies{alloc.result_};
    };
}

auto DefaultShutdown() noexcept -> actor::Shutdown
{
    return []() {};
}

auto DefaultStartup() noexcept -> actor::StateMachine
{
    return [](auto) { return false; };
}

auto DefaultStateMachine() noexcept -> actor::Startup
{
    return [](auto) { return false; };
}

auto MakeArbitraryInproc() noexcept -> UnallocatedCString
{
    static auto counter = std::atomic_int{0};
    auto out = std::stringstream{};
    out << inproc_prefix_;
    out << "arbitrary"sv;
    out << path_seperator_;
    out << std::to_string(++counter);

    return out.str();
}

auto MakeArbitraryInproc(alloc::Default alloc) noexcept -> CString
{
    const auto data = MakeArbitraryInproc();
    auto out = CString{alloc};
    out.assign(data.data(), data.size());

    return out;
}

auto MakeDeterministicInproc(
    const std::string_view path,
    const int instance,
    const int version) noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << inproc_prefix_;
    out << std::to_string(instance);
    out << path_seperator_;
    out << path;
    out << path_seperator_;
    out << std::to_string(version);

    return out.str();
}

auto MakeDeterministicInproc(
    const std::string_view path,
    const int instance,
    const int version,
    const std::string_view suffix) noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << MakeDeterministicInproc(path, instance, version);
    out << path_seperator_;
    out << suffix;

    return out.str();
}

auto RawToZ85(const ReadView input, Writer&& destination) noexcept -> bool
{
    if (const auto size = input.size(); 0_uz != size % 4_uz) {
        LogError()()("Invalid input size ")(size)(" not divisible by 4")
            .Flush();

        return false;
    }

    const auto target = input.size() + input.size() / 4_uz;
    const auto nullTerminatedTarget = target + 1_uz;
    auto buffer = std::optional<Space>{};
    auto out = [&] {
        if (destination.CanTruncate()) {

            return destination.Reserve(nullTerminatedTarget);
        } else {
            buffer.emplace();

            return writer(*buffer).Reserve(nullTerminatedTarget);
        }
    }();

    if (false == out.IsValid(nullTerminatedTarget)) {
        LogError()()("Failed to allocate output").Flush();

        return false;
    }

    const auto* rc = ::zmq_z85_encode(
        out.as<char>(),
        reinterpret_cast<const std::uint8_t*>(input.data()),
        input.size());

    if (nullptr == rc) {
        LogError()()("failed to encoded bytes").Flush();

        return false;
    }

    if (destination.CanTruncate()) {
        if (destination.Truncate(target)) {

            return true;
        } else {
            LogError()()("failed to strip superfluous null terminator").Flush();

            return false;
        }
    } else {
        auto bytes = reader(*buffer);
        bytes.remove_suffix(1_uz);

        return copy(bytes, std::move(destination));
    }
}

auto Z85ToRaw(
    const ReadView input,
    Writer&& destination,
    bool inputIsNullTerminated) noexcept -> bool
{
    auto buffer = std::optional<CString>{};
    const auto view = [&]() -> std::string_view {
        if (inputIsNullTerminated) {

            return input;
        } else {
            buffer.emplace(input);

            return *buffer;
        }
    }();

    if (const auto size = view.size(); 0_uz != size % 5_uz) {
        LogError()()("Invalid input size ")(size)(" not divisible by 5")
            .Flush();

        return false;
    }

    const auto target = view.size() * 4_uz / 5_uz;
    auto out = destination.Reserve(target);

    if (false == out.IsValid(target)) {
        LogError()()("Failed to allocate output").Flush();

        return false;
    }

    return ::zmq_z85_decode(out.as<std::uint8_t>(), view.data());
}
}  // namespace opentxs::network::zeromq
