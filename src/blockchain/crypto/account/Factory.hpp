// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Factory.hpp"

#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Account;
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

namespace opentxs::blockchain::crypto::account
{
template <typename ReturnType, typename... Args>
struct Factory {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Account& id,
        const Args&... args) noexcept -> std::unique_ptr<ReturnType>;
};

template <>
struct Factory<crypto::HD, proto::HDPath, HDProtocol, PasswordPrompt> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Account& id,
        const proto::HDPath& data,
        const HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> std::unique_ptr<crypto::HD>
    {
        return factory::BlockchainHDSubaccount(
            api, parent, data, standard, reason, id);
    }
};

template <>
struct Factory<crypto::HD, proto::HDAccount> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Account& id,
        const proto::HDAccount& data) noexcept -> std::unique_ptr<crypto::HD>
    {
        return factory::BlockchainHDSubaccount(api, parent, data, id);
    }
};

template <>
struct Factory<crypto::Notification, opentxs::PaymentCode, identity::Nym> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Account& id,
        const opentxs::PaymentCode& code,
        const identity::Nym& nym) noexcept
        -> std::unique_ptr<crypto::Notification>
    {
        return factory::BlockchainNotificationSubaccount(
            api, parent, code, nym, id);
    }
};

template <>
struct Factory<
    crypto::PaymentCode,
    api::session::Contacts,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    PasswordPrompt> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Account& id,
        const api::session::Contacts& contacts,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        static const auto blank = block::TransactionHash{};

        return factory::BlockchainPCSubaccount(
            api, contacts, parent, local, remote, path, blank, reason, id);
    }
};

template <>
struct Factory<
    crypto::PaymentCode,
    api::session::Contacts,
    opentxs::PaymentCode,
    opentxs::PaymentCode,
    proto::HDPath,
    opentxs::blockchain::block::TransactionHash,
    PasswordPrompt> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Account& id,
        const api::session::Contacts& contacts,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::TransactionHash& txid,
        const PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        return factory::BlockchainPCSubaccount(
            api, contacts, parent, local, remote, path, txid, reason, id);
    }
};

template <>
struct Factory<
    crypto::PaymentCode,
    api::session::Contacts,
    proto::Bip47Channel> {
    static auto get(
        const api::Session& api,
        const crypto::Account& parent,
        identifier::Account& id,
        const api::session::Contacts& contacts,
        const proto::Bip47Channel& data) noexcept
        -> std::unique_ptr<crypto::PaymentCode>
    {
        return factory::BlockchainPCSubaccount(api, contacts, parent, data, id);
    }
};
}  // namespace opentxs::blockchain::crypto::account
