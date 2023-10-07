// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/plugin/Plugin.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>

#include "TBB.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::storage::driver::implementation
{
auto Plugin::empty_bucket(Bucket bucket) const noexcept -> Results
{
    auto out = make_results();
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, out.size()}, [&](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& [driver, result] = out[i];
                result = driver->EmptyBucket(bucket);

                if (false == result) {
                    LogError()()("error emptying bucket in ")(
                        driver->Description())(" driver")
                        .Flush();
                }
            }
        });

    return out;
}

auto Plugin::evaluate(const Results& in) noexcept -> std::uint64_t
{
    using Range = tbb::blocked_range<const Results::value_type*>;

    return tbb::parallel_reduce(
        Range{in.data(), std::next(in.data(), in.size())},
        0,
        [](const Range& r, std::uint64_t init) {
            for (const auto& i : r) { init += i.second; }

            return init;
        },
        [](std::uint64_t lhs, std::uint64_t rhs) { return lhs + rhs; });
}

// NOTE TBB doesn't have this algorithm
auto Plugin::find_best(Results& in) noexcept -> Results::iterator
{
    return std::max_element(in.begin(), in.end(), Plugin::compare_results);
}

auto Plugin::scan(const Log& log) const noexcept -> Results
{
    auto out = make_results();
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, out.size()}, [&](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                check_revision(log, out[i]);
            }
        });

    return out;
}

auto Plugin::commit(const Hash& root, Transaction data, Bucket bucket)
    const noexcept -> Results
{
    auto out = make_results();
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, out.size()}, [&](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& [driver, result] = out[i];
                result = driver->Commit(root, data, bucket);

                if (false == result) {
                    LogError()()("error committing to ")(driver->Description())(
                        " driver")
                        .Flush();
                }
            }
        });

    return out;
}

auto Plugin::store(Transaction data, Bucket bucket) const noexcept -> Results
{
    auto out = make_results();
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, out.size()}, [&](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                auto& [driver, result] = out[i];
                result = driver->Store(data, bucket);

                if (false == result) {
                    LogError()()("error storing to ")(driver->Description())(
                        " driver")
                        .Flush();
                }
            }
        });

    return out;
}

auto Plugin::synchronize_drivers(const Hash& hash, Bucket bucket) noexcept
    -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, drivers_.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                synchronize(hash, bucket, *drivers_[i]);
            }
        });
}
}  // namespace opentxs::storage::driver::implementation
