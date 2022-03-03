// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/drivers/filesystem/Common.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace storage
{
class Config;
class Plugin;
}  // namespace storage

class Flag;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver::filesystem
{
class Archiving final : public Common, public virtual storage::Driver
{
private:
    using ot_super = Common;

public:
    auto EmptyBucket(const bool bucket) const -> bool final;

    void Cleanup() final;

    Archiving(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket,
        const UnallocatedCString& folder,
        crypto::key::Symmetric& key);

    ~Archiving() final;

private:
    crypto::key::Symmetric& encryption_key_;
    const bool encrypted_;

    auto calculate_path(
        const UnallocatedCString& key,
        const bool bucket,
        UnallocatedCString& directory) const -> UnallocatedCString final;
    auto prepare_read(const UnallocatedCString& ciphertext) const
        -> UnallocatedCString final;
    auto prepare_write(const UnallocatedCString& plaintext) const
        -> UnallocatedCString final;
    auto root_filename() const -> UnallocatedCString final;

    void Init_Archiving();
    void Cleanup_Archiving();

    Archiving() = delete;
    Archiving(const Archiving&) = delete;
    Archiving(Archiving&&) = delete;
    auto operator=(const Archiving&) -> Archiving& = delete;
    auto operator=(Archiving&&) -> Archiving& = delete;
};
}  // namespace opentxs::storage::driver::filesystem
