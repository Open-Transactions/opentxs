// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>

#include "opentxs/util/Container.hpp"
#include "util/log/Logger.hpp"

namespace opentxs::internal
{
class LogBuffer
{
public:
    using Source = Logger::Source;

    static auto Reset(std::thread::id id, std::stringstream& buf) noexcept
        -> void;

    auto Reset(std::stringstream& buf) const noexcept -> void;
    auto ThreadID() const noexcept -> std::string_view;

    auto Get() noexcept -> std::shared_ptr<Source>;
    auto Refresh() noexcept -> std::shared_ptr<Source>;

    LogBuffer() noexcept;
    LogBuffer(const LogBuffer&) = delete;
    LogBuffer(LogBuffer&&) = delete;
    auto operator=(const LogBuffer&) -> LogBuffer& = delete;
    auto operator=(LogBuffer&&) -> LogBuffer& = delete;

    ~LogBuffer();

private:
    const std::thread::id id_;
    const CString hex_id_;
    const std::shared_ptr<internal::Logger> logger_;
    int session_counter_;
    std::weak_ptr<Source> data_;

    LogBuffer(std::thread::id id) noexcept;
    LogBuffer(
        std::thread::id id,
        std::pair<int, std::shared_ptr<Source>>) noexcept;
};
}  // namespace opentxs::internal
