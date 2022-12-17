// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <string_view>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs
{
namespace blockchain
{
namespace block
{
class Outpoint;
class Position;
}  // namespace block
}  // namespace blockchain

namespace display
{
class Scale;
}  // namespace display

namespace identifier
{
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace internal
{
class Log;
}  // namespace internal

class Amount;
class Armored;
class Data;
class String;
class StringXML;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT Log
{
public:
    class Imp;

    [[noreturn]] static auto Assert(
        const char* file,
        const std::size_t line,
        const char* message) noexcept -> void;
    [[noreturn]] static auto Assert(
        const char* file,
        const std::size_t line) noexcept -> void;
    static auto Trace(const char* file, const std::size_t line) noexcept
        -> void;
    static auto Trace(
        const char* file,
        const std::size_t line,
        const char* message) noexcept -> void;

    [[noreturn]] auto Abort() const noexcept -> void;
    auto asHex(const Data& in) const noexcept -> const Log&;
    auto asHex(std::string_view in) const noexcept -> const Log&;
    auto Flush() const noexcept -> void;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Log&;
    auto operator()() const noexcept -> const Log&;
    auto operator()(char* in) const noexcept -> const Log&;
    auto operator()(const Amount& in) const noexcept -> const Log&;
    auto operator()(const Amount& in, UnitType currency) const noexcept
        -> const Log&;
    auto operator()(const Amount& in, const display::Scale& scale)
        const noexcept -> const Log&;
    auto operator()(const Armored& in) const noexcept -> const Log&;
    auto operator()(const CString& in) const noexcept -> const Log&;
    auto operator()(const String& in) const noexcept -> const Log&;
    auto operator()(const StringXML& in) const noexcept -> const Log&;
    auto operator()(const Time in) const noexcept -> const Log&;
    auto operator()(const UnallocatedCString& in) const noexcept -> const Log&;
    auto operator()(const blockchain::block::Outpoint& outpoint) const noexcept
        -> const Log&;
    auto operator()(const blockchain::block::Position& position) const noexcept
        -> const Log&;
    auto operator()(const boost::system::error_code& error) const noexcept
        -> const Log&;
    auto operator()(const char* in) const noexcept -> const Log&;
    auto operator()(const identifier::Generic& in) const noexcept -> const Log&;
    auto operator()(const identifier::Notary& in) const noexcept -> const Log&;
    auto operator()(const identifier::Nym& in) const noexcept -> const Log&;
    auto operator()(const identifier::UnitDefinition& in) const noexcept
        -> const Log&;
    auto operator()(const std::chrono::nanoseconds& in) const noexcept
        -> const Log&;
    auto operator()(const std::filesystem::path& in) const noexcept
        -> const Log&;
    auto operator()(const std::string_view in) const noexcept -> const Log&;
    template <typename T>
    auto operator()(const T& in) const noexcept -> const Log&
    {
        return this->operator()(std::to_string(in));
    }

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Log&;

    OPENTXS_NO_EXPORT Log(Imp* imp) noexcept;
    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    auto operator=(const Log&) -> Log& = delete;
    auto operator=(Log&&) -> Log& = delete;

    OPENTXS_NO_EXPORT ~Log();

private:
    Imp* imp_;
};

OPENTXS_EXPORT auto LogAbort() noexcept -> Log&;
OPENTXS_EXPORT auto LogConsole() noexcept -> Log&;
OPENTXS_EXPORT auto LogDebug() noexcept -> Log&;
OPENTXS_EXPORT auto LogDetail() noexcept -> Log&;
OPENTXS_EXPORT auto LogError() noexcept -> Log&;
OPENTXS_EXPORT auto LogInsane() noexcept -> Log&;
OPENTXS_EXPORT auto LogTrace() noexcept -> Log&;
OPENTXS_EXPORT auto LogVerbose() noexcept -> Log&;
OPENTXS_EXPORT auto PrintStackTrace() noexcept -> UnallocatedCString;
}  // namespace opentxs
