// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/Storage.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Storage : virtual public session::Storage
{
public:
    virtual auto InitBackup() -> void = 0;
    virtual auto InitEncryptedBackup(opentxs::crypto::key::Symmetric& key)
        -> void = 0;
    auto Internal() const noexcept -> const internal::Storage& final
    {
        return *this;
    }
    virtual auto start() -> void = 0;

    auto Internal() noexcept -> internal::Storage& final { return *this; }

    virtual ~Storage() override = default;

protected:
    Storage() = default;

private:
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;
};
}  // namespace opentxs::api::session::internal
