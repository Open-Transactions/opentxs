// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/crypto/Wallet.hpp"

#include "opentxs/blockchain/crypto/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Account;
}  // namespace identifier

namespace proto
{
class HDPath;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct Wallet : virtual public crypto::Wallet {
    virtual auto AddHDNode(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        identifier::Account& id) noexcept -> bool = 0;

    ~Wallet() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
