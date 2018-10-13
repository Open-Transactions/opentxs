// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_LOGSOURCE_HPP
#define OPENTXS_CORE_LOGSOURCE_HPP

#include "opentxs/Forward.hpp"

#include <atomic>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>

namespace opentxs
{
class LogSource
{
public:
    static void SetVerbosity(const int level);
    static void Shutdown();
    static const LogSource& StartLog(
        const LogSource& source,
        const std::string& function);

    const LogSource& operator()(const char* in) const;
    const LogSource& operator()(const std::string& in) const;
    const LogSource& operator()(const OTString& in) const;
    const LogSource& operator()(const OTStringXML& in) const;
    const LogSource& operator()(const OTArmored& in) const;
    const LogSource& operator()(const String& in) const;
    const LogSource& operator()(const StringXML& in) const;
    const LogSource& operator()(const Armored& in) const;
    const LogSource& operator()(const OTIdentifier& in) const;
    const LogSource& operator()(const Identifier& in) const;
    template <typename T>
    const LogSource& operator()(const T& in) const
    {
        return this->operator()(std::to_string(in));
    }

    void Flush() const;

    explicit LogSource(const int logLevel);

    ~LogSource() = default;

private:
    using Source = std::pair<OTZMQPushSocket, std::stringstream>;

    static std::atomic<int> verbosity_;
    static std::atomic<bool> running_;
    static std::mutex buffer_lock_;
    static std::map<std::thread::id, Source> buffer_;

    const int level_{-1};

    static Source& get_buffer(std::string& id);

    LogSource() = delete;
    LogSource(const LogSource&) = delete;
    LogSource(LogSource&&) = delete;
    LogSource& operator=(const LogSource&) = delete;
    LogSource& operator=(LogSource&&) = delete;
};
}  // namespace opentxs
#endif
