// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "util/log/LogBuffer.hpp"  // IWYU pragma: associated

#include <memory>
#include <thread>
#include <tuple>

namespace opentxs::internal
{
LogBuffer::LogBuffer(
    std::thread::id id,
    std::pair<int, std::shared_ptr<Source>> data) noexcept
    : id_(id)
    , hex_id_([&] {
        auto buf = std::stringstream{};
        buf << std::hex << id_;

        return buf.str();
    }())
    , logger_(GetLogger())
    , session_counter_(data.first)
    , data_(std::move(data.second))
{
}

LogBuffer::LogBuffer(std::thread::id id) noexcept
    : LogBuffer(id, GetLogger()->Register(id))
{
}

LogBuffer::LogBuffer() noexcept
    : LogBuffer(std::this_thread::get_id())
{
}

auto LogBuffer::Get() noexcept -> std::shared_ptr<Source>
{
    return data_.lock();
}

auto LogBuffer::Refresh() noexcept -> std::shared_ptr<Source>
{
    auto& logger = *logger_;

    if (logger.Session() != session_counter_) {
        std::tie(session_counter_, data_) = logger.Register(id_);
    }

    return Get();
}

auto LogBuffer::Reset(std::stringstream& buf) const noexcept -> void
{
    Reset(id_, buf);
}

auto LogBuffer::Reset(std::thread::id id, std::stringstream& buf) noexcept
    -> void
{
    buf = std::stringstream{};
}

auto LogBuffer::ThreadID() const noexcept -> std::string_view
{
    return hex_id_;
}

LogBuffer::~LogBuffer()
{
    auto& logger = *logger_;
    logger.Unregister(id_);
}
}  // namespace opentxs::internal
