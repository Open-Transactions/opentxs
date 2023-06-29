// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Root.hpp"  // IWYU pragma: associated

#include <StorageRoot.pb.h>
#include <ctime>
#include <functional>
#include <type_traits>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/ScopeGuard.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Tree.hpp"

namespace opentxs::storage
{
Root::GC::GC(
    const api::network::Asio& asio,
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& driver,
    const std::int64_t interval) noexcept
    : asio_(asio)
    , crypto_(crypto)
    , factory_(factory)
    , driver_(driver)
    , interval_(interval)
    , lock_()
    , running_(Flag::Factory(false))
    , resume_(Flag::Factory(false))
    , root_(Node::BLANK_HASH)
    , last_(static_cast<std::int64_t>(std::time(nullptr)))
    , promise_()
    , future_(promise_.get_future())
{
    // TODO hold shared_ptr<api::Session> as a member variable
    promise_.set_value(true);
}

auto Root::GC::Cleanup() noexcept -> void { future_.get(); }

auto Root::GC::Check(const UnallocatedCString root) noexcept -> CheckState
{
    if (0 == interval_) {
        LogVerbose()(OT_PRETTY_CLASS())("Garbage collection disabled").Flush();

        return CheckState::Skip;
    }

    if (running_.get()) {
        LogVerbose()(OT_PRETTY_CLASS())("Garbage collection in progress")
            .Flush();

        return CheckState::Skip;
    }

    const auto run = [this] {
        promise_ = {};
        future_ = promise_.get_future();
        running_->On();
        resume_->On();
    };
    auto lock = Lock{lock_};

    if (resume_.get()) {
        run();

        return CheckState::Resume;
    } else {
        const std::uint64_t time = std::time(nullptr);
        const bool intervalExceeded = ((time - last_.load()) > interval_);

        if (intervalExceeded) {
            run();
            root_ = std::move(root);

            return CheckState::Start;
        } else {

            return CheckState::Skip;
        }
    }
}

auto Root::GC::collect_garbage(
    const bool from,
    const Driver* to,
    const SimpleCallback done) noexcept -> void
{
    LogVerbose()(OT_PRETTY_CLASS())("Beginning garbage collection.").Flush();
    auto success{false};
    auto postcondition = ScopeGuard{[&] { promise_.set_value(success); }};
    auto temp = storage::Tree{crypto_, factory_, driver_, root_};
    success = temp.Migrate(*to);

    if (success) {
        driver_.EmptyBucket(from);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("Garbage collection failed").Flush();
    }

    {
        auto lock = Lock{lock_};
        running_->Off();
        resume_->Off();
        root_ = "";
        last_.store(std::time(nullptr));
    }

    OT_ASSERT(done);

    done();
    LogVerbose()(OT_PRETTY_CLASS())("Finished garbage collection.").Flush();
}

auto Root::GC::Init(
    const UnallocatedCString& root,
    bool resume,
    std::uint64_t last) noexcept -> void
{
    auto lock = Lock{lock_};
    root_ = root;
    running_->Off();
    resume_->Set(resume);
    last_.store(last);
}

auto Root::GC::Run(
    const bool from,
    const Driver& to,
    SimpleCallback cb) noexcept -> bool
{
    RunJob([from, op = std::move(cb), me = shared_from_this(), driver = &to] {
        me->collect_garbage(from, driver, std::move(op));
    });

    return true;
}

auto Root::GC::Serialize(proto::StorageRoot& out) const noexcept -> void
{
    auto lock = Lock{lock_};
    out.set_lastgc(last_.load());
    out.set_gc(running_.get());
    out.set_gcroot(root_);
}

Root::GC::~GC() { Cleanup(); }
}  // namespace opentxs::storage
