/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "stdafx.hpp"

#include "opentxs/api/Blockchain.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Activity.hpp"
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#define LOCK_ACCOUNT()                                                         \
    Lock mapLock(lock_);                                                       \
    auto& accountMutex = account_lock_[accountID];                             \
    mapLock.unlock();                                                          \
    Lock accountLock(accountMutex);

#define LOCK_NYM()                                                             \
    Lock mapLock(lock_);                                                       \
    auto& nymMutex = nym_lock_[nymID];                                         \
    mapLock.unlock();                                                          \
    Lock nymLock(nymMutex);

#define MAX_INDEX 2147483648
#define BLOCKCHAIN_VERSION 1
#define ACCOUNT_VERSION 1
#define PATH_VERSION 1
#define COMPRESSED_PUBKEY_SIZE 33
#define BITCOIN_PUBKEY_HASH 0x0
#define BITCOIN_TESTNET_HASH 0x6f
#define LITECOIN_PUBKEY_HASH 0x30
#define DOGECOIN_PUBKEY_HASH 0x1e
#define DASH_PUBKEY_HASH 0x4c

#define OT_METHOD "opentxs::Blockchain::"

namespace opentxs::api
{
Blockchain::Blockchain(
    const Activity& activity,
    const Crypto& crypto,
    const storage::Storage& storage,
    const client::Wallet& wallet)
    : activity_(activity)
    , crypto_(crypto)
    , storage_(storage)
    , wallet_(wallet)
    , lock_()
    , nym_lock_()
    , account_lock_()
{
}

std::shared_ptr<proto::Bip44Account> Blockchain::Account(
    const Identifier& nymID,
    const Identifier& accountID) const
{
    LOCK_ACCOUNT()

    const std::string sNymID = nymID.str();
    const std::string sAccountID = accountID.str();
    auto account = load_account(accountLock, sNymID, sAccountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account does not exist."
              << std::endl;
    }

    return account;
}

std::set<OTIdentifier> Blockchain::AccountList(
    const Identifier& nymID,
    const proto::ContactItemType type) const
{
    std::set<OTIdentifier> output;
    auto list = storage_.BlockchainAccountList(nymID.str(), type);

    for (const auto& accountID : list) {
        // output.emplace(String(accountID.c_str()));
        output.emplace(Identifier::Factory(accountID));
    }

    return output;
}

proto::Bip44Address& Blockchain::add_address(
    const std::uint32_t index,
    proto::Bip44Account& account,
    const BIP44Chain chain) const
{
    OT_ASSERT(MAX_INDEX >= index);

    account.set_revision(account.revision() + 1);

    if (chain) {
        account.set_internalindex(index + 1);

        return *account.add_internaladdress();
    } else {
        account.set_externalindex(index + 1);

        return *account.add_externaladdress();
    }
}

std::uint8_t Blockchain::address_prefix(const proto::ContactItemType type) const
{
    switch (type) {
        case proto::CITEMTYPE_BCH:
        case proto::CITEMTYPE_BTC: {
            return BITCOIN_PUBKEY_HASH;
        }
        case proto::CITEMTYPE_LTC: {
            return LITECOIN_PUBKEY_HASH;
        }
        case proto::CITEMTYPE_DOGE: {
            return DOGECOIN_PUBKEY_HASH;
        }
        case proto::CITEMTYPE_DASH: {
            return DASH_PUBKEY_HASH;
        }
        case proto::CITEMTYPE_TNBCH:
        case proto::CITEMTYPE_TNBTC:
        case proto::CITEMTYPE_TNXRP:
        case proto::CITEMTYPE_TNLTC:
        case proto::CITEMTYPE_TNXEM:
        case proto::CITEMTYPE_TNDASH:
        case proto::CITEMTYPE_TNMAID:
        case proto::CITEMTYPE_TNLSK:
        case proto::CITEMTYPE_TNDOGE:
        case proto::CITEMTYPE_TNXMR:
        case proto::CITEMTYPE_TNWAVES:
        case proto::CITEMTYPE_TNNXT:
        case proto::CITEMTYPE_TNSC:
        case proto::CITEMTYPE_TNSTEEM: {
            return BITCOIN_TESTNET_HASH;
        }
        default: {
            OT_FAIL;
        }
    }

    return 0x0;
}

std::unique_ptr<proto::Bip44Address> Blockchain::AllocateAddress(
    const Identifier& nymID,
    const Identifier& accountID,
    const std::string& label,
    const BIP44Chain chain) const
{
    LOCK_ACCOUNT()

    const std::string sNymID = nymID.str();
    const std::string sAccountID = accountID.str();
    std::unique_ptr<proto::Bip44Address> output{nullptr};
    auto account = load_account(accountLock, sNymID, sAccountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account does not exist."
              << std::endl;

        return output;
    }

    const auto& type = account->type();
    const auto index =
        chain ? account->internalindex() : account->externalindex();

    if (MAX_INDEX == index) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account is full." << std::endl;

        return output;
    }

    auto& newAddress = add_address(index, *account, chain);
    newAddress.set_version(BLOCKCHAIN_VERSION);
    newAddress.set_index(index);
    newAddress.set_address(calculate_address(*account, chain, index));

    OT_ASSERT(false == newAddress.address().empty());

    otErr << OT_METHOD << __FUNCTION__ << ": Address " << newAddress.address()
          << " allocated." << std::endl;
    newAddress.set_label(label);
    const auto saved = storage_.Store(sNymID, type, *account);

    if (false == saved) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to save account."
              << std::endl;

        return output;
    }

    output.reset(new proto::Bip44Address(newAddress));

    return output;
}

bool Blockchain::AssignAddress(
    const Identifier& nymID,
    const Identifier& accountID,
    const std::uint32_t index,
    const Identifier& contactID,
    const BIP44Chain chain) const
{
    LOCK_ACCOUNT()

    const std::string sNymID = nymID.str();
    const std::string sAccountID = accountID.str();
    const std::string sContactID = contactID.str();
    auto account = load_account(accountLock, sNymID, sAccountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account does not exist."
              << std::endl;

        return false;
    }

    const auto& type = account->type();
    const auto allocatedIndex =
        chain ? account->internalindex() : account->externalindex();

    if (index > allocatedIndex) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Address has not been allocated." << std::endl;

        return false;
    }

    auto& address = find_address(index, chain, *account);
    const auto& existing = address.contact();

    if (false == existing.empty()) {
        move_transactions(nymID, address, existing, sContactID);
    }

    address.set_contact(sContactID);
    account->set_revision(account->revision() + 1);

    // check: does the activity thread exist between nym and contact?
    bool threadExists = false;
    const auto threadList = storage_.ThreadList(sNymID, false);
    for (const auto it : threadList) {
        const auto& id = it.first;

        if (id == sContactID) {
            threadExists = true;
        }
    }

    if (threadExists) {
        // check: does every incoming transaction exist as an activity
        std::shared_ptr<proto::StorageThread> thread =
            activity_.Thread(nymID, contactID);
        for (const std::string& txID : address.incoming()) {
            bool exists = false;
            for (const auto activity : thread->item())
                if (txID.compare(activity.id()) == 0) exists = true;

            // add: transaction to the thread
            if (!exists)
                activity_.AddBlockchainTransaction(
                    nymID,
                    contactID,
                    StorageBox::INCOMINGBLOCKCHAIN,
                    *Transaction(txID));
        }
    } else {
        // create the thread and add the transactions
        for (const auto txID : address.incoming()) {
            activity_.AddBlockchainTransaction(
                nymID,
                contactID,
                StorageBox::INCOMINGBLOCKCHAIN,
                *Transaction(txID));
        }
    }

    return storage_.Store(sNymID, type, *account);
}

Bip44Type Blockchain::bip44_type(const proto::ContactItemType type) const
{
    switch (type) {
        case proto::CITEMTYPE_BTC: {

            return Bip44Type::BITCOIN;
        }
        case proto::CITEMTYPE_LTC: {

            return Bip44Type::LITECOIN;
        }
        case proto::CITEMTYPE_DOGE: {

            return Bip44Type::DOGECOIN;
        }
        case proto::CITEMTYPE_DASH: {

            return Bip44Type::DASH;
        }
        case proto::CITEMTYPE_BCH: {

            return Bip44Type::BITCOINCASH;
        }
        case proto::CITEMTYPE_TNBCH:
        case proto::CITEMTYPE_TNBTC:
        case proto::CITEMTYPE_TNXRP:
        case proto::CITEMTYPE_TNLTC:
        case proto::CITEMTYPE_TNXEM:
        case proto::CITEMTYPE_TNDASH:
        case proto::CITEMTYPE_TNMAID:
        case proto::CITEMTYPE_TNLSK:
        case proto::CITEMTYPE_TNDOGE:
        case proto::CITEMTYPE_TNXMR:
        case proto::CITEMTYPE_TNWAVES:
        case proto::CITEMTYPE_TNNXT:
        case proto::CITEMTYPE_TNSC:
        case proto::CITEMTYPE_TNSTEEM: {
            return Bip44Type::TESTNET;
        }
        default: {
            OT_FAIL;
        }
    }

    return {};
}

std::string Blockchain::calculate_address(
    const proto::Bip44Account& account,
    const BIP44Chain chain,
    const std::uint32_t index) const
{
    const auto& path = account.path();
    auto fingerprint = path.root();
    auto serialized = crypto_.BIP32().AccountChildKey(path, chain, index);

    if (false == bool(serialized)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to derive key."
              << std::endl;

        return {};
    }

    std::unique_ptr<OTAsymmetricKey> key{nullptr};
    std::unique_ptr<AsymmetricKeySecp256k1> ecKey{nullptr};
    key.reset(OTAsymmetricKey::KeyFactory(*serialized));

    if (false == bool(key)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to instantiate key."
              << std::endl;

        return {};
    }

    ecKey.reset(dynamic_cast<AsymmetricKeySecp256k1*>(key.release()));

    if (false == bool(ecKey)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect key type."
              << std::endl;

        return {};
    }

    auto pubkey = Data::Factory();

    if (false == ecKey->GetPublicKey(pubkey)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to extract public key."
              << std::endl;

        return {};
    }

    if (COMPRESSED_PUBKEY_SIZE != pubkey->GetSize()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect pubkey size ("
              << pubkey->GetSize() << ")." << std::endl;

        return {};
    }

    auto sha256 = Data::Factory();
    auto ripemd160 = Data::Factory();
    auto pubkeyHash = Data::Factory();

    if (!crypto_.Hash().Digest(proto::HASHTYPE_SHA256, pubkey, sha256)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to calculate sha256."
              << std::endl;

        return {};
    }

    if (!crypto_.Hash().Digest(proto::HASHTYPE_RIMEMD160, sha256, pubkeyHash)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to calculate rimemd160."
              << std::endl;

        return {};
    }

    const auto prefix = address_prefix(account.type());
    auto preimage = Data::Factory(&prefix, sizeof(prefix));

    OT_ASSERT(1 == preimage->GetSize());

    preimage += pubkeyHash;

    OT_ASSERT(21 == preimage->GetSize());

    return crypto_.Encode().IdentifierEncode(preimage);
}

proto::Bip44Address& Blockchain::find_address(
    const std::uint32_t index,
    const BIP44Chain chain,
    proto::Bip44Account& account) const
{
    // TODO: determine if we can prove the addresses were inserted in index
    // order. Then we don't need to do a linear search here.
    // Perhaps opentxs-proto should fail validation for improperly sorted
    // Bip44Accounts

    if (chain) {
        for (auto& address : *account.mutable_internaladdress()) {
            if (address.index() == index) {
                return address;
            }
        }
    } else {
        for (auto& address : *account.mutable_externaladdress()) {
            if (address.index() == index) {
                return address;
            }
        }
    }

    OT_FAIL;
}

void Blockchain::init_path(
    const std::string& root,
    const proto::ContactItemType chain,
    const std::uint32_t account,
    const BlockchainAccountType standard,
    proto::HDPath& path) const
{
    path.set_version(PATH_VERSION);
    path.set_root(root);

    switch (standard) {
        case BlockchainAccountType::BIP32: {
            path.add_child(
                account | static_cast<std::uint32_t>(Bip32Child::HARDENED));
        } break;
        case BlockchainAccountType::BIP44: {
            path.add_child(
                static_cast<std::uint32_t>(Bip43Purpose::HDWALLET) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            path.add_child(
                static_cast<std::uint32_t>(bip44_type(chain)) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            path.add_child(account);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

std::shared_ptr<proto::Bip44Account> Blockchain::load_account(
    const Lock&,
    const std::string& nymID,
    const std::string& accountID) const
{
    std::shared_ptr<proto::Bip44Account> account{nullptr};
    storage_.Load(nymID, accountID, account);

    return account;
}

std::unique_ptr<proto::Bip44Address> Blockchain::LoadAddress(
    const Identifier& nymID,
    const Identifier& accountID,
    const std::uint32_t index,
    const BIP44Chain chain) const
{
    LOCK_ACCOUNT()

    std::unique_ptr<proto::Bip44Address> output{};
    const std::string sNymID = nymID.str();
    const std::string sAccountID = accountID.str();
    auto account = load_account(accountLock, sNymID, sAccountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account does not exist."
              << std::endl;

        return output;
    }

    const auto allocatedIndex =
        chain ? account->internalindex() : account->externalindex();

    if (index > allocatedIndex) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Address has not been allocated." << std::endl;

        return output;
    }

    auto& address = find_address(index, chain, *account);
    output.reset(new proto::Bip44Address(address));

    return output;
}

bool Blockchain::move_transactions(
    const Identifier& nymID,
    const proto::Bip44Address& address,
    const std::string& fromContact,
    const std::string& toContact) const
{
    bool output{true};

    for (const auto& txid : address.incoming()) {
        output &= activity_.MoveIncomingBlockchainTransaction(
            nymID,
            Identifier::Factory(fromContact),
            Identifier::Factory(toContact),
            txid);
    }

    return output;
}

OTIdentifier Blockchain::NewAccount(
    const Identifier& nymID,
    const BlockchainAccountType standard,
    const proto::ContactItemType type) const
{
    LOCK_NYM()

    const std::string sNymID = nymID.str();
    auto existing = storage_.BlockchainAccountList(sNymID, type);

    if (0 < existing.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account already exists."
              << std::endl;

        return Identifier::Factory(*existing.begin());
    }

    auto nym = wallet_.Nym(nymID);

    if (false == bool(nym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym does not exist."
              << std::endl;

        return Identifier::Factory();
    }

    proto::HDPath nymPath{};

    if (false == nym->Path(nymPath)) {
        otErr << OT_METHOD << __FUNCTION__ << ": No nym path." << std::endl;

        return Identifier::Factory();
    }

    if (0 == nymPath.root().size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Missing root." << std::endl;

        return Identifier::Factory();
    }

    if (2 > nymPath.child().size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid path." << std::endl;

        return Identifier::Factory();
    }

    proto::HDPath accountPath{};
    init_path(
        nymPath.root(),
        type,
        nymPath.child(1) | static_cast<std::uint32_t>(Bip32Child::HARDENED),
        standard,
        accountPath);
    const auto accountID = Identifier::Factory(type, accountPath);
    Lock accountLock(account_lock_[accountID]);
    proto::Bip44Account account{};
    account.set_version(ACCOUNT_VERSION);
    account.set_id(accountID->str());
    account.set_type(type);
    account.set_revision(0);
    *account.mutable_path() = accountPath;
    account.set_internalindex(0);
    account.set_externalindex(0);
    account.clear_internaladdress();
    account.clear_externaladdress();

    const bool saved = storage_.Store(sNymID, type, account);

    if (saved) {
        return accountID;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Failed to save account."
          << std::endl;

    return Identifier::Factory();
}

bool Blockchain::StoreIncoming(
    const Identifier& nymID,
    const Identifier& accountID,
    const std::uint32_t index,
    const BIP44Chain chain,
    const proto::BlockchainTransaction& transaction) const
{
    LOCK_ACCOUNT()

    const std::string sNymID = nymID.str();
    const std::string sAccountID = accountID.str();
    auto account = load_account(accountLock, sNymID, sAccountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account does not exist."
              << std::endl;

        return false;
    }

    const auto allocatedIndex =
        chain ? account->internalindex() : account->externalindex();

    if (index > allocatedIndex) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Address has not been allocated." << std::endl;

        return false;
    }

    auto& address = find_address(index, chain, *account);
    bool exists = false;

    for (const auto& txid : address.incoming()) {
        if (txid == transaction.txid()) {
            exists = true;
            break;
        }
    }

    if (false == exists) {
        address.add_incoming(transaction.txid());
    }

    auto saved = storage_.Store(sNymID, account->type(), *account);

    if (false == saved) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to save account."
              << std::endl;

        return false;
    }

    saved = storage_.Store(transaction);

    if (false == saved) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to save transaction."
              << std::endl;

        return false;
    }

    if (address.contact().empty()) {
        return true;
    }

    const auto contactID = Identifier::Factory(address.contact());

    return activity_.AddBlockchainTransaction(
        nymID, contactID, StorageBox::INCOMINGBLOCKCHAIN, transaction);
}

bool Blockchain::StoreOutgoing(
    const Identifier& senderNymID,
    const Identifier& accountID,
    const Identifier& recipientContactID,
    const proto::BlockchainTransaction& transaction) const
{
    LOCK_ACCOUNT()

    const std::string sNymID = senderNymID.str();
    const std::string sAccountID = accountID.str();
    auto account = load_account(accountLock, sNymID, sAccountID);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Account does not exist."
              << std::endl;

        return false;
    }

    const auto& txid = transaction.txid();
    account->add_outgoing(txid);
    auto saved = storage_.Store(sNymID, account->type(), *account);

    if (false == saved) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to save account."
              << std::endl;

        return false;
    }

    saved = storage_.Store(transaction);

    if (false == saved) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to save transaction."
              << std::endl;

        return false;
    }

    if (recipientContactID.empty()) {
        return true;
    }

    return activity_.AddBlockchainTransaction(
        senderNymID,
        recipientContactID,
        StorageBox::OUTGOINGBLOCKCHAIN,
        transaction);
}

std::shared_ptr<proto::BlockchainTransaction> Blockchain::Transaction(
    const std::string& txid) const
{
    std::shared_ptr<proto::BlockchainTransaction> output;

    if (false == storage_.Load(txid, output, false)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load transaction."
              << std::endl;
    }

    return output;
}
}  // namespace opentxs::api
