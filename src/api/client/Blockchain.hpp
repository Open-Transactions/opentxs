// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_CRYPTO_SUPPORTED_KEY_HD
namespace opentxs::api::client::implementation
{
class Blockchain : virtual public api::client::Blockchain
{
public:
    std::shared_ptr<proto::Bip44Account> Account(
        const identifier::Nym& nymID,
        const Identifier& accountID) const override;
    std::set<OTIdentifier> AccountList(
        const identifier::Nym& nymID,
        const proto::ContactItemType type) const override;
    std::unique_ptr<proto::Bip44Address> AllocateAddress(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::string& label = "",
        const BIP44Chain chain = EXTERNAL_CHAIN) const override;
    bool AssignAddress(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const Identifier& contactID,
        const BIP44Chain chain = EXTERNAL_CHAIN) const override;
    std::unique_ptr<proto::Bip44Address> LoadAddress(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const BIP44Chain chain) const override;
    OTIdentifier NewAccount(
        const identifier::Nym& nymID,
        const BlockchainAccountType standard,
        const proto::ContactItemType type) const override;
    bool StoreIncoming(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::uint32_t index,
        const BIP44Chain chain,
        const proto::BlockchainTransaction& transaction) const override;
    bool StoreOutgoing(
        const identifier::Nym& senderNymID,
        const Identifier& accountID,
        const Identifier& recipientContactID,
        const proto::BlockchainTransaction& transaction) const override;
    std::shared_ptr<proto::BlockchainTransaction> Transaction(
        const std::string& id) const override;

    ~Blockchain() = default;

private:
    typedef std::map<OTIdentifier, std::mutex> IDLock;

    friend opentxs::Factory;

    const api::Core& api_;
    const api::client::Activity& activity_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable IDLock account_lock_;
    proto::Bip44Address& add_address(
        const std::uint32_t index,
        proto::Bip44Account& account,
        const BIP44Chain chain) const;
    std::uint8_t address_prefix(const proto::ContactItemType type) const;

    Bip44Type bip44_type(const proto::ContactItemType type) const;
    std::string calculate_address(
        const proto::Bip44Account& account,
        const BIP44Chain chain,
        const std::uint32_t index) const;
    proto::Bip44Address& find_address(
        const std::uint32_t index,
        const BIP44Chain chain,
        proto::Bip44Account& account) const;
    void init_path(
        const std::string& root,
        const proto::ContactItemType chain,
        const std::uint32_t account,
        const BlockchainAccountType standard,
        proto::HDPath& path) const;
    std::shared_ptr<proto::Bip44Account> load_account(
        const Lock& lock,
        const std::string& nymID,
        const std::string& accountID) const;
    bool move_transactions(
        const identifier::Nym& nymID,
        const proto::Bip44Address& address,
        const std::string& fromContact,
        const std::string& toContact) const;

    Blockchain(const api::Core& api, const api::client::Activity& activity);
    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    Blockchain& operator=(Blockchain&&) = delete;
};
}  // namespace opentxs::api::client::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
