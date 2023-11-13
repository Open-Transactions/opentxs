// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <array>
#include <shared_mutex>
#include <string_view>

#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Types.hpp"
#include "util/storage/drivers/Driver.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage

class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
class MemDB final : public storage::implementation::Driver
{
public:
    auto Description() const noexcept -> std::string_view final;
    auto Load(const Log& logger, const Hash& key, Search order, Writer& value)
        const noexcept -> bool final;
    auto LoadRoot() const noexcept -> Hash final;

    auto Commit(const Hash& root, Transaction data, Bucket bucket)
        const noexcept -> bool final;
    auto EmptyBucket(Bucket bucket) const noexcept -> bool final;
    auto Store(Transaction data, Bucket bucket) const noexcept -> bool final;

    MemDB(const api::Crypto& crypto, const storage::Config& config) noexcept;
    MemDB() = delete;
    MemDB(const MemDB&) = delete;
    MemDB(MemDB&&) = delete;
    auto operator=(const MemDB&) -> MemDB& = delete;
    auto operator=(MemDB&&) -> MemDB& = delete;

    ~MemDB() final = default;

private:
    struct Data {
        using Map = UnallocatedMap<Hash, UnallocatedCString>;

        Hash root_{};

        auto get_bucket(Bucket bucket) const noexcept -> const Map&;
        auto get_search(Search order) const noexcept
            -> std::array<const Map*, 2>;

        auto get_bucket(Bucket bucket) noexcept -> Map&;

    private:
        Map a_{};
        Map b_{};
    };

    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    mutable GuardedData data_;

    auto store(Data& data, Transaction values, Bucket bucket) const noexcept
        -> bool;
};
}  // namespace opentxs::storage::driver
