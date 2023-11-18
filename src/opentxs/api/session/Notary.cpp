// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/Notary.hpp"  // IWYU pragma: associated

#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Notary.internal.hpp"

namespace opentxs::api::session
{
Notary::Notary(api::internal::Session* imp) noexcept
    : Session(imp)
{
}

auto Notary::DropIncoming(const int count) const -> void
{
    imp_->asNotary().DropIncoming(count);
}

auto Notary::DropOutgoing(const int count) const -> void
{
    imp_->asNotary().DropOutgoing(count);
}

auto Notary::GetAdminNym() const -> UnallocatedCString
{
    return imp_->asNotary().GetAdminNym();
}

auto Notary::GetAdminPassword() const -> UnallocatedCString
{
    return imp_->asNotary().GetAdminPassword();
}

auto Notary::GetOwnerName() const -> std::string_view
{
    return imp_->asNotary().GetOwnerName();
}

auto Notary::GetPrivateMint(
    const identifier::UnitDefinition& unitid,
    std::uint32_t series) const noexcept -> otx::blind::Mint&
{
    return imp_->asNotary().GetPrivateMint(unitid, series);
}

auto Notary::GetPublicMint(const identifier::UnitDefinition& unitID)
    const noexcept -> otx::blind::Mint&
{
    return imp_->asNotary().GetPublicMint(unitID);
}

auto Notary::GetUserTerms() const -> std::string_view
{
    return imp_->asNotary().GetUserTerms();
}

auto Notary::ID() const -> const identifier::Notary&
{
    return imp_->asNotary().ID();
}

auto Notary::NymID() const -> const identifier::Nym&
{
    return imp_->asNotary().NymID();
}

auto Notary::Server() const -> opentxs::server::Server&
{
    return imp_->asNotary().Server();
}

auto Notary::SetMintKeySize(const std::size_t size) const -> void
{
    imp_->asNotary().SetMintKeySize(size);
}

auto Notary::UpdateMint(const identifier::UnitDefinition& unitID) const -> void
{
    imp_->asNotary().UpdateMint(unitID);
}

Notary::~Notary() = default;
}  // namespace opentxs::api::session
