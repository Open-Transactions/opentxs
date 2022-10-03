// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Account.hpp"  // IWYU pragma: associated

#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class Bip47Channel;
class HDAccount;
class HDPath;
}  // namespace proto

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
template <>
struct Account::Factory<crypto::HD, proto::HDPath, HDProtocol, PasswordPrompt> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Generic& id,
        const proto::HDPath& data,
        const HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> std::unique_ptr<crypto::HD>
    {
        return factory::BlockchainHDSubaccount(
            api, parent, data, standard, reason, id);
    }
};
template <>
struct Account::Factory<crypto::HD, proto::HDAccount> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Generic& id,
        const proto::HDAccount& data) noexcept -> std::unique_ptr<crypto::HD>
    {
        return factory::BlockchainHDSubaccount(api, parent, data, id);
    }
};

template <>
struct Account::
    Factory<crypto::Notification, opentxs::PaymentCode, identity::Nym> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Generic& id,
        const opentxs::PaymentCode& code,
        const identity::Nym& nym) noexcept
        -> std::unique_ptr<crypto::Notification>
    {
        return factory::BlockchainNotificationSubaccount(
            api, parent, code, nym, id);
    }
};

template <>
struct Account::Factory<
    crypto::PaymentCode,
    api::session::Contacts,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    PasswordPrompt> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Generic& id,
        const api::session::Contacts& contacts,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        static const auto blank = api.Factory().Data();

        return factory::BlockchainPCSubaccount(
            api, contacts, parent, local, remote, path, blank, reason, id);
    }
};
template <>
struct Account::Factory<
    crypto::PaymentCode,
    api::session::Contacts,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    opentxs::blockchain::block::Txid,
    PasswordPrompt> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Generic& id,
        const api::session::Contacts& contacts,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::Txid& txid,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        return factory::BlockchainPCSubaccount(
            api, contacts, parent, local, remote, path, txid, reason, id);
    }
};
template <>
struct Account::
    Factory<crypto::PaymentCode, api::session::Contacts, proto::Bip47Channel> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Generic& id,
        const api::session::Contacts& contacts,
        const proto::Bip47Channel& data) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        return factory::BlockchainPCSubaccount(api, contacts, parent, data, id);
    }
};

template <typename InterfaceType, typename PayloadType>
auto Account::NodeGroup<InterfaceType, PayloadType>::add(
    const Lock& lock,
    const identifier::Generic& id,
    std::unique_ptr<PayloadType> node) noexcept -> bool
{
    if (false == bool(node)) {
        LogError()(OT_PRETTY_CLASS())("Invalid node").Flush();

        return false;
    }

    if (0 < index_.count(id)) {
        LogError()(OT_PRETTY_CLASS())("Index already exists").Flush();

        return false;
    }

    nodes_.emplace_back(std::move(node));
    const auto position = nodes_.size() - 1_uz;
    index_.emplace(id, position);

    return true;
}
}  // namespace opentxs::blockchain::crypto::implementation
