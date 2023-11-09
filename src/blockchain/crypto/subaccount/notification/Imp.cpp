// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/subaccount/notification/Imp.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <functional>
#include <memory>
#include <utility>

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"                     // IWYU pragma: keep
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto
{
NotificationPrivate::NotificationPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const opentxs::PaymentCode& code,
    proto::HDPath&& path) noexcept(false)
    : SubaccountPrivate(
          api,
          parent,
          SubaccountType::Notification,
          id,
          code.ID(),
          code.asBase58() + " (local)",
          "Notification transactions")
    , code_(code)
    , path_(std::move(path))
    , progress_()
    , self_()
{
    init(false);
}

auto NotificationPrivate::AllowedSubchains() const noexcept -> Set<Subchain>
{
    static const auto subchains = Set<Subchain>{Subchain::NotificationV3};

    return subchains;
}

auto NotificationPrivate::DisplayType() const noexcept -> SubaccountType
{
    return SubaccountType::PaymentCode;
}

auto NotificationPrivate::init(bool existing) noexcept(false) -> void
{
    SubaccountPrivate::init(existing);
    auto handle = progress_.lock();
    const auto& hash = params::get(base_chain(target_)).GenesisHash();

    for (const auto& subchain : AllowedSubchains()) {
        handle->emplace(subchain, block::Position{0, hash});
    }
}

auto NotificationPrivate::InitSelf(std::shared_ptr<Subaccount> me) noexcept
    -> void
{
    self_.emplace(me);
}

auto NotificationPrivate::ScanProgress(Subchain type) const noexcept
    -> block::Position
{
    static const auto allowed = AllowedSubchains();

    if (0u == allowed.count(type)) {
        LogError()()("Invalid subchain ")(print(type)).Flush();

        return Subaccount::ScanProgress(type);
    }

    return progress_.lock_shared()->at(type);
}

auto NotificationPrivate::SetScanProgress(
    const block::Position& progress,
    Subchain type) noexcept -> void
{
    progress_.lock()->operator[](type) = progress;
}
}  // namespace opentxs::blockchain::crypto
