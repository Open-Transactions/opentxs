// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <filesystem>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/storage/file/Mapped.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace otdht
{
class Data;
}  // namespace otdht
}  // namespace network

namespace storage
{
namespace lmdb
{
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class SyncPrivate final : public storage::file::Mapped
{
public:
    auto Load(
        const blockchain::Type chain,
        const block::Height height,
        network::otdht::Data& output) const noexcept -> bool;
    auto Reorg(const blockchain::Type chain, const block::Height height)
        const noexcept -> bool;
    auto Tip(const blockchain::Type chain) const noexcept -> block::Height;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Store(
        const network::otdht::SyncData& items,
        blockchain::Type chain) noexcept -> bool;

    SyncPrivate(
        const api::Session& api,
        storage::lmdb::Database& lmdb,
        const std::filesystem::path& path) noexcept(false);

private:
    using Tips = Map<blockchain::Type, block::Height>;
    using GuardedTips = libguarded::plain_guarded<Tips>;
    using GuardedSocket =
        libguarded::plain_guarded<network::zeromq::socket::Raw>;

    struct Data;

    const api::Session& api_;
    const int tip_table_;
    mutable GuardedTips tips_;
    mutable GuardedSocket checksum_failure_;

    static auto checksum_key() noexcept -> const unsigned char*;

    auto import_genesis(const blockchain::Type chain) noexcept -> void;
    auto reorg(const blockchain::Type chain, const block::Height height)
        const noexcept -> bool;
    auto reorg(
        storage::lmdb::Transaction& tx,
        const blockchain::Type chain,
        const block::Height height) const noexcept -> bool;
};
}  // namespace opentxs::blockchain::database::common
