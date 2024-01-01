// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/StorageRoot.pb.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>

#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/tree/Types.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace imp
{
class Storage;
}  // namespace imp

class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
namespace driver
{
namespace implementation
{
class Plugin;
}  // namespace implementation

class Plugin;
}  // namespace driver

namespace tree
{
class Trunk;
}  // namespace tree

class Driver;
}  // namespace storage

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Root final : public Node
{
public:
    static auto CheckSequence(
        const Log& log,
        const Hash& hash,
        const storage::Driver& driver) noexcept -> std::uint64_t;

    auto GCStatus() const noexcept -> GCParams;
    auto Trunk() const -> const tree::Trunk&;

    auto mutable_Trunk() -> Editor<tree::Trunk>;

    auto FinishGC(bool success) noexcept -> void;
    auto Sequence() const -> std::uint64_t;
    auto StartGC() noexcept -> std::optional<GCParams>;

    Root(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash,
        std::atomic<Bucket>& bucket) noexcept(false);
    Root() = delete;
    Root(const Root&) = delete;
    Root(Root&&) = delete;
    auto operator=(const Root&) -> Root = delete;
    auto operator=(Root&&) -> Root = delete;

    ~Root() final;

private:
    friend driver::implementation::Plugin;
    friend api::session::imp::Storage;

    static constexpr auto current_version_ = VersionNumber{3};

    std::atomic<Bucket>& current_bucket_;
    mutable std::atomic<std::uint64_t> sequence_;
    GCParams gc_params_;
    Hash tree_root_;
    mutable std::mutex tree_lock_;
    mutable std::unique_ptr<tree::Trunk> tree_;

    auto dump(const Lock&, const Log&, Vector<Hash>& out) const noexcept
        -> bool final;
    auto serialize(const Lock&) const -> protobuf::StorageRoot;
    auto trunk() const -> tree::Trunk*;

    auto blank() noexcept -> void final;
    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const Lock& lock) const -> bool final;
    auto save(tree::Trunk* tree, const Lock& lock) -> void;
    auto upgrade(const Lock& lock) noexcept -> bool final;
};
}  // namespace opentxs::storage::tree
