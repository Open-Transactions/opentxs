// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Log;  // IWYU pragma: keep
class LogPrivate;
}  // namespace internal
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::Log
{
public:
    auto Reset() noexcept -> void;

    Log(std::unique_ptr<LogPrivate> imp) noexcept;
    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    auto operator=(const Log&) -> Log& = delete;
    auto operator=(Log&&) -> Log& = delete;

    ~Log();

private:
    std::unique_ptr<LogPrivate> imp_;
};
