// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Driver
{
public:
    virtual auto Description() const noexcept -> std::string_view = 0;
    virtual auto Load(
        const Log& logger,
        const Hash& key,
        Search order,
        Writer& value) const noexcept -> bool = 0;
    virtual auto LoadRoot() const noexcept -> Hash = 0;

    virtual auto Commit(const Hash& root, Transaction data, Bucket bucket)
        const noexcept -> bool = 0;
    virtual auto EmptyBucket(Bucket bucket) const noexcept -> bool = 0;
    virtual auto Store(Transaction data, Bucket bucket) const noexcept
        -> bool = 0;

    virtual ~Driver() = default;

    Driver(const Driver&) = delete;
    Driver(Driver&&) = delete;
    auto operator=(const Driver&) -> Driver& = delete;
    auto operator=(Driver&&) -> Driver& = delete;

protected:
    Driver() = default;
};
}  // namespace opentxs::storage
