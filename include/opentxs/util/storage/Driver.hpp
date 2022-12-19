// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::storage
{
class Driver
{
public:
    virtual auto EmptyBucket(const bool bucket) const -> bool = 0;

    virtual auto Load(
        const UnallocatedCString& key,
        const bool checking,
        UnallocatedCString& value) const -> bool = 0;
    virtual auto LoadFromBucket(
        const UnallocatedCString& key,
        UnallocatedCString& value,
        const bool bucket) const -> bool = 0;

    virtual auto Store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket) const -> bool = 0;
    virtual auto Store(
        const bool isTransaction,
        const UnallocatedCString& value,
        UnallocatedCString& key) const -> bool = 0;

    virtual auto Migrate(const UnallocatedCString& key, const Driver& to) const
        -> bool = 0;

    virtual auto LoadRoot() const -> UnallocatedCString = 0;
    virtual auto StoreRoot(const bool commit, const UnallocatedCString& hash)
        const -> bool = 0;

    virtual ~Driver() = default;

    template <class T>
    auto LoadProto(
        const UnallocatedCString& hash,
        std::shared_ptr<T>& serialized,
        const bool checking = false) const -> bool;

    template <class T>
    auto StoreProto(
        const T& data,
        UnallocatedCString& key,
        UnallocatedCString& plaintext) const -> bool;

    template <class T>
    auto StoreProto(const T& data, UnallocatedCString& key) const -> bool;

    template <class T>
    auto StoreProto(const T& data) const -> bool;

    Driver(const Driver&) = delete;
    Driver(Driver&&) = delete;
    auto operator=(const Driver&) -> Driver& = delete;
    auto operator=(Driver&&) -> Driver& = delete;

protected:
    Driver() = default;
};
}  // namespace opentxs::storage
