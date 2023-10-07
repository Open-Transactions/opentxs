// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Subaccount.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto::internal
{
auto Subaccount::AllowedSubchains() const noexcept -> UnallocatedSet<Subchain>
{
    return {};
}

auto Subaccount::BalanceElement(const Subchain, const Bip32Index) const
    noexcept(false) -> const crypto::Element&
{
    throw std::out_of_range{""};
}

auto Subaccount::Confirm(
    const Subchain,
    const Bip32Index,
    const block::TransactionHash&) noexcept -> bool
{
    return {};
}

auto Subaccount::Describe() const noexcept -> std::string_view { return {}; }

auto Subaccount::ID() const noexcept -> const identifier::Account&
{
    static const auto blank = identifier::Account{};

    return blank;
}

auto Subaccount::IsValid() const noexcept -> bool { return false; }

auto Subaccount::Parent() const noexcept -> const crypto::Account&
{
    LogAbort()().Abort();  // TODO
}

auto Subaccount::PrivateKey(
    const implementation::Element&,
    const Subchain,
    const Bip32Index,
    const PasswordPrompt&) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
}

auto Subaccount::ScanProgress(Subchain) const noexcept -> block::Position
{
    return {};
}

auto Subaccount::SetContact(
    const Subchain,
    const Bip32Index,
    const identifier::Generic&) noexcept(false) -> bool
{
    return {};
}

auto Subaccount::SetLabel(
    const Subchain,
    const Bip32Index,
    const std::string_view) noexcept(false) -> bool
{
    return {};
}

auto Subaccount::SetScanProgress(const block::Position&, Subchain) noexcept
    -> void
{
}

auto Subaccount::Type() const noexcept -> SubaccountType { return {}; }

auto Subaccount::Unconfirm(
    const Subchain,
    const Bip32Index,
    const block::TransactionHash&,
    const Time) noexcept -> bool
{
    return {};
}

auto Subaccount::Unreserve(const Subchain, const Bip32Index) noexcept -> bool
{
    return {};
}

auto Subaccount::UpdateElement(UnallocatedVector<ReadView>&) const noexcept
    -> void
{
}
}  // namespace opentxs::blockchain::crypto::internal
