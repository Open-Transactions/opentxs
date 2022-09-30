// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "internal/util/Lockable.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Storage;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
class Config;
class Plugin;
}  // namespace storage

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
// In-memory implementation of opentxs::storage
class MemDB final : public implementation::Plugin,
                    virtual public storage::Driver,
                    Lockable
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const UnallocatedCString& key,
        UnallocatedCString& value,
        const bool bucket) const -> bool final;
    auto LoadRoot() const -> UnallocatedCString final;
    auto StoreRoot(const bool commit, const UnallocatedCString& hash) const
        -> bool final;

    void Cleanup() final {}

    MemDB(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);
    MemDB() = delete;
    MemDB(const MemDB&) = delete;
    MemDB(MemDB&&) = delete;
    auto operator=(const MemDB&) -> MemDB& = delete;
    auto operator=(MemDB&&) -> MemDB& = delete;

    ~MemDB() final = default;

private:
    using ot_super = Plugin;

    mutable UnallocatedCString root_;
    mutable UnallocatedMap<UnallocatedCString, UnallocatedCString> a_;
    mutable UnallocatedMap<UnallocatedCString, UnallocatedCString> b_;

    void store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket,
        std::promise<bool>* promise) const final;
};
}  // namespace opentxs::storage::driver
