// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/plugin/Plugin.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <variant>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/Types.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "internal/util/storage/tree/Types.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"  // IWYU pragma: keep
#include "opentxs/util/storage/Driver.hpp"
#include "util/ScopeGuard.hpp"
#include "util/storage/Config.hpp"
#include "util/storage/tree/Root.hpp"

namespace opentxs::factory
{
auto StoragePlugin(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const std::atomic<storage::Bucket>& primaryBucket,
    const storage::Config& config) noexcept
    -> std::shared_ptr<storage::driver::Plugin>
{
    using ReturnType = opentxs::storage::driver::implementation::Plugin;

    return std::make_shared<ReturnType>(crypto, factory, primaryBucket, config);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver::implementation
{
using namespace std::literals;

Plugin::Plugin(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const std::atomic<Bucket>& primaryBucket,
    const storage::Config& config)
    : crypto_(crypto)
    , factory_(factory)
    , primary_bucket_(primaryBucket)
    , config_(config)
    , primary_driver_()
    , backup_drivers_()
    , drivers_()
    , null_()
    , root_(NullHash{})
    , write_()
    , init_promise_()
    , init_(init_promise_.get_future())
{
    Init_Plugin();
}

auto Plugin::calculate_hash(ReadView data, Hash& out) const noexcept -> bool
{
    static constexpr auto hashType = crypto::HashType::Sha256;
    out = UnencodedHash{};
    const auto hashed = crypto_.Hash().Digest(
        hashType, data, std::get<UnencodedHash>(out).WriteInto());

    if (hashed) {

        return true;
    } else {
        out = NullHash{};

        return false;
    }
}

auto Plugin::check_revision(const Log& log, Results::value_type& in) noexcept
    -> void
{
    auto& [p, revision] = in;

    if (nullptr == p) { return; }

    auto& driver = *p;
    const auto hash = driver.LoadRoot();

    if (false == is_valid(hash)) { return; }

    revision = tree::Root::CheckSequence(log, hash, driver);
    log(OT_PRETTY_STATIC(Plugin))(driver.Description())(
        " driver has root hash ")(hash)(" and revision ")(revision)
        .Flush();
}

auto Plugin::Cleanup() -> void { Cleanup_Plugin(); }

auto Plugin::Cleanup_Plugin() -> void {}

auto Plugin::DoGC(const tree::GCParams& params) noexcept -> bool
{
    const auto& log = LogTrace();
    const auto from = params.from_;
    const auto to = next(from);
    log(OT_PRETTY_CLASS())("migrating all active objects from ")(print(from))(
        " to ")(print(to))
        .Flush();

    if (migrate(params.root_, from, to, nullptr, false)) {
        log(OT_PRETTY_CLASS())("purging stale bucket ")(print(from)).Flush();

        return EmptyBucket(from);
    } else {

        return false;
    }
}

auto Plugin::compare_results(
    const Results::value_type& lhs,
    const Results::value_type& rhs) noexcept -> bool
{
    return lhs.second < rhs.second;
}

auto Plugin::EmptyBucket(Bucket bucket) const noexcept -> bool
{
    return evaluate(evaluate(empty_bucket(bucket)));
}

auto Plugin::evaluate(std::uint64_t results) noexcept -> bool
{
    return 0 < results;
}

auto Plugin::find_best(Results& lhs, Results::iterator rhs) noexcept
    -> std::size_t
{
    return static_cast<std::size_t>(std::distance(lhs.begin(), rhs));
}

auto Plugin::FindBestRoot() noexcept -> Hash
{
    const auto post = ScopeGuard{[this] { init_promise_.set_value(); }};
    const auto& log = LogTrace();

    if (drivers_.empty()) {
        LogError()(OT_PRETTY_CLASS())(" no storage drivers instantiated")
            .Flush();

        return NullHash{};
    } else {
        log(OT_PRETTY_CLASS())("searching ")(drivers_.size())(
            " drivers for best root hash")
            .Flush();
    }

    auto results = scan(log);
    const auto pos = find_best(results, find_best(results));
    const auto& [p, bestRevision] = results[pos];
    const auto& bestDriver = *p;

    if (0u == bestRevision) {
        log(OT_PRETTY_CLASS())("initializing empty database").Flush();

        return NullHash{};
    } else {
        log(OT_PRETTY_CLASS())(bestDriver.Description())(" at index ")(
            pos)(" has best root hash")
            .Flush();
    }

    const auto bestHash = bestDriver.LoadRoot();
    *root_.lock() = bestHash;

    if (false == backup_drivers_.empty()) {
        const auto bucket = [&] {
            auto out = std::atomic<Bucket>{};
            const auto root =
                tree::Root{crypto_, factory_, *this, bestHash, out};

            return out.load();
        }();
        synchronize_drivers(bestHash, bucket);
    }

    log(OT_PRETTY_CLASS())("best root hash is ")(bestHash).Flush();

    return bestHash;
}

auto Plugin::init(
    const UnallocatedCString& primary,
    std::unique_ptr<storage::Driver>& plugin) -> void
{
    if (OT_STORAGE_PRIMARY_PLUGIN_MEMDB == primary) {
        init_memdb(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_LMDB == primary) {
        init_lmdb(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_SQLITE == primary) {
        init_sqlite(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_FS == primary) {
        init_fs(plugin);
    }

    OT_ASSERT(plugin);
}

auto Plugin::init_memdb(std::unique_ptr<storage::Driver>& plugin) -> void
{
    LogVerbose()(OT_PRETTY_CLASS())("Initializing primary MemDB plugin.")
        .Flush();
    plugin = factory::StorageMemDB(crypto_, config_);
}

auto Plugin::Init_Plugin() -> void
{
    if (config_.migrate_plugin_) {
        migrate_primary(
            config_.previous_primary_plugin_, config_.primary_plugin_);
    } else {
        init(config_.primary_plugin_, primary_driver_);
    }

    OT_ASSERT(primary_driver_);

    // TODO init backup plugins
    drivers_.reserve(1_uz + backup_drivers_.size());
    drivers_.clear();
    drivers_.emplace_back(primary_driver_.get());
    const auto get_pointer = [](const auto& sp) {
        OT_ASSERT(sp);

        return sp.get();
    };
    std::transform(
        backup_drivers_.begin(),
        backup_drivers_.end(),
        std::back_inserter(drivers_),
        get_pointer);
}

auto Plugin::InitBackup() -> void
{
    if (config_.fs_backup_directory_.empty()) { return; }

    init_fs_backup(config_.fs_backup_directory_.string());
}

auto Plugin::InitEncryptedBackup([[maybe_unused]] crypto::symmetric::Key& key)
    -> void
{
    if (config_.fs_encrypted_backup_directory_.empty()) { return; }

    init_fs_backup(config_.fs_encrypted_backup_directory_.string());
}

auto Plugin::Load(
    const Hash& key,
    ErrorReporting checking,
    Writer&& value,
    const Driver* specifiedDriver) const noexcept -> bool
{
    return load(
        key,
        checking,
        get_search_order(primary_bucket_.load()),
        std::move(value),
        specifiedDriver);
}

auto Plugin::load(
    const Hash& key,
    ErrorReporting checking,
    Search order,
    Writer&& value,
    const Driver* specifiedDriver) const noexcept -> bool
{
    using enum ErrorReporting;
    const auto& log = (silent == checking) ? LogTrace() : LogError();

    if (nullptr == specifiedDriver) {
        for (const auto* driver : drivers_) {
            if (driver->Load(log, key, order, value)) {

                return true;
            } else {
                log(OT_PRETTY_CLASS())("key ")(key)(" not found by ")(
                    driver->Description())(" driver")
                    .Flush();
            }
        }

        log(OT_PRETTY_CLASS())("key ")(key)(" not found by any driver").Flush();
    } else {
        if (specifiedDriver->Load(log, key, order, value)) {

            return true;
        } else {
            log(OT_PRETTY_CLASS())("key ")(key)(" not found by ")(
                specifiedDriver->Description())(" driver")
                .Flush();
        }
    }

    return false;
}

auto Plugin::LoadRoot() const noexcept -> Hash
{
    init_.get();

    return *root_.lock();
}

auto Plugin::make_results() const noexcept -> Results
{
    auto out = Results{};
    out.reserve(drivers_.size());
    out.clear();
    const auto get_pointer = [](auto& p) { return std::make_pair(p, 0); };
    std::transform(
        drivers_.begin(), drivers_.end(), std::back_inserter(out), get_pointer);

    return out;
}

auto Plugin::migrate(
    const Hash& rootHash,
    Bucket from,
    Bucket to,
    Driver* driver,
    bool setRoot) noexcept -> bool
{
    try {
        const auto& log = LogTrace();
        const auto hashes = [&] {
            auto bucket = std::atomic<Bucket>{};
            const auto root =
                tree::Root{crypto_, factory_, *this, rootHash, bucket};
            auto out = Vector<Hash>{};

            if (false == root.Dump(out)) {

                throw std::runtime_error{"failed to query hash list"};
            }

            return out;
        }();
        log(OT_PRETTY_CLASS())("copying ")(hashes.size())(" objects").Flush();

        return migrate(
            hashes,
            to,
            get_search_order(from),
            driver,
            setRoot ? std::make_optional<Hash>(rootHash) : std::nullopt);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Plugin::migrate(
    std::span<const Hash> hashes,
    Bucket to,
    Search order,
    Driver* driver,
    std::optional<Hash> root) noexcept -> bool
{
    try {
        static constexpr auto max = 100_uz;
        const auto stop = hashes.size();
        auto buf = PendingWrite{};
        auto n = 0_uz;
        const auto fill_buffer = [&] {
            while ((buf.size() < max) && (n < stop)) {
                using enum ErrorReporting;
                const auto& hash = hashes[n];
                const auto loaded =
                    load(hash, verbose, order, buf.Add(hash), driver);

                if (false == loaded) {
                    throw std::runtime_error{
                        "failed to load item "s.append(to_string(hash))};
                }

                ++n;
            }
        };

        do {
            fill_buffer();
            buf.RecalculateViews();
            const auto tx = Transaction{buf.Keys(), buf.Data()};
            const auto isLastIteration = (n == stop);
            const auto setRoot = isLastIteration && root.has_value();
            const auto saved = [&] {
                if (nullptr == driver) {
                    if (setRoot) {

                        return evaluate(evaluate(commit(*root, tx, to)));
                    } else {

                        return evaluate(evaluate(store(tx, to)));
                    }
                } else {
                    if (setRoot) {

                        return driver->Commit(*root, tx, to);
                    } else {

                        return driver->Store(tx, to);
                    }
                }
            }();

            if (false == saved) {
                throw std::runtime_error{"failed to save batch"};
            }

            buf.Reset();
        } while (n < stop);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Plugin::migrate_primary(
    const UnallocatedCString& from,
    const UnallocatedCString& to) -> void
{
    auto& old = primary_driver_;
    init(from, old);

    OT_ASSERT(old);

    std::unique_ptr<storage::Driver> newDriver{nullptr};
    init(to, newDriver);

    OT_ASSERT(newDriver);

    const auto rootHash = old->LoadRoot();
    const auto fromBucket = [&] {
        auto bucket = std::atomic<Bucket>{};
        const auto root =
            tree::Root{crypto_, factory_, *this, rootHash, bucket};

        return bucket.load();
    }();
    const auto migrated =
        migrate(old->LoadRoot(), fromBucket, fromBucket, newDriver.get(), true);

    if (false == migrated) {
        LogAbort()(OT_PRETTY_CLASS())("Failed to migrate from primary driver")
            .Abort();
    }

    old.reset(newDriver.release());
}

auto Plugin::Primary() const noexcept -> const storage::Driver&
{
    return const_cast<Plugin*>(this)->Primary();
}

auto Plugin::Primary() noexcept -> storage::Driver& { return *primary_driver_; }

auto Plugin::Store(ReadView value, Hash& key) const noexcept -> bool
{
    if (calculate_hash(value, key)) {
        LogTrace()(OT_PRETTY_CLASS())("queueing hash ")(key)(" for next save")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("failed to calculate hash for item")
            .Flush();

        return false;
    }

    write_.lock()->Add(key, value);

    return true;
}

auto Plugin::StoreRoot(const Hash& hash) const noexcept -> bool
{
    LogTrace()(OT_PRETTY_CLASS())(
        "committing queue and updating root hash to ")(hash)
        .Flush();
    auto handle = write_.lock();
    auto& data = *handle;
    const auto write = [&] {
        auto out = PendingWrite{};
        out.Reset();
        data.swap(out);
        out.RecalculateViews();

        return out;
    }();
    const auto tx = Transaction{write.Keys(), write.Data()};
    const auto bucket = primary_bucket_.load();
    const auto result = evaluate(evaluate(commit(hash, tx, bucket)));
    *root_.lock() = hash;

    return result;
}

auto Plugin::synchronize(
    const Hash& hash,
    Bucket bucket,
    storage::Driver& driver) noexcept -> void
{
    if (hash == driver.LoadRoot()) { return; }

    LogConsole()(OT_PRETTY_STATIC(Plugin))("synchronizing ")(
        driver.Description())(" driver which is uninitialized or out of date")
        .Flush();

    const auto migrated =
        migrate(hash, bucket, bucket, std::addressof(driver), true);

    if (migrated) {
        LogConsole()(OT_PRETTY_STATIC(Plugin))("successfully synchronised ")(
            driver.Description())(" driver leaf nodes")
            .Flush();
    } else {
        LogError()(OT_PRETTY_STATIC(Plugin))("failed to synchronize ")(
            driver.Description())(" driver leaf nodes")
            .Flush();
    }
}

Plugin::~Plugin() { Cleanup_Plugin(); }
}  // namespace opentxs::storage::driver::implementation
