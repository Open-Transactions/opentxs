// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Notification.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>

#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Types.internal.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/AccountSubtype.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/Types.hpp"

namespace opentxs::blockchain::crypto::internal
{
auto Notification::Blank() noexcept -> Notification&
{
    static auto blank = Notification{};

    return blank;
}

auto Notification::CalculateID(
    const api::Session& api,
    const crypto::Target& target,
    const opentxs::PaymentCode& code) noexcept -> identifier::Account
{
    using enum identifier::AccountSubtype;
    auto preimage = api.Factory().DataFromBytes(code.ID().Bytes());
    serialize(target, preimage);

    return api.Factory().AccountIDFromPreimage(
        preimage.Bytes(), blockchain_subaccount);
}

auto Notification::LocalPaymentCode() const noexcept
    -> const opentxs::PaymentCode&
{
    static const auto blank = opentxs::PaymentCode{};

    return blank;
}

auto Notification::Path() const noexcept -> proto::HDPath { return {}; }
}  // namespace opentxs::blockchain::crypto::internal
