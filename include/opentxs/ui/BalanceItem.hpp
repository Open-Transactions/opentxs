// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <list>
#include <string>

#include "ListRow.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace ui
{
class BalanceItem;
}  // namespace ui

using OTUIBalanceItem = SharedPimpl<ui::BalanceItem>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT BalanceItem : virtual public ListRow
{
public:
    virtual auto Amount() const noexcept -> opentxs::Amount = 0;
    virtual auto Confirmations() const noexcept -> int = 0;
    virtual auto Contacts() const noexcept -> std::pmr::vector<std::string> = 0;
    virtual auto DisplayAmount() const noexcept -> std::string = 0;
    virtual auto Memo() const noexcept -> std::string = 0;
    virtual auto Text() const noexcept -> std::string = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Type() const noexcept -> StorageBox = 0;
    virtual auto UUID() const noexcept -> std::string = 0;
    virtual auto Workflow() const noexcept -> std::string = 0;

    ~BalanceItem() override = default;

protected:
    BalanceItem() noexcept = default;

private:
    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    auto operator=(const BalanceItem&) -> BalanceItem& = delete;
    auto operator=(BalanceItem&&) -> BalanceItem& = delete;
};
}  // namespace ui
}  // namespace opentxs
