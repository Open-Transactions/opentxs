// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Subaccount.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "internal/blockchain/crypto/Deterministic.hpp"
#include "internal/blockchain/crypto/Imported.hpp"
#include "internal/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto::internal
{
auto Subaccount::AllowedSubchains() const noexcept -> Set<Subchain>
{
    return {};
}

auto Subaccount::asDeterministic() const noexcept
    -> const internal::Deterministic&
{
    return const_cast<Subaccount*>(this)->asDeterministic();
}

auto Subaccount::asDeterministic() noexcept -> internal::Deterministic&
{
    return internal::Deterministic::Blank();
}

auto Subaccount::asDeterministicPublic() const noexcept
    -> const crypto::Deterministic&
{
    return const_cast<Subaccount*>(this)->asDeterministicPublic();
}

auto Subaccount::asDeterministicPublic() noexcept -> crypto::Deterministic&
{
    return crypto::Deterministic::Blank();
}

auto Subaccount::asImported() const noexcept -> const internal::Imported&
{
    return const_cast<Subaccount*>(this)->asImported();
}

auto Subaccount::asImported() noexcept -> internal::Imported&
{
    return internal::Imported::Blank();
}

auto Subaccount::asImportedPublic() const noexcept -> const crypto::Imported&
{
    return const_cast<Subaccount*>(this)->asImportedPublic();
}

auto Subaccount::asImportedPublic() noexcept -> crypto::Imported&
{
    return crypto::Imported::Blank();
}

auto Subaccount::asNotification() const noexcept
    -> const internal::Notification&
{
    return const_cast<Subaccount*>(this)->asNotification();
}

auto Subaccount::asNotification() noexcept -> internal::Notification&
{
    return internal::Notification::Blank();
}

auto Subaccount::asNotificationPublic() const noexcept
    -> const crypto::Notification&
{
    return const_cast<Subaccount*>(this)->asNotificationPublic();
}

auto Subaccount::asNotificationPublic() noexcept -> crypto::Notification&
{
    return crypto::Notification::Blank();
}

auto Subaccount::BalanceElement(const Subchain, const Bip32Index) const
    noexcept(false) -> const crypto::Element&
{
    throw std::out_of_range{""};
}

auto Subaccount::Blank() noexcept -> Subaccount&
{
    static auto blank = Subaccount{};

    return blank;
}

auto Subaccount::Confirm(
    const Subchain,
    const Bip32Index,
    const block::TransactionHash&) noexcept -> bool
{
    return {};
}

auto Subaccount::Describe() const noexcept -> std::string_view { return {}; }

auto Subaccount::DisplayName() const noexcept -> std::string_view { return {}; }

auto Subaccount::ID() const noexcept -> const identifier::Account&
{
    static const auto blank = identifier::Account{};

    return blank;
}

auto Subaccount::InitSelf(std::shared_ptr<Subaccount>) noexcept -> void {}

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

auto Subaccount::Self() const noexcept -> const crypto::Subaccount&
{
    return const_cast<Subaccount*>(this)->Self();
}

auto Subaccount::Self() noexcept -> crypto::Subaccount&
{
    return crypto::Subaccount::Blank();
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

auto Subaccount::Source() const noexcept -> const identifier::Generic&
{
    return ID();
}

auto Subaccount::SourceDescription() const noexcept -> std::string_view
{
    return {};
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
