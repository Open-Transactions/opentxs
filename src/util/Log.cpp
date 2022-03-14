// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"          // IWYU pragma: associated
#include "1_Internal.hpp"        // IWYU pragma: associated
#include "opentxs/util/Log.hpp"  // IWYU pragma: associated

#include <boost/multiprecision/cpp_dec_float.hpp>  // IWYU pragma: keep
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/stacktrace.hpp>
#include <boost/system/error_code.hpp>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <future>
#include <memory>
#include <thread>

#include "internal/core/Amount.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/util/Log.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Log.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::internal
{
auto Log::Endpoint() noexcept -> const char*
{
    static const auto output = zmq::MakeDeterministicInproc("logsink", -1, 1);

    return output.c_str();
}

auto Log::SetVerbosity(const int level) noexcept -> void
{
    static auto& logger = opentxs::Log::Imp::logger_;
    logger.verbosity_ = level;
}

auto Log::Shutdown() noexcept -> void
{
    static auto& logger = opentxs::Log::Imp::logger_;
    logger.running_.shutdown();
    auto lock = Lock{logger.lock_};
    logger.map_.clear();
}

auto Log::Start() noexcept -> void {}
}  // namespace opentxs::internal

namespace opentxs
{
Log::Imp::Logger Log::Imp::logger_{};

Log::Imp::Imp(const int logLevel, opentxs::Log& parent) noexcept
    : level_(logLevel)
    , parent_(parent)
{
}

auto Log::Imp::active() const noexcept -> bool
{
    return logger_.verbosity_.load() >= level_;
}

auto Log::Imp::Assert(
    const char* file,
    const std::size_t line,
    const char* message) const noexcept -> void
{
    if (auto done = logger_.running_.get(); false == done) {
        auto id = UnallocatedCString{};
        auto& [socket, buffer] = get_buffer(id);
        buffer = std::stringstream{};
        buffer << "OT ASSERT";

        if (nullptr != file) { buffer << " in " << file << " line " << line; }

        if (nullptr != message) { buffer << ": " << message; }

        buffer << "\n" << boost::stacktrace::stacktrace();
    }

    send(true);
    abort();
}

auto Log::Imp::Flush() const noexcept -> void { send(false); }

auto Log::Imp::get_buffer(UnallocatedCString& out) noexcept -> Logger::Source&
{
    struct Buffer {
        const std::thread::id id_;
        const UnallocatedCString text_;
        const int index_;
        Logger::SourceMap::iterator source_;

        Buffer() noexcept
            : id_(std::this_thread::get_id())
            , text_([&] {
                auto buf = std::stringstream{};
                buf << std::hex << id_;

                return buf.str();
            }())
            , index_(++logger_.index_)
            , source_([&] {
                auto lock = Lock{logger_.lock_};
                auto [it, added] = logger_.map_.try_emplace(
                    index_,
                    [] {
                        using Direction = zmq::socket::Direction;
                        auto out =
                            Context().ZMQ().PushSocket(Direction::Connect);
                        const auto started =
                            out->Start(internal::Log::Endpoint());

                        assert(started);

                        return out;
                    }(),
                    std::stringstream{});
                assert(added);

                return it;
            }())
        {
        }

        ~Buffer()
        {
            auto lock = Lock{logger_.lock_};
            logger_.map_.erase(index_);
        }
    };

    static thread_local auto buffer = Buffer{};
    out = buffer.text_;

    return buffer.source_->second;
}

auto Log::Imp::operator()(const std::string_view in) const noexcept
    -> const opentxs::Log&
{
    if (false == active()) { return parent_; }

    auto id = UnallocatedCString{};

    if (auto done = logger_.running_.get(); false == done) {
        std::get<1>(get_buffer(id)) << in;
    }

    return parent_;
}

auto Log::Imp::operator()(const boost::system::error_code& error) const noexcept
    -> const opentxs::Log&
{
    if (false == active()) { return parent_; }

    auto id = UnallocatedCString{};

    if (auto done = logger_.running_.get(); false == done) {
        std::get<1>(get_buffer(id)) << error.message();
    }

    return parent_;
}

auto Log::Imp::send(const bool terminate) const noexcept -> void
{
    if (auto done = logger_.running_.get(); false == done) {
        auto id = UnallocatedCString{};
        auto& [socket, buffer] = get_buffer(id);
        auto message = zmq::Message{};
        message.StartBody();
        message.AddFrame(level_);
        message.AddFrame(buffer.str());
        message.AddFrame(id);
        auto promise = std::promise<void>{};
        auto future = promise.get_future();
        const auto* pPromise = &promise;

        if (terminate) {
            message.AddFrame(&pPromise, sizeof(pPromise));
        } else {
            promise.set_value();
        }

        socket->Send(std::move(message));
        buffer = std::stringstream{};
        future.wait_for(10s);
    }

    if (terminate) { abort(); }
}

auto Log::Imp::Trace(
    const char* file,
    const std::size_t line,
    const char* message) const noexcept -> void
{
    if (auto done = logger_.running_.get(); false == done) {
        UnallocatedCString id{};
        auto& [socket, buffer] = get_buffer(id);
        buffer = std::stringstream{};
        buffer << "Stack trace requested";

        if (nullptr != file) { buffer << " in " << file << " line " << line; }

        if (nullptr != message) { buffer << ": " << message; }

        buffer << "\n" << PrintStackTrace();
    }

    send(false);
}
}  // namespace opentxs

namespace opentxs
{
Log::Log(const int logLevel) noexcept
    : imp_(std::make_unique<Imp>(logLevel, *this).release())
{
}

auto Log::operator()() const noexcept -> const Log& { return *this; }

auto Log::operator()(char* in) const noexcept -> const Log&
{
    return operator()(std::string_view{in, std::strlen(in)});
}

auto Log::operator()(const char* in) const noexcept -> const Log&
{
    return operator()(std::string_view{in, std::strlen(in)});
}

auto Log::operator()(const std::string_view in) const noexcept -> const Log&
{
    return (*imp_)(in);
}

auto Log::operator()(const CString& in) const noexcept -> const Log&
{
    return (*imp_)(in);
}

auto Log::operator()(const UnallocatedCString& in) const noexcept -> const Log&
{
    return (*imp_)(in);
}

auto Log::operator()(const std::chrono::nanoseconds& in) const noexcept
    -> const Log&
{
    if (false == imp_->active()) { return *this; }

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

    return (*imp_)(value.str());
}

auto Log::operator()(const OTString& in) const noexcept -> const Log&
{
    return (*imp_)(in->Bytes());
}

auto Log::operator()(const OTArmored& in) const noexcept -> const Log&
{
    return operator()(in.get());
}

auto Log::operator()(const Amount& in) const noexcept -> const Log&
{
    if (false == imp_->active()) { return *this; }

    const auto intValue = [&] {
        auto out = UnallocatedCString{};
        in.Serialize(opentxs::writer(out));

        return out;
    }();
    const auto floatValue = in.Internal().ToFloat();

    return operator()(floatValue.str() + " (" + intValue + ")");
}

auto Log::operator()(const Amount& in, UnitType currency) const noexcept
    -> const Log&
{
    if (false == imp_->active()) { return *this; }

    if (UnitType::Unknown == currency) { return operator()(in); }

    const auto intValue = [&] {
        auto out = UnallocatedCString{};
        in.Serialize(opentxs::writer(out));

        return out;
    }();

    return operator()(
        display::GetDefinition(currency).Format(in) + " (" + intValue + ")");
}

auto Log::operator()(const Amount& in, const display::Scale& scale)
    const noexcept -> const Log&
{
    if (false == imp_->active()) { return *this; }

    return operator()(scale.Format(in));
}

auto Log::operator()(const String& in) const noexcept -> const Log&
{
    return operator()(in.Get());
}

auto Log::operator()(const StringXML& in) const noexcept -> const Log&
{
    return operator()(in.Get());
}

auto Log::operator()(const Armored& in) const noexcept -> const Log&
{
    return operator()(in.Get());
}

auto Log::operator()(const OTIdentifier& in) const noexcept -> const Log&
{
    return operator()(in.get());
}

auto Log::operator()(const Identifier& in) const noexcept -> const Log&
{
    if (false == imp_->active()) { return *this; }

    return (*imp_)(in.str());
}

auto Log::operator()(const OTNymID& in) const noexcept -> const Log&
{
    return operator()(in.get());
}

auto Log::operator()(const identifier::Nym& in) const noexcept -> const Log&
{
    if (false == imp_->active()) { return *this; }

    return (*imp_)(in.str());
}

auto Log::operator()(const OTNotaryID& in) const noexcept -> const Log&
{
    return operator()(in.get());
}

auto Log::operator()(const identifier::Notary& in) const noexcept -> const Log&
{
    if (false == imp_->active()) { return *this; }

    return (*imp_)(in.str());
}

auto Log::operator()(const OTUnitID& in) const noexcept -> const Log&
{
    return operator()(in.get());
}

auto Log::operator()(const identifier::UnitDefinition& in) const noexcept
    -> const Log&
{
    if (false == imp_->active()) { return *this; }

    return (*imp_)(in.str());
}

auto Log::operator()(const Time in) const noexcept -> const Log&
{
    if (false == imp_->active()) { return *this; }

    return operator()(formatTimestamp(in));
}

auto Log::operator()(const boost::system::error_code& error) const noexcept
    -> const Log&
{
    return imp_->operator()(error);
}

auto Log::Assert(const char* file, const std::size_t line) const noexcept
    -> void
{
    Assert(file, line, nullptr);
}

auto Log::Assert(const char* file, const std::size_t line, const char* message)
    const noexcept -> void
{
    imp_->Assert(file, line, message);
}

auto Log::Flush() const noexcept -> void { imp_->Flush(); }

auto Log::Trace(const char* file, const std::size_t line) const noexcept -> void
{
    Trace(file, line, nullptr);
}

auto Log::Trace(const char* file, const std::size_t line, const char* message)
    const noexcept -> void
{
    imp_->Trace(file, line, message);
}

Log::~Log()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs

namespace opentxs
{
auto LogConsole() noexcept -> Log&
{
    static auto logger = Log{0};

    return logger;
}

auto LogDebug() noexcept -> Log&
{
    static auto logger = Log{3};

    return logger;
}

auto LogDetail() noexcept -> Log&
{
    static auto logger = Log{1};

    return logger;
}

auto LogError() noexcept -> Log&
{
    static auto logger = Log{-1};

    return logger;
}

auto LogInsane() noexcept -> Log&
{
    static auto logger = Log{5};

    return logger;
}

auto LogTrace() noexcept -> Log&
{
    static auto logger = Log{4};

    return logger;
}

auto LogVerbose() noexcept -> Log&
{
    static auto logger = Log{2};

    return logger;
}

auto PrintStackTrace() noexcept -> UnallocatedCString
{
    auto output = std::stringstream{};
    output << boost::stacktrace::stacktrace();

    return output.str();
}
}  // namespace opentxs
