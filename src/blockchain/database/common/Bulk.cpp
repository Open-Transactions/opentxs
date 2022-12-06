// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Bulk.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <string_view>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/storage/file/Mapped.hpp"

namespace opentxs::blockchain::database::common
{
Bulk::Bulk(
    storage::lmdb::Database& lmdb,
    const std::filesystem::path& path) noexcept(false)
    : Mapped(
          path,
          "blk",
          lmdb,
          Table::Config,
          static_cast<std::size_t>(common::Database::Key::NextBlockAddress),
          {})  // TODO allocator
{
}

Bulk::~Bulk() = default;
}  // namespace opentxs::blockchain::database::common
