// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/util/Log.hpp"  // IWYU pragma: associated

#include <cassert>
#include <chrono>
#include <cstring>
#include <memory>
#include <sstream>

#include "BoostStacktrace.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "util/log/Imp.hpp"

namespace opentxs
{
Log::Log(Imp* imp) noexcept
    : imp_(imp)
{
    assert(imp_);
}

auto Log::Abort() const noexcept -> void { imp_->Abort(); }

auto Log::Assert(const char* file, const std::size_t line) noexcept -> void
{
    Assert(file, line, nullptr);
}

auto Log::Assert(
    const char* file,
    const std::size_t line,
    const char* message) noexcept -> void
{
    LogError().imp_->Assert(file, line, message);
}

auto Log::asHex(const Data& in) const noexcept -> const Log&
{
    imp_->asHex(in);

    return *this;
}

auto Log::asHex(std::string_view in) const noexcept -> const Log&
{
    imp_->asHex(in);

    return *this;
}

auto Log::Flush() const noexcept -> void { imp_->Flush(); }

auto Log::Internal() const noexcept -> const internal::Log& { return *imp_; }

auto Log::Internal() noexcept -> internal::Log& { return *imp_; }

auto Log::operator()() const noexcept -> const Log& { return *this; }

auto Log::operator()(char* in) const noexcept -> const Log&
{
    return operator()(std::string_view{in, std::strlen(in)});
}

auto Log::operator()(const Amount& in) const noexcept -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const Amount& in, UnitType currency) const noexcept
    -> const Log&
{
    imp_->Buffer(in, currency);

    return *this;
}

auto Log::operator()(const Amount& in, const display::Scale& scale)
    const noexcept -> const Log&
{
    imp_->Buffer(in, scale);

    return *this;
}

auto Log::operator()(const Armored& in) const noexcept -> const Log&
{
    return operator()(in.Get());
}

auto Log::operator()(const CString& in) const noexcept -> const Log&
{
    imp_->Buffer(std::string_view{in});

    return *this;
}

auto Log::operator()(const PaymentCode& in) const noexcept -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const String& in) const noexcept -> const Log&
{
    return operator()(in.Get());
}

auto Log::operator()(const StringXML& in) const noexcept -> const Log&
{
    return operator()(in.Get());
}

auto Log::operator()(const Time in) const noexcept -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const UnallocatedCString& in) const noexcept -> const Log&
{
    imp_->Buffer(std::string_view{in});

    return *this;
}

auto Log::operator()(const blockchain::block::Outpoint& in) const noexcept
    -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const blockchain::block::Position& in) const noexcept
    -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const boost::system::error_code& in) const noexcept
    -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const char* in) const noexcept -> const Log&
{
    return operator()(std::string_view{in, std::strlen(in)});
}

auto Log::operator()(const identifier::Account& in, const api::Crypto& api)
    const noexcept -> const Log&
{
    return operator()(static_cast<const identifier::Generic&>(in), api);
}

auto Log::operator()(const identifier::Generic& in, const api::Crypto& api)
    const noexcept -> const Log&
{
    imp_->Buffer(in, api);

    return *this;
}

auto Log::operator()(const identifier::Nym& in, const api::Crypto& api)
    const noexcept -> const Log&
{
    return operator()(static_cast<const identifier::Generic&>(in), api);
}

auto Log::operator()(const identifier::Notary& in, const api::Crypto& api)
    const noexcept -> const Log&
{
    return operator()(static_cast<const identifier::Generic&>(in), api);
}

auto Log::operator()(
    const identifier::UnitDefinition& in,
    const api::Crypto& api) const noexcept -> const Log&
{
    return operator()(static_cast<const identifier::Generic&>(in), api);
}

auto Log::operator()(const std::chrono::nanoseconds& in) const noexcept
    -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const std::filesystem::path& in) const noexcept
    -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::operator()(const std::string_view in) const noexcept -> const Log&
{
    imp_->Buffer(in);

    return *this;
}

auto Log::Trace(const char* file, const std::size_t line) noexcept -> void
{
    Trace(file, line, nullptr);
}

auto Log::Trace(
    const char* file,
    const std::size_t line,
    const char* message) noexcept -> void
{
    LogError().imp_->Trace(file, line, message);
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
auto LogAbort() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(-2).release()};

    return logger;
}

auto LogConsole() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(0).release()};

    return logger;
}

auto LogDebug() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(3).release()};

    return logger;
}

auto LogDetail() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(1).release()};

    return logger;
}

auto LogError() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(-1).release()};

    return logger;
}

auto LogInsane() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(5).release()};

    return logger;
}

auto LogTrace() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(4).release()};

    return logger;
}

auto LogVerbose() noexcept -> Log&
{
    static auto logger = Log{std::make_unique<Log::Imp>(2).release()};

    return logger;
}

auto PrintStackTrace() noexcept -> UnallocatedCString
{
    auto output = std::stringstream{};
    output << boost::stacktrace::stacktrace();

    return output.str();
}
}  // namespace opentxs
