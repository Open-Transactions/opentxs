// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/common/Base.hpp"  // IWYU pragma: associated

#include <future>

#include "internal/otx/client/Pair.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
Base::Base() noexcept
    : ot_(OTTestEnvironment::GetOT())
{
}

auto Base::CurrencyContract(
    const ot::api::Session& api,
    const ot::UnallocatedCString& nymid,
    const ot::UnallocatedCString& shortname,
    const ot::UnallocatedCString& terms,
    const ot::UnitType unitOfAccount,
    const ot::Amount& redemptionIncrement,
    const ot::PasswordPrompt& reason) noexcept(false) -> ot::OTUnitDefinition
{
    return api.Wallet().Internal().CurrencyContract(
        nymid, shortname, terms, unitOfAccount, redemptionIncrement, reason);
}

auto Base::InternalWallet(const ot::api::Session& api) noexcept
    -> const ot::api::session::internal::Wallet&
{
    return api.Wallet().Internal();
}

auto Base::NotaryContract(
    const ot::api::Session& api,
    const ot::identifier::Notary& id) noexcept -> ot::OTServerContract
{
    return api.Wallet().Internal().Server(id);
}

auto Base::NotaryContract(
    const ot::api::Session& api,
    const ot::UnallocatedCString& nymid,
    const ot::UnallocatedCString& name,
    const ot::UnallocatedCString& terms,
    const ot::UnallocatedList<ot::contract::Server::Endpoint>& endpoints,
    const ot::PasswordPrompt& reason,
    const ot::VersionNumber version) noexcept -> ot::OTServerContract
{
    return api.Wallet().Internal().Server(
        nymid, name, terms, endpoints, reason, version);
}

auto Base::StopPair(const ot::api::session::Client& api) noexcept -> void
{
    api.Internal().asClient().Pair().Stop().get();
}
}  // namespace ottest
