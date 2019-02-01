// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain
{
public:
    virtual std::shared_ptr<proto::Bip44Account> Account(
        const identifier::Nym& nymID,
        const Identifier& accountID) const = 0;
    virtual std::set<OTIdentifier> AccountList(
        const identifier::Nym& nymID,
        const proto::ContactItemType type) const = 0;
    virtual std::unique_ptr<proto::Bip44Address> AllocateAddress(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::string& label = "",
        const BIP44Chain chain = EXTERNAL_CHAIN) const = 0;
    virtual bool AssignAddress(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const Identifier& contactID,
        const BIP44Chain chain = EXTERNAL_CHAIN) const = 0;
    virtual std::unique_ptr<proto::Bip44Address> LoadAddress(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const BIP44Chain chain) const = 0;
    virtual OTIdentifier NewAccount(
        const identifier::Nym& nymID,
        const BlockchainAccountType standard,
        const proto::ContactItemType type) const = 0;
    virtual bool StoreIncoming(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const BIP44Chain chain,
        const proto::BlockchainTransaction& transaction) const = 0;
    virtual bool StoreOutgoing(
        const identifier::Nym& senderNymID,
        const Identifier& accountID,
        const Identifier& recipientContactID,
        const proto::BlockchainTransaction& transaction) const = 0;
    virtual std::shared_ptr<proto::BlockchainTransaction> Transaction(
        const std::string& id) const = 0;

    virtual ~Blockchain() = default;

protected:
    Blockchain() = default;

private:
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    Blockchain& operator=(Blockchain&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#endif
