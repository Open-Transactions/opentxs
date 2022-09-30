// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace ui
{
class BlockchainSubaccountSource;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model contains the status for a single blockchain account.
  Each subaccount is a row in this model.
*/
class OPENTXS_EXPORT BlockchainAccountStatus : virtual public List
{
public:
    /// Returns the chain type for this blockchain account.
    virtual auto Chain() const noexcept -> blockchain::Type = 0;
    /// returns the first row, containing a valid BlockchainSubaccountSource or
    /// an empty smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> SharedPimpl<BlockchainSubaccountSource> = 0;
    /// returns the next row, containing a valid BlockchainSubaccountSource or
    /// an empty smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> SharedPimpl<BlockchainSubaccountSource> = 0;
    /// Returns the NymID of the owner of this account.
    virtual auto Owner() const noexcept -> const identifier::Nym& = 0;
    BlockchainAccountStatus(const BlockchainAccountStatus&) = delete;
    BlockchainAccountStatus(BlockchainAccountStatus&&) = delete;
    auto operator=(const BlockchainAccountStatus&)
        -> BlockchainAccountStatus& = delete;
    auto operator=(BlockchainAccountStatus&&)
        -> BlockchainAccountStatus& = delete;

    ~BlockchainAccountStatus() override = default;

protected:
    BlockchainAccountStatus() noexcept = default;
};
}  // namespace opentxs::ui
