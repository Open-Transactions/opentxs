// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/interface/ui/UI.hpp"  // IWYU pragma: associated

#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BalanceItemBlockchain(
    const ui::implementation::AccountActivityInternalInterface&,
    const api::session::Client&,
    const ui::implementation::AccountActivityRowID&,
    const ui::implementation::AccountActivitySortKey&,
    ui::implementation::CustomData&,
    const identifier::Nym&,
    const identifier::Account&) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>
{
    return std::make_shared<ui::implementation::AccountActivityRowBlank>();
}

auto BlockchainAccountActivityModel(
    const api::session::Client&,
    const identifier::Nym&,
    const identifier::Account&,
    const SimpleCallback&) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>
{
    struct Blank final : public ui::internal::AccountActivity,
                         public ui::internal::blank::Widget {
        using RowType = opentxs::SharedPimpl<opentxs::ui::BalanceItem>;
        using RowIDType = ui::implementation::AccountActivityRowID;

        auto AccountID() const noexcept -> UnallocatedCString final
        {
            return {};
        }
        auto API() const noexcept(false) -> const api::Session& final
        {
            throw std::out_of_range{"blank model"};
        }
        auto Balance() const noexcept -> const opentxs::Amount final
        {
            return {};
        }
        auto BalancePolarity() const noexcept -> int final { return {}; }
        [[noreturn]] auto Contract() const noexcept
            -> const contract::Unit& final
        {
            LogAbort()().Abort();
        }
        auto ContractID() const noexcept -> UnallocatedCString final
        {
            return {};
        }
        auto DepositAddress() const noexcept -> UnallocatedCString final
        {
            return {};
        }
        auto DepositAddress(const blockchain::Type chain) const noexcept
            -> UnallocatedCString final
        {
            return {};
        }
        auto DepositChains() const noexcept
            -> UnallocatedVector<blockchain::Type> final
        {
            return {};
        }
        auto DisplayBalance() const noexcept -> UnallocatedCString final
        {
            return {};
        }
        auto DisplayUnit() const noexcept -> UnallocatedCString final
        {
            return {};
        }
        auto First() const noexcept -> RowType final
        {
            static const auto blank = RowType{std::make_shared<
                ui::implementation::AccountActivityRowBlank>()};

            return blank;
        }
        auto GetQt() const noexcept -> ui::qt::internal::Model* final
        {
            return nullptr;
        }
        auto last(const RowIDType&) const noexcept -> bool final
        {
            return true;
        }
        auto Name() const noexcept -> UnallocatedCString final { return {}; }
        auto Next() const noexcept -> RowType final { return First(); }
        [[noreturn]] auto Notary() const noexcept
            -> const contract::Server& final
        {
            LogAbort()().Abort();
        }
        auto NotaryID() const noexcept -> UnallocatedCString final
        {
            return {};
        }
        auto NotaryName() const noexcept -> UnallocatedCString final
        {
            return {};
        }
        auto Notify(std::span<const opentxs::PaymentCode>) const noexcept
            -> bool final
        {
            return {};
        }
        auto Notify(
            std::span<const opentxs::PaymentCode>,
            ui::implementation::SendMonitor::Callback) const noexcept
            -> int final
        {
            return {};
        }
        auto Send(
            const identifier::Generic&,
            const opentxs::Amount&,
            const std::string_view,
            std::span<const opentxs::PaymentCode>) const noexcept -> bool final
        {
            return false;
        }
        auto Send(
            const identifier::Generic&,
            const UnallocatedCString&,
            const std::string_view,
            Scale,
            std::span<const opentxs::PaymentCode>) const noexcept -> bool final
        {
            return false;
        }
        auto Send(
            const identifier::Generic&,
            const UnallocatedCString&,
            const std::string_view,
            Scale,
            ui::implementation::SendMonitor::Callback,
            std::span<const opentxs::PaymentCode>) const noexcept -> int final
        {
            return false;
        }
        auto Send(
            const UnallocatedCString&,
            const opentxs::Amount&,
            const std::string_view,
            std::span<const opentxs::PaymentCode>) const noexcept -> bool final
        {
            return false;
        }
        auto Send(
            const UnallocatedCString&,
            const UnallocatedCString&,
            const std::string_view,
            Scale,
            std::span<const opentxs::PaymentCode>) const noexcept -> bool final
        {
            return false;
        }
        auto Send(
            const UnallocatedCString&,
            const UnallocatedCString&,
            const std::string_view,
            Scale,
            ui::implementation::SendMonitor::Callback,
            std::span<const opentxs::PaymentCode> notify) const noexcept
            -> int final
        {
            return false;
        }
        [[noreturn]] auto SendMonitor() const noexcept
            -> ui::implementation::SendMonitor& final
        {
            LogAbort()().Abort();
        }
        auto SyncPercentage() const noexcept -> double final { return {}; }
        auto SyncProgress() const noexcept -> std::pair<int, int> final
        {
            return {};
        }
        auto Type() const noexcept -> AccountType final { return {}; }
        auto Unit() const noexcept -> UnitType final { return {}; }
        auto ValidateAddress(const UnallocatedCString&) const noexcept
            -> bool final
        {
            return {};
        }
        auto ValidateAmount(const UnallocatedCString&) const noexcept
            -> UnallocatedCString final
        {
            return {};
        }

        [[noreturn]] auto AmountValidator() noexcept
            -> ui::AmountValidator& final
        {
            LogAbort()().Abort();
        }
        [[noreturn]] auto DestinationValidator() noexcept
            -> ui::DestinationValidator& final
        {
            LogAbort()().Abort();
        }
        [[noreturn]] auto DisplayScaleQt() noexcept -> ui::DisplayScaleQt& final
        {
            LogAbort()().Abort();
        }
        [[noreturn]] auto SendMonitor() noexcept
            -> ui::implementation::SendMonitor& final
        {
            LogAbort()().Abort();
        }

        auto SetCallbacks(Callbacks&&) noexcept -> void final {}

        ~Blank() final = default;
    };

    return std::make_unique<Blank>();
}
}  // namespace opentxs::factory
