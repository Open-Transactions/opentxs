// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs::ui
{
class OPENTXS_EXPORT BlockchainSubchain : virtual public ListRow
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
