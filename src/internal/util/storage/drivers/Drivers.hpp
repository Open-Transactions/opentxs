// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace storage
{
class Root;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver::internal
{
class Multiplex : virtual public Driver
{
public:
    virtual auto BestRoot(bool& primaryOutOfSync) -> UnallocatedCString = 0;
    virtual void InitBackup() = 0;
    virtual void InitEncryptedBackup(opentxs::crypto::key::Symmetric& key) = 0;
    virtual auto Primary() -> Driver& = 0;
    virtual void SynchronizePlugins(
        const UnallocatedCString& hash,
        const opentxs::storage::Root& root,
        const bool syncPrimary) = 0;

    Multiplex(const Multiplex&) = delete;
    Multiplex(Multiplex&&) = delete;
    auto operator=(const Multiplex&) -> Multiplex& = delete;
    auto operator=(Multiplex&&) -> Multiplex& = delete;

    ~Multiplex() override = default;

protected:
    Multiplex() = default;
};
}  // namespace opentxs::storage::driver::internal
