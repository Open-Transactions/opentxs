// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui
{
class BlockchainSubchain : virtual public ListRow
{
public:
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Progress() const noexcept -> UnallocatedCString = 0;
    virtual auto Type() const noexcept -> blockchain::crypto::Subchain = 0;

    BlockchainSubchain(const BlockchainSubchain&) = delete;
    BlockchainSubchain(BlockchainSubchain&&) = delete;
    auto operator=(const BlockchainSubchain&) -> BlockchainSubchain& = delete;
    auto operator=(BlockchainSubchain&&) -> BlockchainSubchain& = delete;

    ~BlockchainSubchain() override = default;

protected:
    BlockchainSubchain() noexcept = default;
};
}  // namespace opentxs::ui
