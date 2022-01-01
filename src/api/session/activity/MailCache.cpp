// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "api/session/activity/MailCache.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <iterator>
#include <map>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

#include "core/Worker.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ByteLiterals.hpp"
#include "util/JobCounter.hpp"
#include "util/ScopeGuard.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::session::activity
{
struct MailCache::Imp {
    struct Task {
        Outstanding counter_;
        const OTPasswordPrompt reason_;
        const OTNymID nym_;
        const OTIdentifier item_;
        const StorageBox box_;
        const SimpleCallback done_;
        std::promise<std::string> promise_;

        Task(
            const api::Session& api,
            const identifier::Nym& nym,
            const Identifier& id,
            const StorageBox box,
            const PasswordPrompt& reason,
            SimpleCallback done,
            JobCounter& jobs) noexcept
            : counter_(jobs.Allocate())
            , reason_([&] {
                auto out =
                    api.Factory().PasswordPrompt(reason.GetDisplayString());
                out->SetPassword(reason.Password());

                return out;
            }())
            , nym_(nym)
            , item_(id)
            , box_(box)
            , done_(std::move(done))
            , promise_()
        {
            ++counter_;
        }

        ~Task() = default;

    private:
        Task() = delete;
        Task(const Task&) = delete;
        Task(Task&&) = delete;
        auto operator=(const Task&) -> Task& = delete;
        auto operator=(Task&&) -> Task& = delete;
    };

    auto Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const noexcept -> std::unique_ptr<Message>
    {
        auto output = std::unique_ptr<Message>{};
        auto raw = std::string{};
        auto alias = std::string{};
        const bool loaded =
            api_.Storage().Load(nym.str(), id.str(), box, raw, alias, true);

        if (false == loaded) {
            LogError()(OT_PRETTY_CLASS())("Failed to load message ")(id)
                .Flush();

            return output;
        }

        if (raw.empty()) {
            LogError()(OT_PRETTY_CLASS())("Empty message ")(id).Flush();

            return output;
        }

        output = api_.Factory().Message();

        OT_ASSERT(output);

        if (false ==
            output->LoadContractFromString(String::Factory(raw.c_str()))) {
            LogError()(OT_PRETTY_CLASS())("Failed to deserialize message ")(id)
                .Flush();

            output.reset();
        }

        return output;
    }
    auto ProcessThreadPool(Task* pTask) const noexcept -> void
    {
        OT_ASSERT(nullptr != pTask);

        auto& task = *pTask;
        auto message = std::string{};
        auto postcondition = ScopeGuard{[&] {
            task.promise_.set_value(message);

            {
                auto work = MakeWork(value(WorkType::MessageLoaded));
                work.AddFrame(task.nym_);
                work.AddFrame(task.item_);
                work.AddFrame(task.box_);
                work.AddFrame(message);
                message_loaded_.Send(std::move(work));
            }

            --task.counter_;
            // NOTE make a copy of the callback since executing it will delete
            // the task
            const auto cb{task.done_};

            OT_ASSERT(cb);

            cb();
        }};
        const auto mail = Mail(task.nym_, task.item_, task.box_);

        if (!mail) {
            message = "Error: Unable to load mail item";

            return;
        }

        const auto nym = api_.Wallet().Nym(task.nym_);

        if (false == bool(nym)) {
            message = "Error: Unable to load recipient nym";

            return;
        }

        const auto object =
            api_.Factory().PeerObject(nym, mail->m_ascPayload, task.reason_);

        if (!object) {
            message = "Error: Unable to decrypt message";

            return;
        }

        if (!object->Message()) {
            message = "Unable to display message";

            return;
        }

        message = *object->Message();
    }

    auto CacheText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box,
        const std::string& text) noexcept -> void
    {
        auto key = this->key(nym, id, box);
        auto promise = std::promise<std::string>{};
        promise.set_value(text);
        auto lock = Lock{lock_};
        results_.try_emplace(std::move(key), promise.get_future());
    }
    auto Get(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box,
        const PasswordPrompt& reason) noexcept
        -> std::shared_future<std::string>
    {
        auto lock = Lock{lock_};
        auto key = this->key(nym, id, box);

        if (const auto it = results_.find(key); results_.end() != it) {

            return it->second;
        }

        auto [tIt, newTask] = tasks_.try_emplace(
            key,
            api_,
            nym,
            id,
            box,
            reason,
            [this, key] { finish_task(key); },
            jobs_);

        OT_ASSERT(newTask);

        auto& task = tIt->second;
        const auto [fIt, newFuture] =
            results_.try_emplace(key, task.promise_.get_future());

        OT_ASSERT(newFuture);

        const auto& future = fIt->second;
        fifo_.push(std::move(key));
        const auto sent = api_.Network().Asio().Internal().Post(
            ThreadPool::General,
            [this, pTask = &task] { ProcessThreadPool(pTask); });

        OT_ASSERT(sent);

        return future;
    }

    Imp(const api::Session& api,
        const opentxs::network::zeromq::socket::Publish& messageLoaded) noexcept
        : api_(api)
        , message_loaded_(messageLoaded)
        , lock_()
        , jobs_()
        , cached_bytes_(0)
        , tasks_()
        , results_()
        , fifo_()
    {
    }

    ~Imp() = default;

private:
    const api::Session& api_;
    const opentxs::network::zeromq::socket::Publish& message_loaded_;
    mutable std::mutex lock_;
    JobCounter jobs_;
    std::size_t cached_bytes_;
    std::pmr::map<OTIdentifier, Task> tasks_;
    std::pmr::map<OTIdentifier, std::shared_future<std::string>> results_;
    std::queue<OTIdentifier> fifo_;

    auto key(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const noexcept -> OTIdentifier
    {
        const auto preimage = [&] {
            auto out = space(nym.size() + id.size() + sizeof(box));
            auto it = out.data();
            std::memcpy(it, nym.data(), nym.size());
            std::advance(it, nym.size());
            std::memcpy(it, id.data(), id.size());
            std::advance(it, id.size());
            std::memcpy(it, &box, sizeof(box));

            return out;
        }();
        auto out = api_.Factory().Identifier();
        out->CalculateDigest(reader(preimage));

        return out;
    }

    // NOTE this should only be called from the thread pool
    auto finish_task(const Identifier& key) noexcept -> void
    {
        static constexpr auto limit = 250_MiB;
        static constexpr auto wait = std::chrono::seconds{0};

        auto lock = Lock{lock_};
        tasks_.erase(key);

        {
            const auto& result = results_.at(key).get();
            cached_bytes_ += result.size();
        }

        while (cached_bytes_ > limit) {
            // NOTE don't clear the only cached item, no matter how large
            if (1 >= fifo_.size()) { break; }

            {
                auto& id = fifo_.front();

                {
                    const auto& future = results_.at(id);
                    const auto status = future.wait_for(wait);

                    if (std::future_status::ready != status) { break; }

                    const auto& result = future.get();
                    cached_bytes_ -= result.size();
                }

                results_.erase(id);
            }

            fifo_.pop();
        }
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

MailCache::MailCache(
    const api::Session& api,
    const opentxs::network::zeromq::socket::Publish& messageLoaded) noexcept
    : imp_(std::make_unique<Imp>(api, messageLoaded))
{
}

auto MailCache::CacheText(
    const identifier::Nym& nym,
    const Identifier& id,
    const StorageBox box,
    const std::string& text) noexcept -> void
{
    return imp_->CacheText(nym, id, box, text);
}

auto MailCache::GetText(
    const identifier::Nym& nym,
    const Identifier& id,
    const StorageBox box,
    const PasswordPrompt& reason) noexcept -> std::shared_future<std::string>
{
    return imp_->Get(nym, id, box, reason);
}

auto MailCache::LoadMail(
    const identifier::Nym& nym,
    const Identifier& id,
    const StorageBox& box) const noexcept -> std::unique_ptr<Message>
{
    return imp_->Mail(nym, id, box);
}

MailCache::~MailCache() = default;
}  // namespace opentxs::api::session::activity
