// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
class Factory
{
public:
    static ui::implementation::AccountActivityExternalInterface*
    AccountActivity(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const Identifier& accountID);
    static ui::implementation::AccountSummaryExternalInterface* AccountSummary(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const proto::ContactItemType currency);
    static ui::implementation::IssuerItemRowInternal* AccountSummaryItem(
        const ui::implementation::IssuerItemInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::IssuerItemRowID& rowID,
        const ui::implementation::IssuerItemSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static api::client::internal::Activity* Activity(
        const api::Core& api,
        const api::client::Contacts& contact);
    static ui::implementation::ActivitySummaryExternalInterface*
    ActivitySummary(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Flag& running,
        const Identifier& nymID);
    static ui::implementation::ActivitySummaryRowInternal* ActivitySummaryItem(
        const ui::implementation::ActivitySummaryInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const ui::implementation::ActivitySummaryRowID& rowID,
        const ui::implementation::ActivitySummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const Flag& running);
    static ui::ActivityThread* ActivityThread(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const Identifier& threadID);
    static ui::implementation::AccountActivityRowInternal* BalanceItem(
        const ui::implementation::AccountActivityInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::AccountActivityRowID& rowID,
        const ui::implementation::AccountActivitySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const Identifier& nymID,
        const Identifier& accountID);
    static crypto::Bitcoin* Bitcoin(const api::Crypto& crypto);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    static api::client::Blockchain* Blockchain(
        const api::Core& api,
        const api::client::Activity& activity);
#endif
    static api::client::Cash* Cash(
        const api::client::Manager& api,
        const api::client::ServerAction& serverAction);
    static api::client::internal::Manager* ClientManager(
        const Flag& running,
        const ArgList& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static ui::implementation::ContactListExternalInterface* ContactList(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID);
    static ui::implementation::ContactListRowInternal* ContactListItem(
        const ui::implementation::ContactListInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::ContactListRowID& rowID,
        const ui::implementation::ContactListSortKey& key);
    static api::client::internal::Contacts* Contacts(const api::Core& api);
    static ui::implementation::ContactExternalInterface* ContactWidget(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& contactID);
    static ui::implementation::ContactSubsectionRowInternal* ContactItemWidget(
        const ui::implementation::ContactSubsectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::ContactSubsectionRowID& rowID,
        const ui::implementation::ContactSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactRowInternal* ContactSectionWidget(
        const ui::implementation::ContactInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::ContactRowID& rowID,
        const ui::implementation::ContactSortKey& key,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactSectionRowInternal*
    ContactSubsectionWidget(
        const ui::implementation::ContactSectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::ContactSectionRowID& rowID,
        const ui::implementation::ContactSectionSortKey& key,
        const ui::implementation::CustomData& custom);
    static api::Crypto* Crypto(const api::Settings& settings);
    static api::crypto::Config* CryptoConfig(const api::Settings& settings);
    static api::network::Dht* Dht(
        const bool defaultEnable,
        const api::Core& api,
        std::int64_t& nymPublishInterval,
        std::int64_t& nymRefreshInterval,
        std::int64_t& serverPublishInterval,
        std::int64_t& serverRefreshInterval,
        std::int64_t& unitPublishInterval,
        std::int64_t& unitRefreshInterval);
    static crypto::key::Ed25519* Ed25519Key(
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Ed25519* Ed25519Key(const String& publicKey);
    static crypto::key::Ed25519* Ed25519Key(const proto::KeyRole role);
    static api::crypto::Encode* Encode(const crypto::EncodingProvider& base58);
    static api::Endpoints* Endpoints(
        const network::zeromq::Context& zmq,
        const int instance);
    static api::Factory* FactoryAPI(const api::Core& api);
    static api::crypto::Hash* Hash(
        const api::crypto::Encode& encode,
        const crypto::HashingProvider& ssl,
        const crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
        ,
        const crypto::Ripemd160& bitcoin
#endif
    );
#if OT_CRYPTO_WITH_BIP39
    static api::HDSeed* HDSeed(
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const crypto::Bip32& bip32,
        const crypto::Bip39& bip39,
        const crypto::LegacySymmetricProvider& aes);
#endif
    static api::Identity* Identity(const api::Core& api);
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const Identifier& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& issuerID);
    static ui::implementation::AccountSummaryRowInternal* IssuerItem(
        const ui::implementation::AccountSummaryInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::AccountSummaryRowID& rowID,
        const ui::implementation::AccountSummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const proto::ContactItemType currency);
    static api::Legacy* Legacy();
    static api::internal::Log* Log(
        const network::zeromq::Context& zmq,
        const std::string& endpoint);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const bool loading,
        const bool pending);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::ManagedNumber* ManagedNumber(
        const TransactionNumber number,
        ServerContext& context);
    static ui::implementation::MessagableExternalInterface* MessagableList(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID);
    static api::internal::Native* Native(
        Flag& running,
        const ArgList& args,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    static OTCallback* NullCallback();
    static internal::NymFile* NymFile(
        const api::Core& core,
        std::shared_ptr<const Nym> targetNym,
        std::shared_ptr<const Nym> signerNym);
    static crypto::OpenSSL* OpenSSL(const api::Crypto& crypto);
    static api::client::Pair* Pair(
        const Flag& running,
        const api::client::Manager& client);
    static ui::implementation::PayableExternalInterface* PayableList(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const proto::ContactItemType& currency);
    static ui::implementation::PayableListRowInternal* PayableListItem(
        const ui::implementation::PayableInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::PayableListRowID& rowID,
        const ui::implementation::PayableListSortKey& key,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
    static ui::implementation::ActivityThreadRowInternal* PaymentItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static opentxs::PIDFile* PIDFile(const std::string& path);
    static ui::implementation::ProfileExternalInterface* ProfileWidget(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Identifier& nymID);
    static ui::implementation::ProfileSubsectionRowInternal* ProfileItemWidget(
        const ui::implementation::ProfileSubsectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::ProfileSubsectionRowID& rowID,
        const ui::implementation::ProfileSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ProfileRowInternal* ProfileSectionWidget(
        const ui::implementation::ProfileInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::ProfileRowID& rowID,
        const ui::implementation::ProfileSortKey& key,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ProfileSectionRowInternal*
    ProfileSubsectionWidget(
        const ui::implementation::ProfileSectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ui::implementation::ProfileSectionRowID& rowID,
        const ui::implementation::ProfileSectionSortKey& key,
        const ui::implementation::CustomData& custom);
    static rpc::internal::RPC* RPC(const api::Native& native);
    static crypto::key::RSA* RSAKey(const proto::AsymmetricKey& serializedKey);
    static crypto::key::RSA* RSAKey(const String& publicKey);
    static crypto::key::RSA* RSAKey(const proto::KeyRole role);
    static crypto::Secp256k1* Secp256k1(
        const api::Crypto& crypto,
        const api::crypto::Util& util,
        const crypto::EcdsaProvider& ecdsa);
    static crypto::key::Secp256k1* Secp256k1Key(
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Secp256k1* Secp256k1Key(const String& publicKey);
    static crypto::key::Secp256k1* Secp256k1Key(const proto::KeyRole role);
    static api::client::ServerAction* ServerAction(
        const api::client::Manager& api,
        const ContextLockCallback& lockCallback);
    static api::server::Manager* ServerManager(
        const Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    static api::Settings* Settings();
    static api::Settings* Settings(const String& path);
    static crypto::Sodium* Sodium(const api::Crypto& crypto);
    static api::storage::StorageInternal* Storage(
        const Flag& running,
        const api::Crypto& crypto,
        const api::Settings& config,
        const std::string& dataFolder,
        const String& defaultPluginCLI,
        const String& archiveDirectoryCLI,
        const std::chrono::seconds gcIntervalCLI,
        String& encryptedDirectoryCLI,
        StorageConfig& storageConfig);
    static api::crypto::Symmetric* Symmetric(crypto::SymmetricProvider& sodium);
#if OT_STORAGE_FS
    static opentxs::api::storage::Plugin* StorageFSArchive(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket,
        const std::string& folder,
        crypto::key::Symmetric& key);
    static opentxs::api::storage::Plugin* StorageFSGC(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static opentxs::api::storage::Plugin* StorageMemDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#if OT_STORAGE_LMDB
    static opentxs::api::storage::Plugin* StorageLMDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static opentxs::api::storage::Multiplex* StorageMultiplex(
        const api::storage::Storage& storage,
        const Flag& primaryBucket,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
#if OT_STORAGE_SQLITE
    static opentxs::api::storage::Plugin* StorageSqlite3(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
#endif
    static api::client::Sync* Sync(
        const Flag& running,
        const api::client::Manager& api,
        OTClient& otclient,
        const ContextLockCallback& lockCallback);
    static crypto::Trezor* Trezor(const api::Crypto& crypto);
    static api::client::UI* UI(
        const api::client::Manager& api,
        const Flag& running);
    static api::Wallet* Wallet(const api::client::Manager& client);
    static api::Wallet* Wallet(const api::server::Manager& server);
    static api::client::Workflow* Workflow(
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contact);
    static api::network::ZAP* ZAP(const network::zeromq::Context& context);
    static api::network::ZMQ* ZMQ(const api::Core& api, const Flag& running);
};
}  // namespace opentxs
