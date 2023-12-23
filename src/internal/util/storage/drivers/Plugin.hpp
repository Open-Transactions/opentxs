// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/storage/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/storage/Driver.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace storage
{
namespace tree
{
class Root;
struct GCParams;
}  // namespace tree
}  // namespace storage

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
class Plugin
{
public:
    virtual auto Load(
        const Hash& key,
        ErrorReporting checking,
        Writer&& value,
        const Driver* driver = nullptr) const noexcept -> bool = 0;
    virtual auto LoadRoot() const noexcept -> Hash = 0;
    virtual auto Primary() const noexcept -> const Driver& = 0;

    virtual auto DoGC(const tree::GCParams& params) noexcept -> bool = 0;
    virtual auto EmptyBucket(Bucket bucket) const noexcept -> bool = 0;
    virtual auto FindBestRoot() noexcept -> Hash = 0;
    virtual auto InitBackup() -> void = 0;
    virtual auto InitEncryptedBackup(opentxs::crypto::symmetric::Key& key)
        -> void = 0;
    virtual auto Primary() noexcept -> Driver& = 0;
    virtual auto Store(ReadView value, Hash& key) const noexcept -> bool = 0;
    virtual auto StoreRoot(const Hash& hash) const noexcept -> bool = 0;

    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    auto operator=(const Plugin&) -> Plugin& = delete;
    auto operator=(Plugin&&) -> Plugin& = delete;

    virtual ~Plugin() = default;

protected:
    Plugin() = default;
};
}  // namespace opentxs::storage::driver
