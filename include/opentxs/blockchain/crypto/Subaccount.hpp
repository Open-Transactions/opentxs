// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <set>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Subaccount;
}  // namespace internal

class Account;
class Element;
}  // namespace crypto
}  // namespace blockchain

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Subaccount
{
public:
    using Txid = opentxs::blockchain::block::Txid;

    virtual auto AllowedSubchains() const noexcept
        -> std::pmr::set<Subchain> = 0;
    /// Throws std::out_of_range for invalid index
    virtual auto BalanceElement(const Subchain type, const Bip32Index index)
        const noexcept(false) -> const crypto::Element& = 0;
    virtual auto ID() const noexcept -> const Identifier& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> internal::Subaccount& = 0;
    virtual auto Parent() const noexcept -> const Account& = 0;
    virtual auto ScanProgress(Subchain subchain) const noexcept
        -> block::Position = 0;
    virtual auto Type() const noexcept -> SubaccountType = 0;

    OPENTXS_NO_EXPORT virtual ~Subaccount() = default;

protected:
    Subaccount() noexcept = default;

private:
    Subaccount(const Subaccount&) = delete;
    Subaccount(Subaccount&&) = delete;
    auto operator=(const Subaccount&) -> Subaccount& = delete;
    auto operator=(Subaccount&&) -> Subaccount& = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
