// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Factory.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <exception>
#include <memory>

#include "blockchain/crypto/subaccount/notification/Imp.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/identity/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainNotificationSubaccount(
    const api::Session& api,
    const blockchain::crypto::Account& parent,
    const identifier::Account& id,
    const opentxs::PaymentCode& code,
    const identity::Nym& nym) noexcept
    -> std::shared_ptr<blockchain::crypto::internal::Subaccount>
{
    try {
        using ReturnType = blockchain::crypto::NotificationPrivate;
        auto out = std::make_shared<ReturnType>(api, parent, id, code, [&] {
            auto path = proto::HDPath{};
            nym.Internal().PaymentCodePath(path);

            return path;
        }());
        out->InitSelf(out);

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return std::make_shared<blockchain::crypto::internal::Subaccount>();
    }
}
}  // namespace opentxs::factory
