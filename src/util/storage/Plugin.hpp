// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <future>
#include <memory>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "opentxs/util/storage/Plugin.hpp"

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
}  // namespace storage

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::implementation
{
class Plugin : virtual public storage::Plugin
{
public:
    auto EmptyBucket(const bool bucket) const -> bool override = 0;

    auto Load(
        const UnallocatedCString& key,
        const bool checking,
        UnallocatedCString& value) const -> bool override;
    auto LoadFromBucket(
        const UnallocatedCString& key,
        UnallocatedCString& value,
        const bool bucket) const -> bool override = 0;
    auto Store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket) const -> bool override;
    void Store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket,
        std::promise<bool>& promise) const override;
    auto Store(
        const bool isTransaction,
        const UnallocatedCString& value,
        UnallocatedCString& key) const -> bool override;

    auto Migrate(const UnallocatedCString& key, const storage::Driver& to) const
        -> bool override;

    auto LoadRoot() const -> UnallocatedCString override = 0;
    auto StoreRoot(const bool commit, const UnallocatedCString& hash) const
        -> bool override = 0;

    virtual void Cleanup() = 0;

    Plugin() = delete;
    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    auto operator=(const Plugin&) -> Plugin& = delete;
    auto operator=(Plugin&&) -> Plugin& = delete;

    ~Plugin() override = default;

protected:
    const api::Crypto& crypto_;
    const api::network::Asio& asio_;
    const storage::Config& config_;

    Plugin(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);

    virtual void store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket,
        std::promise<bool>* promise) const = 0;

private:
    const api::session::Storage& storage_;
    const Flag& current_bucket_;
};
}  // namespace opentxs::storage::implementation

namespace opentxs::storage
{
template <class T>
auto Driver::LoadProto(
    const UnallocatedCString& hash,
    std::shared_ptr<T>& serialized,
    const bool checking) const -> bool
{
    auto raw = UnallocatedCString{};
    const auto loaded = Load(hash, checking, raw);
    auto valid{false};

    if (loaded) {
        serialized = proto::DynamicFactory<T>(raw.data(), raw.size());

        OT_ASSERT(serialized);

        valid = proto::Validate<T>(*serialized, VERBOSE);
    } else {

        return false;
    }

    if (!valid) {
        if (loaded) {
            LogError()(": Specified object was located but could not be "
                       "validated.")
                .Flush();
            LogError()(": Hash: ")(hash).Flush();
            LogError()(": Size: ")(raw.size()).Flush();
        } else {

            LogDetail()("Specified object is missing.").Flush();
            LogDetail()("Hash: ")(hash).Flush();
            LogDetail()("Size: ")(raw.size()).Flush();
        }
    }

    OT_ASSERT(valid);

    return valid;
}

template <class T>
auto Driver::StoreProto(
    const T& data,
    UnallocatedCString& key,
    UnallocatedCString& plaintext) const -> bool
{
    if (!proto::Validate<T>(data, VERBOSE)) { return false; }

    plaintext = proto::ToString(data);

    return Store(true, plaintext, key);
}

template <class T>
auto Driver::StoreProto(const T& data, UnallocatedCString& key) const -> bool
{
    UnallocatedCString notUsed;

    return StoreProto<T>(data, key, notUsed);
}

template <class T>
auto Driver::StoreProto(const T& data) const -> bool
{
    UnallocatedCString notUsed;

    return StoreProto<T>(data, notUsed);
}
}  // namespace opentxs::storage
