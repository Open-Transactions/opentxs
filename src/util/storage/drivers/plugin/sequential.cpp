// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/plugin/Plugin.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <numeric>
#include <ranges>

#include "opentxs/util/Log.hpp"

namespace opentxs::storage::driver::implementation
{
auto Plugin::empty_bucket(Bucket bucket) const noexcept -> Results
{
    auto out = make_results();
    auto store = [=](auto& row) {
        auto& [driver, result] = row;
        result = driver->EmptyBucket(bucket);

        if (false == result) {
            LogError()()("error emptying bucket in ")(driver->Description())(
                " driver")
                .Flush();
        }
    };
    std::ranges::for_each(out, store);

    return out;
}

auto Plugin::find_best(Results& in) noexcept -> Results::iterator
{
    return std::max_element(in.begin(), in.end(), Plugin::compare_results);
}

auto Plugin::evaluate(const Results& in) noexcept -> std::uint64_t
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-member-function"
    struct Visitor {
        auto operator()(std::uint64_t lhs, const Results::value_type& rhs)
            const noexcept -> std::uint64_t
        {
            return lhs + rhs.second;
        }
        auto operator()(const Results::value_type& lhs, std::uint64_t rhs)
            const noexcept -> std::uint64_t
        {
            return lhs.second + rhs;
        }
        auto operator()(
            const Results::value_type& lhs,
            const Results::value_type& rhs) const noexcept -> std::uint64_t
        {
            return lhs.second + rhs.second;
        }
        auto operator()(std::uint64_t lhs, std::uint64_t rhs) const noexcept
            -> std::uint64_t
        {
            return lhs + rhs;
        }
    };
#pragma GCC diagnostic pop

    return std::reduce(in.begin(), in.end(), 0, Visitor{});
}

auto Plugin::scan(const Log& log) const noexcept -> Results
{
    auto out = make_results();
    auto check = [&log](auto& row) { check_revision(log, row); };
    std::ranges::for_each(out, check);

    return out;
}

auto Plugin::commit(const Hash& root, Transaction data, Bucket bucket)
    const noexcept -> Results
{
    auto out = make_results();
    auto save = [&](auto& row) {
        auto& [driver, result] = row;
        result = driver->Commit(root, data, bucket);

        if (false == result) {
            LogError()()("error committing to ")(driver->Description())(
                " driver")
                .Flush();
        }
    };
    std::ranges::for_each(out, save);

    return out;
}

auto Plugin::store(Transaction data, Bucket bucket) const noexcept -> Results
{
    auto out = make_results();
    auto save = [&](auto& row) {
        auto& [driver, result] = row;
        result = driver->Store(data, bucket);

        if (false == result) {
            LogError()()("error storing to ")(driver->Description())(" driver")
                .Flush();
        }
    };
    std::ranges::for_each(out, save);

    return out;
}

auto Plugin::synchronize_drivers(const Hash& hash, Bucket bucket) noexcept
    -> void
{
    auto sync = [&, this](auto* driver) { synchronize(hash, bucket, *driver); };
    std::ranges::for_each(drivers_, sync);
}
}  // namespace opentxs::storage::driver::implementation
