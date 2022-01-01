// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <map>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Bytes.hpp"
#include "storage/Plugin.hpp"

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
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool final;
    auto LoadRoot() const -> std::string final;
    auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool final;

    void Cleanup() final {}

    MemDB(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);

    ~MemDB() final = default;

private:
    using ot_super = Plugin;

    mutable std::string root_;
    mutable std::pmr::map<std::string, std::string> a_;
    mutable std::pmr::map<std::string, std::string> b_;

    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const final;

    MemDB() = delete;
    MemDB(const MemDB&) = delete;
    MemDB(MemDB&&) = delete;
    auto operator=(const MemDB&) -> MemDB& = delete;
    auto operator=(MemDB&&) -> MemDB& = delete;
};
}  // namespace opentxs::storage::driver
