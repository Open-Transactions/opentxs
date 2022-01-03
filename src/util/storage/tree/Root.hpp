// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/StorageRoot.pb.h"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Tree.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
namespace imp
{
class Storage;
}  // namespace imp
}  // namespace session
}  // namespace api

namespace storage
{
class Driver;
class Tree;

namespace driver
{
class Multiplex;
}  // namespace driver

namespace implementation
{
class Driver;
}  // namespace implementation
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Root final : public Node
{
public:
    auto Tree() const -> const storage::Tree&;

    auto mutable_Tree() -> Editor<storage::Tree>;

    auto Migrate(const Driver& to) const -> bool final;
    auto Save(const Driver& to) const -> bool;
    auto Sequence() const -> std::uint64_t;

    ~Root() final = default;

private:
    using ot_super = Node;
    friend opentxs::storage::driver::Multiplex;
    friend api::session::imp::Storage;

    class GC
    {
    public:
        enum class CheckState {
            Resume,
            Skip,
            Start,
        };

        auto Serialize(proto::StorageRoot& out) const noexcept -> void;

        auto Check(const std::string root) noexcept -> CheckState;
        auto Cleanup() noexcept -> void;
        auto Init(
            const std::string& root,
            bool resume,
            std::uint64_t last) noexcept -> void;
        auto Resume(bool fromBucket) noexcept -> bool;
        auto Run(const bool from, const Driver& to, SimpleCallback cb) noexcept
            -> bool;
        auto Start(bool fromBucket) noexcept -> bool;

        GC(const api::network::Asio& asio,
           const Driver& driver,
           const std::int64_t interval)
        noexcept;

        ~GC();

    private:
        const api::network::Asio& asio_;
        const Driver& driver_;
        const std::uint64_t interval_;
        mutable std::mutex lock_;
        OTFlag running_;
        OTFlag resume_;
        std::string root_;
        std::atomic<std::uint64_t> last_;
        std::promise<bool> promise_;
        std::shared_future<bool> future_;

        auto collect_garbage(
            const bool from,
            const Driver* to,
            const SimpleCallback done) noexcept -> void;
    };

    Flag& current_bucket_;
    mutable std::atomic<std::uint64_t> sequence_;
    mutable GC gc_;
    std::string tree_root_;
    mutable std::mutex tree_lock_;
    mutable std::unique_ptr<storage::Tree> tree_;

    auto serialize(const Lock&) const -> proto::StorageRoot;
    auto tree() const -> storage::Tree*;

    void blank(const VersionNumber version) final;
    void cleanup() const;
    void init(const std::string& hash) final;
    auto save(const Lock& lock, const Driver& to) const -> bool;
    auto save(const Lock& lock) const -> bool final;
    void save(storage::Tree* tree, const Lock& lock);

    Root(
        const api::network::Asio& asio,
        const Driver& storage,
        const std::string& hash,
        const std::int64_t interval,
        Flag& bucket);
    Root() = delete;
    Root(const Root&) = delete;
    Root(Root&&) = delete;
    auto operator=(const Root&) -> Root = delete;
    auto operator=(Root&&) -> Root = delete;
};
}  // namespace opentxs::storage