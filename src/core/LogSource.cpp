// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/LogSource.hpp"

#include "opentxs/api/Native.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/OT.hpp"

#define LOG_SINK "inproc://opentxs/logsink/1"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
std::atomic<int> LogSource::verbosity_{0};
std::atomic<bool> LogSource::running_{true};
std::mutex LogSource::buffer_lock_{};
std::map<std::thread::id, LogSource::Source> LogSource::buffer_{};

LogSource::LogSource(const int logLevel)
    : level_(logLevel)
{
}

const LogSource& LogSource::operator()(const char* in) const
{
    if (verbosity_.load() < level_) { return *this; }

    std::string id{};

    if (running_.load()) { std::get<1>(get_buffer(id)) << in; }

    return *this;
}

const LogSource& LogSource::operator()(const std::string& in) const
{
    return operator()(in.c_str());
}

const LogSource& LogSource::operator()(const OTString& in) const
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const OTStringXML& in) const
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const OTArmored& in) const
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const String& in) const
{
    return operator()(in.Get());
}

const LogSource& LogSource::operator()(const StringXML& in) const
{
    return operator()(in.Get());
}

const LogSource& LogSource::operator()(const Armored& in) const
{
    return operator()(in.Get());
}

const LogSource& LogSource::operator()(const OTIdentifier& in) const
{
    return operator()(in.get());
}

const LogSource& LogSource::operator()(const Identifier& in) const
{
    return operator()(in.str().c_str());
}

void LogSource::Flush() const
{
    if (running_.load()) {
        std::string id{};
        auto& [socket, buffer] = get_buffer(id);
        auto message = zmq::Message::Factory();
        message->AddFrame();
        message->AddFrame(std::to_string(level_));
        message->AddFrame(buffer.str());
        message->AddFrame(id);
        socket->Push(message);
        buffer = std::stringstream{};
    }
}

LogSource::Source& LogSource::get_buffer(std::string& out)
{
    const auto id = std::this_thread::get_id();
    std::stringstream convert{};
    convert << id;
    out = convert.str();
    auto it = buffer_.find(id);

    if (buffer_.end() == it) {
        Lock lock(buffer_lock_);
        auto it = buffer_.emplace(
            id,
            Source{OT::App().ZMQ().PushSocket(zmq::Socket::Direction::Connect),
                   std::stringstream{}});
        auto& source = std::get<0>(it)->second;
        auto& socket = std::get<0>(source).get();
        socket.Start(LOG_SINK);

        return source;
    }

    return it->second;
}

void LogSource::SetVerbosity(const int level) { verbosity_.store(level); }

void LogSource::Shutdown()
{
    running_.store(false);
    buffer_.clear();
}

const LogSource& LogSource::StartLog(
    const LogSource& source,
    const std::string& function)
{
    return source(function);
}
}  // namespace opentxs
