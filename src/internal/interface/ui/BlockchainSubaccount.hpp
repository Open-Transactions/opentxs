// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/List.hpp"
#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class BlockchainSubchain;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class BlockchainSubaccount : virtual public List, virtual public ListRow
{
public:
    virtual auto First() const noexcept -> SharedPimpl<BlockchainSubchain> = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept -> SharedPimpl<BlockchainSubchain> = 0;
    virtual auto SubaccountID() const noexcept
        -> const identifier::Generic& = 0;

    BlockchainSubaccount(const BlockchainSubaccount&) = delete;
    BlockchainSubaccount(BlockchainSubaccount&&) = delete;
    auto operator=(const BlockchainSubaccount&)
        -> BlockchainSubaccount& = delete;
    auto operator=(BlockchainSubaccount&&) -> BlockchainSubaccount& = delete;

    ~BlockchainSubaccount() override = default;

protected:
    BlockchainSubaccount() noexcept = default;
};
}  // namespace opentxs::ui
