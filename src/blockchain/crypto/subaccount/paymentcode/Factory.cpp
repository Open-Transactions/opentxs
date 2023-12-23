// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Factory.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/crypto/subaccount/paymentcode/Imp.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainPCSubaccount(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const blockchain::crypto::Account& parent,
    const identifier::Account& id,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const PasswordPrompt& reason) noexcept
    -> std::shared_ptr<blockchain::crypto::internal::Subaccount>
{
    using ReturnType = blockchain::crypto::PaymentCodePrivate;

    try {
        auto out = std::make_shared<ReturnType>(
            api, contacts, parent, id, local, remote, path, reason);
        out->InitSelf(out);

        return out;
    } catch (const std::exception& e) {
        LogVerbose()()(e.what()).Flush();

        return std::make_shared<blockchain::crypto::internal::Subaccount>();
    }
}

auto BlockchainPCSubaccount(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const blockchain::crypto::Account& parent,
    const identifier::Account& id,
    const proto::Bip47Channel& serialized) noexcept
    -> std::shared_ptr<blockchain::crypto::internal::Subaccount>
{
    using ReturnType = blockchain::crypto::PaymentCodePrivate;
    auto contact = contacts.PaymentCodeToContact(
        api.Factory().Internal().Session().PaymentCode(serialized.remote()),
        blockchain::crypto::target_to_unit(parent.Target()));

    assert_false(contact.empty());

    try {
        auto out = std::make_shared<ReturnType>(
            api, contacts, parent, id, serialized, std::move(contact));
        out->InitSelf(out);

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return std::make_shared<blockchain::crypto::internal::Subaccount>();
    }
}
}  // namespace opentxs::factory
