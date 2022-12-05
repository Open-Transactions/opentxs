// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/Plugin.hpp"  // IWYU pragma: associated

#include "internal/api/network/Asio.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::storage::implementation
{
Plugin::Plugin(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& storage,
    const storage::Config& config,
    const Flag& bucket)
    : crypto_(crypto)
    , asio_(asio)
    , config_(config)
    , storage_(storage)
    , current_bucket_(bucket)
{
}

auto Plugin::Load(
    const UnallocatedCString& key,
    const bool checking,
    UnallocatedCString& value) const -> bool
{
    if (key.empty()) {
        if (!checking) {
            LogError()(OT_PRETTY_CLASS())("Error: Tried to load empty key.")
                .Flush();
        }

        return false;
    }

    bool valid = false;
    const bool bucket{current_bucket_};

    if (LoadFromBucket(key, value, bucket)) { valid = 0 < value.size(); }

    if (!valid) {
        // try again in the other bucket
        if (LoadFromBucket(key, value, !bucket)) {
            valid = 0 < value.size();
        } else {
            // just in case...
            if (LoadFromBucket(key, value, bucket)) {
                valid = 0 < value.size();
            }
        }
    }

    if (!valid && !checking) {
        LogDetail()(OT_PRETTY_CLASS())("Specified object is not found.")(
            " Hash: ")(key)(".")(" Size: ")(value.size())(".")
            .Flush();
    }

    return valid;
}

auto Plugin::Migrate(const UnallocatedCString& key, const storage::Driver& to)
    const -> bool
{
    if (key.empty()) { return false; }

    UnallocatedCString value;
    const bool targetBucket{current_bucket_};
    auto sourceBucket = targetBucket;

    if (&to == this) { sourceBucket = !targetBucket; }

    // try to load the key from the source bucket
    if (LoadFromBucket(key, value, sourceBucket)) {

        // save to the target bucket
        if (to.Store(false, key, value, targetBucket)) {
            return true;
        } else {
            LogError()(OT_PRETTY_CLASS())("Save failure.").Flush();

            return false;
        }
    }

    // If the key is not in the source bucket, it should be in the target
    // bucket
    const bool exists = to.LoadFromBucket(key, value, targetBucket);

    if (!exists) {
        LogVerbose()(OT_PRETTY_CLASS())("Missing key.").Flush();

        return false;
    }

    return true;
}

auto Plugin::Store(
    const bool isTransaction,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket) const -> bool
{
    std::promise<bool> promise;
    auto future = promise.get_future();
    store(isTransaction, key, value, bucket, &promise);

    return future.get();
}

void Plugin::Store(
    const bool isTransaction,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket,
    std::promise<bool>& promise) const
{
    // NOTE taking arguments by reference is safe if and only if the caller is
    // waiting on the future before allowing the input values to pass out of
    // scope
    asio_.Internal().Post(
        ThreadPool::Storage,
        [&] { store(isTransaction, key, value, bucket, &promise); },
        "Store");
}

auto Plugin::Store(
    const bool isTransaction,
    const UnallocatedCString& value,
    UnallocatedCString& key) const -> bool
{
    const bool bucket{current_bucket_};
    const auto hashed =
        crypto_.Hash().Digest(storage_.HashType(), value, writer(key));

    if (hashed) { return Store(isTransaction, key, value, bucket); }

    return false;
}
}  // namespace opentxs::storage::implementation
