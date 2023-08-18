// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/plugin/Plugin.hpp"  // IWYU pragma: associated

#include <algorithm>  // IWYU pragma: keep
#include <execution>
#include <numeric>

#include "internal/util/LogMacros.hpp"

namespace opentxs::storage::driver::implementation
{
auto Plugin::empty_bucket(Bucket bucket) const noexcept -> Results
{
    auto out = make_results();
    auto store = [=, this](auto& row) {
        auto& [driver, result] = row;
        result = driver->EmptyBucket(bucket);

        if (false == result) {
            LogError()(OT_PRETTY_CLASS())("error emptying bucket in ")(
                driver->Description())(" driver")
                .Flush();
        }
    };
    using namespace std::execution;
    std::for_each(par, out.begin(), out.end(), store);

    return out;
}

auto Plugin::find_best(Results& in) noexcept -> Results::iterator
{
    using namespace std::execution;

    return std::max_element(par, in.begin(), in.end(), Plugin::compare_results);
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
    using namespace std::execution;

    return std::reduce(par, in.begin(), in.end(), 0, Visitor{});
}

auto Plugin::scan(const Log& log) const noexcept -> Results
{
    auto out = make_results();
    auto check = [&log](auto& row) { check_revision(log, row); };
    using namespace std::execution;
    std::for_each(par, out.begin(), out.end(), check);

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
            LogError()(OT_PRETTY_CLASS())("error committing to ")(
                driver->Description())(" driver")
                .Flush();
        }
    };
    using namespace std::execution;
    std::for_each(par, out.begin(), out.end(), save);

    return out;
}

auto Plugin::store(Transaction data, Bucket bucket) const noexcept -> Results
{
    auto out = make_results();
    auto save = [&, this](auto& row) {
        auto& [driver, result] = row;
        result = driver->Store(data, bucket);

        if (false == result) {
            LogError()(OT_PRETTY_CLASS())("error storing to ")(
                driver->Description())(" driver")
                .Flush();
        }
    };
    using namespace std::execution;
    std::for_each(par, out.begin(), out.end(), save);

    return out;
}

auto Plugin::synchronize_drivers(const Hash& hash, Bucket bucket) noexcept
    -> void
{
    auto sync = [&, this](auto* driver) { synchronize(hash, bucket, *driver); };
    using namespace std::execution;
    std::for_each(par, drivers_.begin(), drivers_.end(), sync);
}
}  // namespace opentxs::storage::driver::implementation
