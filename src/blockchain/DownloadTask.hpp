// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::blockchain::download
{
enum class State : std::uint8_t {
    New,
    Downloading,
    Downloaded,
    Processing,
    Processed,
    Update,
};

template <
    typename DownloadType,
    typename FinishedType,
    typename ExtraData = int>
class Task
{
public:
    using Position = block::Position;

    const Position position_;
    const ExtraData extra_;
    mutable std::atomic<State> state_;

private:
    mutable std::promise<DownloadType> download_;
    mutable std::promise<FinishedType> process_;

public:
    using Downloaded = std::shared_future<DownloadType>;
    using Finished = std::shared_future<FinishedType>;

    mutable Downloaded data_;
    mutable Finished output_;
    const Finished previous_;

    auto download(
        DownloadType&& data,
        std::optional<ExtraData> check = std::nullopt) const noexcept -> bool
    {
        auto expect{State::Downloading};

        if (false == state_.compare_exchange_strong(expect, State::Update)) {
            LogVerbose()(log_)(" download job ")(id_)(" for block ")(
                position_.second->asHex())(" at height ")(position_.first)(
                " failed state check")
                .Flush();

            return false;
        }

        if (check.has_value() && (check.value() != extra_)) {
            LogVerbose()(log_)(" download job ")(id_)(" for block ")(
                position_.second->asHex())(" at height ")(position_.first)(
                " failed metadata check")
                .Flush();
            state_.store(expect);

            return false;
        }

        try {
            download_.set_value(std::move(data));
            state_.store(State::Downloaded);
            LogTrace()(log_)(" download job ")(id_)(" for block ")(
                position_.second->asHex())(" at height ")(position_.first)(
                " completed")
                .Flush();
        } catch (const std::exception& e) {
            LogError()(log_)(" download job ")(id_)(" for block ")(
                position_.second->asHex())(" at height ")(position_.first)(
                " failed: ")(e.what())
                .Flush();

            OT_FAIL;
        }

        return true;
    }
    auto process(FinishedType&& data) const noexcept -> void
    {
        auto expect{State::Processing};

        if (false == state_.compare_exchange_strong(expect, State::Update)) {
            return;
        }

        try {
            process_.set_value(std::move(data));
            state_.store(State::Processed);
            LogTrace()(log_)(" process job ")(id_)(" for block ")(
                position_.second->asHex())(" at height ")(position_.first)(
                " completed")
                .Flush();
        } catch (const std::exception& e) {
            LogError()(log_)(" process job ")(id_)(" for block ")(
                position_.second->asHex())(" at height ")(position_.first)(
                " failed: ")(e.what())
                .Flush();

            OT_FAIL;
        }
    }
    auto process(std::exception_ptr p) const noexcept -> void
    {
        auto expect{State::Processing};

        if (false == state_.compare_exchange_strong(expect, State::Update)) {
            return;
        }

        LogVerbose()("Redownloading ")(log_)(" job ")(id_)(" for block ")(
            position_.second->asHex())(" at height ")(position_.first)
            .Flush();
        {
            auto promise = std::promise<DownloadType>{};
            download_.swap(promise);
        }
        {
            try {
                process_.set_exception(std::move(p));
            } catch (...) {
            }

            auto promise = std::promise<FinishedType>{};
            process_.swap(promise);
        }
        data_ = download_.get_future();
        state_.store(State::New);
    }
    // WARNING if redownloading is necessary call this function prior to and
    // instead of calling calling process()
    auto redownload() const noexcept -> void
    {
        auto expect{State::Processing};

        if (false == state_.compare_exchange_strong(expect, State::Update)) {
            return;
        }

        LogVerbose()("Redownloading ")(log_)(" job ")(id_)(" for block ")(
            position_.second->asHex())(" at height ")(position_.first)
            .Flush();
        auto promise = std::promise<DownloadType>{};
        download_.swap(promise);
        data_ = download_.get_future();
        state_.store(State::New);
    }

    Task(
        Position&& position,
        Finished&& previous,
        const char* log,
        ExtraData extra = {}) noexcept
        : position_(std::move(position))
        , extra_(std::move(extra))
        , state_(State::New)
        , download_()
        , process_()
        , data_(download_.get_future())
        , output_(process_.get_future())
        , previous_(std::move(previous))
        , id_(id())
        , log_(std::move(log))
    {
    }

    ~Task() = default;

private:
    const int id_;
    const char* log_;

    static auto id() noexcept -> int
    {
        static auto counter = std::atomic_int{-1};

        return ++counter;
    }

    Task(const Task& rhs) = delete;
    Task(Task&& rhs) = delete;
    auto operator=(const Task& rhs) -> Task& = delete;
    auto operator=(Task&& rhs) -> Task& = delete;
};

template <
    typename DownloadType,
    typename FinishedType,
    typename ExtraData = int>
class Batch
{
public:
    using ID = std::int64_t;
    using TaskType = Task<DownloadType, FinishedType, ExtraData>;
    using Vector = std::pmr::vector<std::shared_ptr<TaskType>>;
    using Callback = std::function<void(const Batch&)>;
    using Position = typename TaskType::Position;

    const ID id_;
    const Vector data_;
    const ExtraData extra_;
    mutable std::atomic<std::size_t> downloaded_;

    operator bool() const noexcept { return 0 < data_.size(); }

    auto Elapsed() const noexcept { return Clock::now() - last_activity_; }
    auto isDownloaded() const noexcept -> bool
    {
        const auto size = data_.size();

        return (0 < size) && (downloaded_.load() == size);
    }

    auto Download(
        const Position& item,
        DownloadType&& data,
        std::optional<ExtraData> check = std::nullopt) -> bool
    {
        if (data_.at(index_.at(item))->download(std::move(data), check)) {
            ++downloaded_;
            last_activity_ = Clock::now();

            return true;
        }

        return false;
    }
    // NOTE this function is intended for unit testing. Normally you should just
    // allow the Batch to fall out of scope instead of calling this explicitly.
    auto Finish() noexcept -> void
    {
        try {
            if (cb_ && (0 < data_.size())) { cb_(*this); }
        } catch (...) {
        }

        cb_ = {};
        const_cast<Index&>(index_).clear();
        const_cast<Vector&>(data_).clear();
    }

    Batch() noexcept
        : Batch(-1, {}, {}, {})
    {
    }
    Batch(
        const ID id,
        Vector&& data,
        Callback cb = {},
        ExtraData&& extra = {}) noexcept
        : id_(id)
        , data_(std::move(data))
        , extra_(std::move(extra))
        , downloaded_(0)
        , cb_(cb)
        , index_(index(data_))
        , started_(Clock::now())
        , last_activity_(started_)
    {
    }
    Batch(Batch&& rhs) noexcept
        : id_(rhs.id_)
        , data_(std::move(const_cast<Vector&>(rhs.data_)))
        , extra_(std::move(const_cast<ExtraData&>(rhs.extra_)))
        , downloaded_(rhs.downloaded_.load())
        , cb_(rhs.cb_)
        , index_(std::move(const_cast<Index&>(rhs.index_)))
        , started_(rhs.started_)
        , last_activity_(rhs.last_activity_)
    {
        rhs.cb_ = {};
    }
    auto operator=(Batch&& rhs) noexcept -> Batch&
    {
        if (&rhs != this) {
            std::swap(const_cast<ID&>(id_), const_cast<ID&>(rhs.id_));
            std::swap(
                const_cast<Vector&>(data_), const_cast<Vector&>(rhs.data_));
            std::swap(
                const_cast<ExtraData&>(extra_),
                const_cast<ExtraData&>(rhs.extra_));
            {
                auto val = downloaded_.load();
                downloaded_.store(rhs.downloaded_.load());
                rhs.downloaded_.store(val);
            }
            std::swap(cb_, rhs.cb_);
            std::swap(
                const_cast<Index&>(index_), const_cast<Index&>(rhs.index_));
            std::swap(started_, rhs.started_);
            std::swap(last_activity_, rhs.last_activity_);
        }

        return *this;
    }

    ~Batch() { Finish(); }

private:
    using Index = std::pmr::map<Position, std::size_t>;

    Callback cb_;
    const Index index_;
    Time started_;
    Time last_activity_;

    static auto index(const Vector& in) noexcept -> Index
    {
        auto output = Index{};
        auto count = std::size_t{};

        for (const auto& task : in) {
            output.emplace(task->position_, count++);
        }

        return output;
    }

    Batch(const Batch&) = delete;
    auto operator=(const Batch&) -> Batch& = delete;
};
}  // namespace opentxs::blockchain::download
