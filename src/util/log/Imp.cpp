// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"      // IWYU pragma: associated
#include "util/log/Imp.hpp"  // IWYU pragma: associated

#include <boost/multiprecision/cpp_dec_float.hpp>  // IWYU pragma: keep
#include <boost/stacktrace.hpp>
#include <boost/system/error_code.hpp>
#include <atomic>
#include <chrono>
#include <compare>
#include <cstdlib>
#include <ratio>
#include <sstream>
#include <utility>

#include "internal/core/Amount.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/util/Log.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/log/LogBuffer.hpp"
#include "util/log/Logger.hpp"

namespace opentxs
{
using namespace std::literals;

Log::Imp::Imp(const int logLevel) noexcept
    : level_(logLevel)
    , logger_(GetLogger())
{
}

auto Log::Imp::Abort() const noexcept -> void
{
    buffer("\n"sv);
    buffer(PrintStackTrace());
    send(LogAction::terminate, Console::err);
    wait_for_terminate();
}

auto Log::Imp::active() const noexcept -> bool
{
    return logger_->verbosity_.load() >= level_;
}

auto Log::Imp::asHex(const Data& in) const noexcept -> void
{
    if (false == active()) { return; }

    buffer(in.asHex());
}

auto Log::Imp::asHex(std::string_view in) const noexcept -> void
{
    if (false == active()) { return; }

    buffer(to_hex(reinterpret_cast<const std::byte*>(in.data()), in.size()));
}

auto Log::Imp::Assert(
    const char* file,
    const std::size_t line,
    const char* message) const noexcept -> void
{
    const auto text = [&] {
        auto out = std::stringstream{"OT ASSERT"};

        if (nullptr != file) { out << " in " << file << " line " << line; }

        if (nullptr != message) { out << ": " << message; }

        out << "\n" << boost::stacktrace::stacktrace();

        return out;
    }();
    buffer(text.str());
    Abort();
}

auto Log::Imp::Buffer(const Amount& in) const noexcept -> void
{
    if (false == active()) { return; }

    const auto intValue = [&] {
        auto out = UnallocatedCString{};
        in.Serialize(opentxs::writer(out));

        return out;
    }();
    const auto floatValue = in.Internal().ToFloat();
    buffer(floatValue.str() + " (" + intValue + ")");
}

auto Log::Imp::Buffer(const Amount& in, UnitType currency) const noexcept
    -> void
{
    if (false == active()) { return; }

    if (UnitType::Unknown == currency) { return Buffer(in); }

    const auto intValue = [&] {
        auto out = UnallocatedCString{};
        in.Serialize(opentxs::writer(out));

        return out;
    }();
    buffer(display::GetDefinition(currency).Format(in) + " (" + intValue + ")");
}

auto Log::Imp::Buffer(const Amount& in, const display::Scale& scale)
    const noexcept -> void
{
    if (false == active()) { return; }

    buffer(scale.Format(in));
}

auto Log::Imp::Buffer(const Time in) const noexcept -> void
{
    if (false == active()) { return; }

    buffer(formatTimestamp(in));
}

auto Log::Imp::Buffer(const blockchain::block::Outpoint& in) const noexcept
    -> void
{
    if (false == active()) { return; }

    buffer(in.str());
}

auto Log::Imp::Buffer(const blockchain::block::Position& in) const noexcept
    -> void
{
    if (false == active()) { return; }

    buffer(in.print());
}

auto Log::Imp::Buffer(const boost::system::error_code& in) const noexcept
    -> void
{
    if (false == active()) { return; }

    buffer(in.message());
}

auto Log::Imp::Buffer(const identifier::Generic& in) const noexcept -> void
{
    if (false == active()) { return; }

    buffer(in.asBase58(Context().Crypto()));
}

auto Log::Imp::Buffer(const std::chrono::nanoseconds& in) const noexcept -> void
{
    if (false == active()) { return; }

    auto value = std::stringstream{};
    static constexpr auto nanoThreshold = 2us;
    static constexpr auto microThreshold = 2ms;
    static constexpr auto milliThreshold = 2s;
    static constexpr auto threshold = std::chrono::minutes{2};
    static constexpr auto minThreshold = std::chrono::hours{2};
    static constexpr auto usRatio = 1000ull;
    static constexpr auto msRatio = 1000ull * usRatio;
    static constexpr auto ratio = 1000ull * msRatio;
    static constexpr auto minRatio = 60ull * ratio;
    static constexpr auto hourRatio = 60ull * minRatio;

    if (in < nanoThreshold) {
        value << std::to_string(in.count()) << " nanoseconds";
    } else if (in < microThreshold) {
        value << std::to_string(in.count() / usRatio) << " microseconds";
    } else if (in < milliThreshold) {
        value << std::to_string(in.count() / msRatio) << " milliseconds";
    } else if (in < threshold) {
        value << std::to_string(in.count() / ratio) << " seconds";
    } else if (in < minThreshold) {
        value << std::to_string(in.count() / minRatio) << " minutes";
    } else {
        value << std::to_string(in.count() / hourRatio) << " hours";
    }

    buffer(value.str());
}

auto Log::Imp::Buffer(const std::filesystem::path& in) const noexcept -> void
{
    if (false == active()) { return; }

    buffer(in.string().c_str());
}

auto Log::Imp::Buffer(const std::string_view in) const noexcept -> void
{
    if (false == active()) { return; }

    buffer(in);
}

auto Log::Imp::buffer(std::string_view text) const noexcept -> void
{
    if (false == valid(text)) { return; }

    if (auto p = get_data(); p) { p->first << text; }
}

auto Log::Imp::Flush() const noexcept -> void
{
    const auto console = [&] {
        switch (level_) {
            case 0: {

                return Console::out;
            }
            default: {

                return Console::err;
            }
        }
    }();
    const auto action = [&] {
        switch (level_) {
            case -2: {

                return LogAction::terminate;
            }
            default: {

                return LogAction::flush;
            }
        }
    }();
    send(action, console);
}

auto Log::Imp::get_buffer() noexcept -> internal::LogBuffer&
{
    static thread_local auto buffer = internal::LogBuffer{};

    return buffer;
}

auto Log::Imp::get_data() noexcept -> std::shared_ptr<internal::Logger::Source>
{
    return get_data(get_buffer());
}

auto Log::Imp::get_data(internal::LogBuffer& buf) noexcept
    -> std::shared_ptr<internal::Logger::Source>
{
    auto out = buf.Get();

    // NOTE this makes logging work if the Context is shutdown then restarted
    if (false == out.operator bool()) { out = buf.Refresh(); }

    return out;
}

auto Log::Imp::send(const LogAction action, const Console console)
    const noexcept -> void
{
    auto& buf = get_buffer();
    const auto id = buf.ThreadID();

    if (auto p = get_data(buf); p) {
        auto& [buffer, socket] = *p;
        // TODO c++20
        socket.SendDeferred(
            [&](const auto& text) {
                auto message = network::zeromq::Message{};
                message.StartBody();
                message.AddFrame(level_);
                message.AddFrame(text.str());
                message.AddFrame(id.data(), id.size());
                message.AddFrame(action);
                message.AddFrame(console);

                return message;
            }(buffer),
            __FILE__,
            __LINE__);
        buf.Reset(buffer);
    }

    if (LogAction::terminate == action) { wait_for_terminate(); }
}

auto Log::Imp::Trace(
    const char* file,
    const std::size_t line,
    const char* message) const noexcept -> void
{
    const auto text = [&] {
        auto out = std::stringstream{"Stack trace requested"};

        if (nullptr != file) { out << " in " << file << " line " << line; }

        if (nullptr != message) { out << ": " << message; }

        out << "\n" << boost::stacktrace::stacktrace();

        return out;
    }();
    Flush();
}

auto Log::Imp::wait_for_terminate() const noexcept -> void
{
    Sleep(10s);
    std::abort();
}
}  // namespace opentxs
