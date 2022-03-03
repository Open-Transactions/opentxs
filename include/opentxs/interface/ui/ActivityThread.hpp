// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class ActivityThread;
class ActivityThreadItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ActivityThread : virtual public List
{
public:
    virtual auto CanMessage() const noexcept -> bool = 0;
    virtual auto DisplayName() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem> = 0;
    virtual auto GetDraft() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem> = 0;
    virtual auto Participants() const noexcept -> UnallocatedCString = 0;
    virtual auto Pay(
        const UnallocatedCString& amount,
        const Identifier& sourceAccount,
        const UnallocatedCString& memo = "",
        const PaymentType type = PaymentType::Cheque) const noexcept
        -> bool = 0;
    virtual auto Pay(
        const Amount amount,
        const Identifier& sourceAccount,
        const UnallocatedCString& memo = "",
        const PaymentType type = PaymentType::Cheque) const noexcept
        -> bool = 0;
    virtual auto PaymentCode(const UnitType currency) const noexcept
        -> UnallocatedCString = 0;
    virtual auto SendDraft() const noexcept -> bool = 0;
    virtual auto SetDraft(const UnallocatedCString& draft) const noexcept
        -> bool = 0;
    virtual auto ThreadID() const noexcept -> UnallocatedCString = 0;

    ~ActivityThread() override = default;

protected:
    ActivityThread() noexcept = default;

private:
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    auto operator=(const ActivityThread&) -> ActivityThread& = delete;
    auto operator=(ActivityThread&&) -> ActivityThread& = delete;
};
}  // namespace opentxs::ui
