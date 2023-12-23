// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <filesystem>
#include <ostream>
#include <source_location>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/storage/Types.hpp"
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
namespace api
{
class Crypto;
}  // namespace api

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
class Account;
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace internal
{
class Log;
}  // namespace internal

namespace log
{
class Streambuf;
}  // namespace log

class Amount;
class Armored;
class Data;
class PaymentCode;
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
        std::string_view message,
        const std::source_location& loc =
            std::source_location::current()) noexcept -> void;
    [[noreturn]] static auto Assert(
        const std::source_location& loc =
            std::source_location::current()) noexcept -> void;
    static auto Trace(
        const std::source_location& loc =
            std::source_location::current()) noexcept -> void;
    static auto Trace(
        std::string_view message,
        const std::source_location& loc =
            std::source_location::current()) noexcept -> void;

    [[noreturn]] auto Abort() const noexcept -> void;
    auto asHex(const Data& in) const noexcept -> const Log&;
    auto asHex(std::string_view in) const noexcept -> const Log&;
    auto Flush() const noexcept -> void;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Log&;
    auto operator()(
        const std::source_location& loc =
            std::source_location::current()) const noexcept -> const Log&;
    auto operator()(char* in) const noexcept -> const Log&;
    auto operator()(const Amount& in) const noexcept -> const Log&;
    auto operator()(const Amount& in, UnitType currency) const noexcept
        -> const Log&;
    auto operator()(const Amount& in, const display::Scale& scale)
        const noexcept -> const Log&;
    auto operator()(const Armored& in) const noexcept -> const Log&;
    auto operator()(const CString& in) const noexcept -> const Log&;
    auto operator()(const PaymentCode& in) const noexcept -> const Log&;
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
    auto operator()(const identifier::Account& in, const api::Crypto& api)
        const noexcept -> const Log&;
    auto operator()(const identifier::Generic& in, const api::Crypto& api)
        const noexcept -> const Log&;
    auto operator()(const identifier::Notary& in, const api::Crypto& api)
        const noexcept -> const Log&;
    auto operator()(const identifier::Nym& in, const api::Crypto& api)
        const noexcept -> const Log&;
    auto operator()(
        const identifier::UnitDefinition& in,
        const api::Crypto& api) const noexcept -> const Log&;
    auto operator()(const storage::Hash& in) const noexcept -> const Log&;
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
}  // namespace opentxs

namespace opentxs::log
{
class OPENTXS_EXPORT Stream final : public std::ostream
{
public:
    Stream(const std::source_location& loc, Streambuf& buf) noexcept;
    Stream() = delete;
    Stream(const Stream&) = delete;
    Stream(Stream&&) = delete;
    auto operator=(const Stream&) -> Stream& = delete;
    auto operator=(Stream&&) -> Stream& = delete;

    ~Stream() final;
};
}  // namespace opentxs::log

namespace opentxs
{
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

namespace opentxs::log
{
OPENTXS_EXPORT auto Abort(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
OPENTXS_EXPORT auto Console(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
OPENTXS_EXPORT auto Debug(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
OPENTXS_EXPORT auto Detail(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
OPENTXS_EXPORT auto Error(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
OPENTXS_EXPORT auto Insane(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
OPENTXS_EXPORT auto Trace(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
OPENTXS_EXPORT auto Verbose(
    const std::source_location& loc = std::source_location::current()) noexcept
    -> Stream;
}  // namespace opentxs::log

namespace opentxs
{
OPENTXS_EXPORT inline auto assert_true(
    bool expression,
    std::string_view message,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void
{
    if (false == expression) { LogAbort()(loc)(message).Abort(); }
}

OPENTXS_EXPORT inline auto assert_true(
    bool expression,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void
{
    assert_true(expression, "assertion failure", loc);
}

OPENTXS_EXPORT inline auto assert_false(
    bool expression,
    std::string_view message,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void
{
    if (expression) { LogAbort()(loc)(message).Abort(); }
}

OPENTXS_EXPORT inline auto assert_false(
    bool expression,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void
{
    assert_false(expression, "assertion failure", loc);
}
}  // namespace opentxs
