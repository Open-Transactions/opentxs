// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class BlockchainStatistics;
class BlockchainStatisticsItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 The rows in this model each represent a single blockchain and its related
 wallet statistics.
*/
class OPENTXS_EXPORT BlockchainStatistics : virtual public List
{
public:
    /// returns the first row, containing a valid BlockchainStatisticsItem or an
    /// empty smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BlockchainStatisticsItem> = 0;
    /// returns the next row, containing a valid BlockchainStatisticsItem or an
    /// empty smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BlockchainStatisticsItem> = 0;

    BlockchainStatistics(const BlockchainStatistics&) = delete;
    BlockchainStatistics(BlockchainStatistics&&) = delete;
    auto operator=(const BlockchainStatistics&)
        -> BlockchainStatistics& = delete;
    auto operator=(BlockchainStatistics&&) -> BlockchainStatistics& = delete;

    ~BlockchainStatistics() override = default;

protected:
    BlockchainStatistics() noexcept = default;
};
}  // namespace opentxs::ui
