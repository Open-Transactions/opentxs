// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Notification.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <memory>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainNotificationSubaccount(
    const api::Session& api,
    const blockchain::crypto::Account& parent,
    const opentxs::PaymentCode& code,
    const identity::Nym& nym,
    identifier::Generic& id) noexcept
    -> std::unique_ptr<blockchain::crypto::Notification>
{
    using ReturnType = blockchain::crypto::implementation::Notification;

    return std::make_unique<ReturnType>(
        api,
        parent,
        code,
        [&] {
            auto out = proto::HDPath{};
            nym.Internal().PaymentCodePath(out);

            return out;
        }(),
        id);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::crypto::implementation
{
Notification::Notification(
    const api::Session& api,
    const crypto::Account& parent,
    const opentxs::PaymentCode& code,
    proto::HDPath&& path,
    identifier::Generic& out) noexcept
    : Subaccount(
          api,
          parent,
          SubaccountType::Notification,
          calculate_id(api, parent.Chain(), code),
          out)
    , code_(code)
    , path_(std::move(path))
    , progress_()
{
    init();
}

auto Notification::AllowedSubchains() const noexcept -> UnallocatedSet<Subchain>
{
    static const auto subchains =
        UnallocatedSet<Subchain>{Subchain::NotificationV3};

    return subchains;
}

auto Notification::calculate_id(
    const api::Session& api,
    const blockchain::Type chain,
    const opentxs::PaymentCode& code) noexcept -> identifier::Generic
{
    auto preimage = api.Factory().DataFromBytes(code.ID().Bytes());
    preimage.Concatenate(&chain, sizeof(chain));

    return api.Factory().IdentifierFromPreimage(preimage.Bytes());
}

auto Notification::init() noexcept -> void
{
    Subaccount::init();
    auto handle = progress_.lock();
    const auto& hash = params::get(chain_).GenesisHash();

    for (const auto& subchain : AllowedSubchains()) {
        handle->emplace(subchain, block::Position{0, hash});
    }
}

auto Notification::ScanProgress(Subchain type) const noexcept -> block::Position
{
    static const auto allowed = AllowedSubchains();

    if (0u == allowed.count(type)) {
        LogError()(OT_PRETTY_CLASS())("Invalid subchain ")(print(type)).Flush();

        return Subaccount::ScanProgress(type);
    }

    return progress_.lock_shared()->at(type);
}

auto Notification::SetScanProgress(
    const block::Position& progress,
    Subchain type) noexcept -> void
{
    progress_.lock()->operator[](type) = progress;
}
}  // namespace opentxs::blockchain::crypto::implementation
