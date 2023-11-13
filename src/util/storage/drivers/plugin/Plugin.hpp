// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <utility>

#include "internal/util/storage/Types.hpp"
#include "internal/util/storage/drivers/Plugin.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "opentxs/util/storage/Types.hpp"
#include "util/storage/drivers/plugin/PendingWrite.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
namespace tree
{
struct GCParams;
}  // namespace tree

class Config;
}  // namespace storage

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver::implementation
{
class Plugin final : virtual public driver::Plugin
{
public:
    auto EmptyBucket(Bucket bucket) const noexcept -> bool final;
    auto Load(
        const Hash& key,
        ErrorReporting checking,
        Writer&& value,
        const Driver* driver) const noexcept -> bool final;
    auto LoadRoot() const noexcept -> Hash final;
    auto Primary() const noexcept -> const storage::Driver& final;
    auto Store(ReadView value, Hash& key) const noexcept -> bool final;
    auto StoreRoot(const Hash& hash) const noexcept -> bool final;

    auto DoGC(const tree::GCParams& params) noexcept -> bool final;
    auto FindBestRoot() noexcept -> Hash final;
    auto InitBackup() -> void final;
    auto InitEncryptedBackup(crypto::symmetric::Key& key) -> void final;
    auto Primary() noexcept -> storage::Driver& final;

    Plugin(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const std::atomic<Bucket>& primaryBucket,
        const storage::Config& config);
    Plugin() = delete;
    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    auto operator=(const Plugin&) -> Plugin& = delete;
    auto operator=(Plugin&&) -> Plugin& = delete;

    ~Plugin() final;

private:
    using Results = Vector<std::pair<storage::Driver*, std::uint64_t>>;

    const api::Crypto& crypto_;
    const api::session::Factory& factory_;
    const std::atomic<Bucket>& primary_bucket_;
    const storage::Config& config_;
    std::unique_ptr<storage::Driver> primary_driver_;
    Vector<std::unique_ptr<storage::Driver>> backup_drivers_;
    Vector<storage::Driver*> drivers_;
    crypto::symmetric::Key null_;
    mutable libguarded::plain_guarded<Hash> root_;
    mutable libguarded::plain_guarded<PendingWrite> write_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    static auto check_revision(const Log& log, Results::value_type&) noexcept
        -> void;
    static auto compare_results(
        const Results::value_type& lhs,
        const Results::value_type& rhs) noexcept -> bool;
    static auto evaluate(const Results& in) noexcept -> std::uint64_t;
    static auto evaluate(std::uint64_t results) noexcept -> bool;
    static auto find_best(Results& in) noexcept -> Results::iterator;
    static auto find_best(Results& lhs, Results::iterator rhs) noexcept
        -> std::size_t;

    auto calculate_hash(ReadView data, Hash& out) const noexcept -> bool;
    auto commit(const Hash& root, Transaction data, Bucket bucket)
        const noexcept -> Results;
    auto empty_bucket(Bucket bucket) const noexcept -> Results;
    auto make_results() const noexcept -> Results;
    auto scan(const Log& log) const noexcept -> Results;

    auto Cleanup() -> void;
    auto Cleanup_Plugin() -> void;
    auto init(
        const UnallocatedCString& primary,
        std::unique_ptr<storage::Driver>& plugin) -> void;
    auto init_fs(std::unique_ptr<storage::Driver>& plugin) -> void;
    auto init_fs_backup(const std::filesystem::path& dir) -> void;
    auto init_lmdb(std::unique_ptr<storage::Driver>& plugin) -> void;
    auto init_memdb(std::unique_ptr<storage::Driver>& plugin) -> void;
    auto init_sqlite(std::unique_ptr<storage::Driver>& plugin) -> void;
    auto Init_Plugin() -> void;
    auto load(
        const Hash& key,
        ErrorReporting checking,
        Search order,
        Writer&& value,
        const Driver* driver) const noexcept -> bool;
    auto migrate(
        const Hash& rootHash,
        Bucket from,
        Bucket to,
        Driver* driver,
        bool setRoot) noexcept -> bool;
    auto migrate(
        std::span<const Hash> hashes,
        Bucket to,
        Search order,
        Driver* driver,
        std::optional<Hash> root) noexcept -> bool;
    auto migrate_primary(
        const UnallocatedCString& from,
        const UnallocatedCString& to) -> void;
    auto store(Transaction data, Bucket bucket) const noexcept -> Results;
    auto synchronize(
        const Hash& hash,
        Bucket bucket,
        storage::Driver& driver) noexcept -> void;
    auto synchronize_drivers(const Hash& hash, Bucket bucket) noexcept -> void;
};
}  // namespace opentxs::storage::driver::implementation
