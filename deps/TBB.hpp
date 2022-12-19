// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/Thread.hpp"

#if __has_include(<oneapi/tbb.h>)
#include <oneapi/tbb.h>  // IWYU pragma: export
#elif __has_include(<tbb/tbb.h>)
#include <tbb/tbb.h>  // IWYU pragma: export
#else
#error can not find tbb headers
#endif

namespace opentxs::tbb
{
#if __has_include(<oneapi/tbb.h>)
using namespace ::oneapi::tbb;

template <typename F>
auto fire_and_forget(F&& f) -> void
{
    auto arena = task_arena{task_arena::attach{}};
    arena.enqueue(std::forward<F>(f));
}

struct Options {
private:
    global_control stack_size_{
        global_control::thread_stack_size,
        thread_pool_stack_size_};
};
#else
using namespace ::tbb;

template <typename F>
class lambda_task final : public task
{
public:
    lambda_task(F&& f)
        : task_(std::move(f))
    {
    }

private:
    F task_;

    auto execute() -> task* final
    {
        std::invoke(task_);

        return nullptr;
    }
};

template <typename F>
auto fire_and_forget(F&& f) -> void
{
    task::enqueue(*new (task::allocate_root()) lambda_task<F>(std::move(f)));
}

struct Options {
private:
    task_scheduler_init stack_size_{
        task_scheduler_init::default_num_threads(),
        thread_pool_stack_size_};
};
#endif
}  // namespace opentxs::tbb
