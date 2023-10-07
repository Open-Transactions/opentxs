// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::storage::Driver

#include "util/storage/drivers/memdb/MemDB.hpp"  // IWYU pragma: associated

#include <array>
#include <memory>
#include <span>
#include <utility>

#include "internal/util/P0330.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"

namespace opentxs::factory
{
auto StorageMemDB(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>
{
    using ReturnType = storage::driver::MemDB;

    return std::make_unique<ReturnType>(crypto, config);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver
{
auto MemDB::Data::get_bucket(Bucket bucket) noexcept -> Map&
{
    using enum Bucket;

    switch (bucket) {
        case left: {

            return a_;
        }
        case right: {

            return b_;
        }
        default: {
            LogAbort()()("invalid bucket").Abort();
        }
    }
}

auto MemDB::Data::get_bucket(Bucket bucket) const noexcept -> const Map&
{
    return const_cast<Data*>(this)->get_bucket(bucket);
}

auto MemDB::Data::get_search(Search order) const noexcept
    -> std::array<const Map*, 2>
{
    using enum Search;

    if (ltr == order) {

        return {std::addressof(a_), std::addressof(b_)};
    } else {

        return {std::addressof(b_), std::addressof(a_)};
    }
}
}  // namespace opentxs::storage::driver

namespace opentxs::storage::driver
{
using namespace std::literals;

MemDB::MemDB(const api::Crypto& crypto, const storage::Config& config) noexcept
    : Driver(crypto, config)
    , data_()
{
}

auto MemDB::Commit(const Hash& root, Transaction tx, Bucket bucket)
    const noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (false == store(data, tx, bucket)) { return false; }

    data.root_ = root;

    return true;
}

auto MemDB::Description() const noexcept -> std::string_view
{
    return "memory database"sv;
}

auto MemDB::EmptyBucket(Bucket bucket) const noexcept -> bool
{
    data_.lock()->get_bucket(bucket).clear();

    return true;
}

auto MemDB::Load(const Log&, const Hash& key, Search order, Writer& value)
    const noexcept -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    const auto search = data.get_search(order);

    for (const auto* bucket : search) {
        if (auto i = bucket->find(key); bucket->end() != i) {
            copy(i->second, std::move(value));

            return true;
        }
    }

    return false;
}

auto MemDB::LoadRoot() const noexcept -> Hash
{
    return data_.lock_shared()->root_;
}

auto MemDB::Store(Transaction values, Bucket bucket) const noexcept -> bool
{
    return store(*data_.lock(), values, bucket);
}

auto MemDB::store(Data& data, Transaction values, Bucket bucket) const noexcept
    -> bool
{
    if (values.first.empty()) { return true; }

    auto& map = data.get_bucket(bucket);
    const auto count = values.first.size();

    for (auto n = 0_uz; n < count; ++n) {
        const auto& key = values.first[n];
        const auto& value = values.second[n];
        map[key] = value;
    }

    return true;
}
}  // namespace opentxs::storage::driver
