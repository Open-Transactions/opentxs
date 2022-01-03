// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/util/storage/Driver.hpp"

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

namespace opentxs::storage::driver::internal
{
class Multiplex : virtual public Driver
{
public:
    virtual auto BestRoot(bool& primaryOutOfSync) -> std::string = 0;
    virtual void InitBackup() = 0;
    virtual void InitEncryptedBackup(opentxs::crypto::key::Symmetric& key) = 0;
    virtual auto Primary() -> Driver& = 0;
    virtual void SynchronizePlugins(
        const std::string& hash,
        const opentxs::storage::Root& root,
        const bool syncPrimary) = 0;

    ~Multiplex() override = default;

protected:
    Multiplex() = default;

private:
    Multiplex(const Multiplex&) = delete;
    Multiplex(Multiplex&&) = delete;
    auto operator=(const Multiplex&) -> Multiplex& = delete;
    auto operator=(Multiplex&&) -> Multiplex& = delete;
};
}  // namespace opentxs::storage::driver::internal
