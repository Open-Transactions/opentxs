// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <optional>

#include "blockchain/DownloadTask.hpp"
#include "core/Worker.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::download
{
template <
    typename CRTP,
    typename DownloadType,
    typename FinishedType,
    typename ExtraData = int>
class Manager
{
public:
    using Position = block::Position;
    using Positions = UnallocatedVector<Position>;
    using BatchType = Batch<DownloadType, FinishedType, ExtraData>;
    using Finished = typename BatchType::TaskType::Finished;
    using PreviousData = std::pair<Position, Finished>;
    using Previous = std::optional<PreviousData>;

    auto Heartbeat() noexcept -> void
    {
        auto& me = downcast();

        me.pipeline_.Push(MakeWork(OT_ZMQ_HEARTBEAT_SIGNAL));
    }
    auto Reset(const Position& position, Finished&& previous) noexcept -> void
    {
        LogVerbose()(OT_PRETTY_CLASS())("resetting ")(log_)(" to height")(
            position.height_)
            .Flush();
        auto lock = Lock{dm_lock_};
        dm_previous_ = std::move(previous);
        dm_done_ = position;
        dm_known_ = position;
        buffer_.clear();
        next_ = 0;
        downcast().update_tip(position, dm_previous_.get());
    }
    auto Start() noexcept { enabled_ = true; }

    Manager(const Manager& rhs) = delete;
    Manager(Manager&& rhs) = delete;
    auto operator=(const Manager& rhs) -> Manager& = delete;
    auto operator=(Manager&& rhs) -> Manager& = delete;

protected:
    using DownloadedData = typename BatchType::Vector;
    using TaskType = typename BatchType::TaskType;

    auto buffer_size() const noexcept
    {
        auto lock = Lock{dm_lock_};

        return buffer_.size();
    }
    auto dm_enabled() const noexcept -> bool { return enabled_; }
    // WARNING Call known() and update_position() from the same thread.
    auto known() const noexcept { return dm_known_; }

    auto allocate_batch(ExtraData extra = {}) noexcept -> BatchType
    {
        auto lock = Lock{dm_lock_};

        if (caught_up(lock)) {
            LogTrace()(OT_PRETTY_CLASS())(log_)(" caught up").Flush();

            return {};
        }

        OT_ASSERT(0 < buffer_.size());

        const auto size = [&] {
            const auto unallocated = this->unallocated(lock);
            const auto batch = downcast().batch_size(unallocated);

            return std::min<std::size_t>(unallocated, batch);
        }();

        LogTrace()(OT_PRETTY_CLASS())(size)(" ")(log_)(" items to download")
            .Flush();

        if (0 == size) { return {}; }

        auto output = BatchType{
            [&] {
                auto output = typename BatchType::Vector{};
                auto i = buffer_.begin();
                std::advance(i, next_);

                for (; i != buffer_.end(); ++i) {
                    const auto& task = *i;

                    if (auto expected{State::New};
                        false == task->state_.compare_exchange_strong(
                                     expected, State::Downloading)) {
                        if (0 == output.size()) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    LogTrace()(OT_PRETTY_CLASS())("queueing ")(
                        log_)(" item at height ")(task->position_.height_)(
                        " for download")
                        .Flush();
                    output.emplace_back(task);

                    if (output.size() == size) { break; }
                }

                return output;
            }(),
            [this](const auto& batch) { finish_downloading(batch); },
            std::move(extra)};

        if (const auto allocated = output.data_.size(); 0 == allocated) {

            return {};
        } else {
            next_ += allocated;
        }

        OT_ASSERT(next_ <= buffer_.size());

        return output;
    }
    auto run_if_enabled() noexcept -> void
    {
        if (enabled_) { downcast().do_work(); }
    }
    auto state_machine() noexcept -> bool
    {
        auto lock = Lock{dm_lock_};

        return state_machine(lock);
    }
    auto update_position(
        Positions&& positions,
        ExtraData extra,
        Previous prior = std::nullopt) noexcept
    {
        if (0 == positions.size()) { return; }

        if (prior) {
            OT_ASSERT(prior->first.height_ <= positions.front().height_);
        }

        auto lock = Lock{dm_lock_};
        {
            const auto& start = positions.front();

            if (dm_known_.height_ >= start.height_) {
                auto next = -1_z;
                auto lastGoodTask = TaskPtr{};

                for (auto i{buffer_.begin()}; i != buffer_.end(); ++i) {
                    const auto& position = (*i)->position_;

                    if (position.height_ < start.height_) {
                        lastGoodTask = *i;
                        ++next;

                        continue;
                    }

                    buffer_.erase(i, buffer_.end());

                    break;
                }

                {
                    using Data = std::tuple<Position, Finished, std::size_t>;

                    auto data = [&]() -> Data {
                        if (0 <= next) {
                            OT_ASSERT(lastGoodTask);

                            const auto& task = *lastGoodTask;

                            return {
                                task.position_,
                                task.output_,
                                static_cast<std::size_t>(next)};
                        } else {
                            OT_ASSERT(prior.has_value());
                            OT_ASSERT(0 == buffer_.size());

                            auto previous = prior.value();

                            return {
                                std::move(previous.first),
                                std::move(previous.second),
                                0};
                        }
                    }();

                    auto& [position, finished, index] = data;

                    if (dm_done_ > start) {
                        dm_update_tip(
                            lock, std::move(position), std::move(finished));
                    }

                    next_ = std::min(next_, index);
                }
            }
        }

        auto previous =
            (0 == buffer_.size()) ? dm_previous_ : buffer_.back()->output_;

        for (auto& position : positions) {
            if ((0 == max_queue_) || (buffer_.size() < max_queue_)) {
                auto& task = buffer_.emplace_back(std::make_shared<TaskType>(
                    std::move(position),
                    std::move(previous),
                    log_.data(),
                    extra));
                previous = task->output_;
                downcast().check_task(*task);
            } else {
                break;
            }
        }

        dm_known_ = buffer_.back()->position_;

        OT_ASSERT(dm_done_.height_ <= dm_known_.height_);

        downcast().trigger_state_machine();
    }

    Manager(
        const Position& position,
        Finished&& previous,
        const UnallocatedCString& log,
        const std::size_t max,
        const std::size_t min) noexcept
        : log_(log)
        , max_queue_(max)
        , dm_lock_()
        , dm_previous_(std::move(previous))
        , dm_done_(position)
        , dm_known_(position)
        , buffer_()
        , next_(0)
        , enabled_(false)
    {
    }

    ~Manager() = default;

private:
    struct BatchData {
        Position first_;
        Position last_;

        BatchData(Position first, Position last) noexcept
            : first_(std::move(first))
            , last_(std::move(last))
        {
        }
    };

    using TaskPtr = std::shared_ptr<TaskType>;
    using Buffer = UnallocatedDeque<TaskPtr>;

    const UnallocatedCString log_;
    const std::size_t max_queue_;
    mutable std::mutex dm_lock_;
    Finished dm_previous_;
    Position dm_done_;
    Position dm_known_;
    Buffer buffer_;
    std::size_t next_;
    std::atomic_bool enabled_;

    // Functions to implement in child class:
    //
    // std::size_t batch_size(std::size_t outstanding): calculate batch size
    //                                                  based on number of
    //                                                  outstanding blocks
    // void batch_ready(): notify interested parties that a batch of work is
    //                     available
    // void check_task(TaskType&): optionally look up task to see if it is
    //                             already downloaded
    // void trigger_state_machine(): cause the state_machine() function to be
    //                               called from a different thread
    // void queue_processing(DownloadedData&& data): process downloaded data
    //                                               possibly in a different
    //                                               thread
    // void update_tip(const Position&, const FinishedType&): perform chain tip
    //                                                        update accounting

    inline auto caught_up(const Lock&) const noexcept -> bool
    {
        return dm_done_ == dm_known_;
    }
    inline auto unallocated(const Lock&) const noexcept -> std::size_t
    {
        const auto outstanding =
            static_cast<std::size_t>(dm_known_.height_ - dm_done_.height_);

        OT_ASSERT(next_ <= outstanding);
        OT_ASSERT(next_ <= buffer_.size());

        return outstanding - next_;
    }

    inline auto downcast() noexcept -> CRTP&
    {
        return static_cast<CRTP&>(*this);
    }
    auto finish_downloading(const BatchType& batch) noexcept -> void
    {
        auto lock = Lock{dm_lock_};
        // Make sure all tasks in batch actually got downloaded
        const auto& data = batch.data_;

        OT_ASSERT(0 < data.size());

        const auto& first = data.front();
        auto index = 0_uz;
        auto b =
            std::find_if(buffer_.begin(), buffer_.end(), [&](const auto& task) {
                ++index;

                return task->position_ == first->position_;
            });

        if (buffer_.end() == b) { return; }

        OT_ASSERT(0 < index);

        --index;

        for (auto i{data.cbegin()}; (i != data.cend()) && (b != buffer_.end());
             ++i, ++b, ++index) {
            const auto& bufferTask = *b;
            const auto& batchTask = *i;

            if (bufferTask.get() != batchTask.get()) { break; }

            auto& task = *batchTask;
            auto& state = task.state_;
            auto expect = State::Downloaded;

            if (state.compare_exchange_strong(expect, State::Update)) {
                state.store(State::Downloaded);
            } else {
                next_ = std::min(next_, index);

                if (State::Downloading == expect) {
                    state.compare_exchange_strong(expect, State::New);
                }
            }
        }

        downcast().trigger_state_machine();
    }
    auto state_machine(const Lock& lock) noexcept -> bool
    {
        if (caught_up(lock)) { return false; }

        OT_ASSERT(0 < buffer_.size());

        {
            auto end{buffer_.end()};
            auto first{end};
            auto last{end};
            auto toDelete = 0_uz;
            auto index = 0_uz;

            for (auto i{buffer_.begin()}; i != end; ++i, ++index) {
                const auto& task = **i;
                const auto state = task.state_.load();

                if (State::Processed != state) {
                    if (State::New == state) { next_ = std::min(next_, index); }

                    break;
                }

                if (end == first) { first = i; }

                last = i;
                ++toDelete;
            }

            if (0 < toDelete) {
                OT_ASSERT(last != end);

                {
                    const auto& task = *last;
                    dm_update_tip(
                        lock,
                        Position{task->position_},
                        Finished{task->output_});
                }

                buffer_.erase(first, std::next(last));

                if (next_ >= toDelete) {
                    next_ -= toDelete;
                } else {
                    next_ = 0;
                }
            }
        }

        OT_ASSERT(dm_done_.height_ <= dm_known_.height_);

        auto process = DownloadedData{};

        for (auto& pTask : buffer_) {
            auto& task = *pTask;
            auto& state = task.state_;
            auto expected{State::Downloaded};

            if (false ==
                state.compare_exchange_strong(expected, State::Processing)) {
                break;
            }

            process.emplace_back(pTask);
        }

        downcast().queue_processing(std::move(process));

        if (caught_up(lock)) { return false; }

        if (0u < unallocated(lock)) { downcast().batch_ready(); }

        return true;
    }
    auto dm_update_tip(const Lock&, Position&& pos, Finished&& data) noexcept
        -> void
    {
        downcast().update_tip(pos, data.get());
        dm_done_ = std::move(pos);
        dm_previous_ = std::move(data);
    }
};
}  // namespace opentxs::blockchain::download
