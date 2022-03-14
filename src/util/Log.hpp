// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <mutex>
#include <sstream>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>

#include "internal/otx/common/StringXML.hpp"
#include "internal/util/Log.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Gatekeeper.hpp"

namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs
{
struct Log::Imp final : public internal::Log {
    struct Logger {
        using Source = std::pair<OTZMQPushSocket, std::stringstream>;
        using SourceMap = UnallocatedMap<int, Source>;

        std::atomic_int verbosity_{-1};
        std::atomic_int index_{-1};
        Gatekeeper running_{};
        std::mutex lock_{};
        SourceMap map_{};
    };

    static Logger logger_;

    auto active() const noexcept -> bool;
    auto operator()(const std::string_view in) const noexcept
        -> const opentxs::Log&;
    auto operator()(const boost::system::error_code& error) const noexcept
        -> const opentxs::Log&;

    [[noreturn]] auto Assert(
        const char* file,
        const std::size_t line,
        const char* message) const noexcept -> void;
    auto Flush() const noexcept -> void;
    auto Trace(const char* file, const std::size_t line, const char* message)
        const noexcept -> void;

    Imp(const int logLevel, opentxs::Log& parent) noexcept;

    ~Imp() final = default;

private:
    const int level_;
    opentxs::Log& parent_;

    static auto get_buffer(UnallocatedCString& id) noexcept -> Logger::Source&;

    auto send(const bool terminate) const noexcept -> void;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs
