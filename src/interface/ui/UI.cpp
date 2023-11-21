// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/interface/ui/UI.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <sstream>

#include "internal/core/Core.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs::ui::internal
{
auto Row::next_index() noexcept -> std::ptrdiff_t
{
    static auto counter = std::atomic<std::ptrdiff_t>{-1};

    return ++counter;
}

auto make_progress(
    blockchain::block::Height& actual,
    blockchain::block::Height& target) noexcept -> double
{
    target = std::max<blockchain::block::Height>(1, target);
    actual = std::max<blockchain::block::Height>(0, actual);
    actual = std::min(actual, target);

    return double(100 * actual) / double(target);
}
}  // namespace opentxs::ui::internal

namespace opentxs::ui::internal::blank
{
auto blank_nym() noexcept -> const identifier::Nym&;

auto BlockchainSubaccountSource::NymID() const noexcept
    -> const identifier::Nym&
{
    return blank_nym();
}

auto BlockchainSubaccountSource::SourceID() const noexcept
    -> const identifier::Generic&
{
    return blank_nym();
}

auto BlockchainSubaccountSource::Type() const noexcept
    -> blockchain::crypto::SubaccountType
{
    return blockchain::crypto::SubaccountType::Error;
}

auto BlockchainSubaccount::NymID() const noexcept -> const identifier::Nym&
{
    return blank_nym();
}

auto BlockchainSubaccount::SubaccountID() const noexcept
    -> const identifier::Generic&
{
    return blank_nym();
}

auto BlockchainSubchain::Type() const noexcept -> blockchain::crypto::Subchain
{
    return blockchain::crypto::Subchain::Error;
}

auto blank_nym() noexcept -> const identifier::Nym&
{
    static const auto data = identifier::Nym{};

    return data;
}
}  // namespace opentxs::ui::internal::blank

namespace opentxs::ui::implementation
{
auto account_name_blockchain(blockchain::Type chain) noexcept
    -> UnallocatedCString
{
    return blockchain::AccountName(chain);
}

auto account_name_custodial(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& contract,
    UnallocatedCString&& alias) noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};

    if (false == alias.empty()) {

        out << alias;
    } else {
        const auto unit = api.Wallet().Internal().UnitDefinition(contract);
        out << unit->Name();
    }

    const auto server = api.Wallet().Internal().Server(notary);
    out << " at " << server->EffectiveName();

    return out.str();
}
}  // namespace opentxs::ui::implementation
