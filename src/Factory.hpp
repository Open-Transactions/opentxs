// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_FACTORY_HPP
#define OPENTXS_FACTORY_HPP

namespace opentxs
{
class Factory
{
public:
    static ui::implementation::AccountActivityExternalInterface*
    AccountActivity(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::ContactManager& contact,
        const api::storage::Storage& storage,
        const Identifier& nymID,
        const Identifier& accountID);
    static ui::implementation::AccountSummaryExternalInterface* AccountSummary(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Wallet& wallet,
        const api::network::ZMQ& connection,
        const api::storage::Storage& storage,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const proto::ContactItemType currency);
    static ui::implementation::IssuerItemRowInternal* AccountSummaryItem(
        const ui::implementation::IssuerItemInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::IssuerItemRowID& rowID,
        const ui::implementation::IssuerItemSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Wallet& wallet,
        const api::storage::Storage& storage);
    static ui::implementation::ActivitySummaryExternalInterface*
    ActivitySummary(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Flag& running,
        const Identifier& nymID);
    static ui::implementation::ActivitySummaryRowInternal* ActivitySummaryItem(
        const ui::implementation::ActivitySummaryInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const ui::implementation::ActivitySummaryRowID& rowID,
        const ui::implementation::ActivitySummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const Flag& running);
    static ui::ActivityThread* ActivityThread(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Sync& sync,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const Identifier& threadID);
    static api::Api* Api(
        const Flag& running,
        const api::Activity& activity,
        const api::Settings& config,
        const api::ContactManager& contacts,
        const api::Crypto& crypto,
        const api::Identity& identity,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet,
        const api::network::ZMQ& zmq);
    static ui::implementation::AccountActivityRowInternal* BalanceItem(
        const ui::implementation::AccountActivityInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::AccountActivityRowID& rowID,
        const ui::implementation::AccountActivitySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& accountID);
    static api::Blockchain* Blockchain(
        const api::Activity& activity,
        const api::Crypto& crypto,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet);
    static api::client::Cash* Cash();
    static ui::implementation::ContactListExternalInterface* ContactList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID);
    static ui::implementation::ContactListRowInternal* ContactListItem(
        const ui::implementation::ContactListInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ContactListRowID& rowID,
        const ui::implementation::ContactListSortKey& key);
    static ui::implementation::ContactExternalInterface* ContactWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& contactID);
    static ui::implementation::ContactSubsectionRowInternal* ContactItemWidget(
        const ui::implementation::ContactSubsectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ContactSubsectionRowID& rowID,
        const ui::implementation::ContactSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactRowInternal* ContactSectionWidget(
        const ui::implementation::ContactInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ContactRowID& rowID,
        const ui::implementation::ContactSortKey& key,
        const ui::implementation::CustomData& custom);
    static ui::implementation::ContactSectionRowInternal*
    ContactSubsectionWidget(
        const ui::implementation::ContactSectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ContactSectionRowID& rowID,
        const ui::implementation::ContactSectionSortKey& key,
        const ui::implementation::CustomData& custom);
    static api::Crypto* Crypto(const api::Native& native);
    static crypto::key::Ed25519* Ed25519Key(
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Ed25519* Ed25519Key(const String& publicKey);
    static crypto::key::Ed25519* Ed25519Key(const proto::KeyRole role);
    static api::crypto::Encode* Encode(crypto::EncodingProvider& base58);
    static api::crypto::Hash* Hash(
        api::crypto::Encode& encode,
        crypto::HashingProvider& ssl,
        crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR
        ,
        crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& bitcoin
#endif
    );
    static api::Identity* Identity(const api::client::Wallet& wallet);
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const proto::Issuer& serialized);
    static api::client::Issuer* Issuer(
        const api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& issuerID);
    static ui::implementation::AccountSummaryRowInternal* IssuerItem(
        const ui::implementation::AccountSummaryInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::AccountSummaryRowID& rowID,
        const ui::implementation::AccountSummarySortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Wallet& wallet,
        const api::storage::Storage& storage,
        const proto::ContactItemType currency);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::Activity& activity,
        const bool loading,
        const bool pending);
    static ui::implementation::ActivityThreadRowInternal* MailItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::Activity& activity);
    static ui::implementation::MessagableExternalInterface* MessagableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID);
    static api::NativeInternal* Native(
        Flag& running,
        const ArgList& args,
        const bool recover,
        const bool serverMode,
        const std::chrono::seconds gcInterval,
        OTCaller* externalPasswordCallback = nullptr);
    static OTCallback* NullCallback();
    static crypto::OpenSSL* OpenSSL();
    static api::client::Pair* Pair(
        const Flag& running,
        const api::client::Sync& sync,
        const api::client::ServerAction& action,
        const api::client::Wallet& wallet,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const network::zeromq::Context& context);
    static ui::implementation::PayableExternalInterface* PayableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID,
        const proto::ContactItemType& currency);
    static ui::implementation::PayableListRowInternal* PayableListItem(
        const ui::implementation::PayableInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::PayableListRowID& rowID,
        const ui::implementation::PayableListSortKey& key,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
    static ui::implementation::ActivityThreadRowInternal* PaymentItem(
        const ui::implementation::ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const ui::implementation::ActivityThreadRowID& rowID,
        const ui::implementation::ActivityThreadSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::Activity& activity);
    static ui::implementation::ProfileExternalInterface* ProfileWidget(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const Identifier& nymID);
    static ui::implementation::ProfileSubsectionRowInternal* ProfileItemWidget(
        const ui::implementation::ProfileSubsectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ProfileSubsectionRowID& rowID,
        const ui::implementation::ProfileSubsectionSortKey& sortKey,
        const ui::implementation::CustomData& custom,
        const api::client::Wallet& wallet);
    static ui::implementation::ProfileRowInternal* ProfileSectionWidget(
        const ui::implementation::ProfileInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ProfileRowID& rowID,
        const ui::implementation::ProfileSortKey& key,
        const ui::implementation::CustomData& custom,
        const api::client::Wallet& wallet);
    static ui::implementation::ProfileSectionRowInternal*
    ProfileSubsectionWidget(
        const ui::implementation::ProfileSectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ui::implementation::ProfileSectionRowID& rowID,
        const ui::implementation::ProfileSectionSortKey& key,
        const ui::implementation::CustomData& custom,
        const api::client::Wallet& wallet);
    static crypto::key::RSA* RSAKey(const proto::AsymmetricKey& serializedKey);
    static crypto::key::RSA* RSAKey(const String& publicKey);
    static crypto::key::RSA* RSAKey(const proto::KeyRole role);
    static crypto::Secp256k1* Secp256k1(
        const api::crypto::Util& util,
        const crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& ecdsa);
    static crypto::key::Secp256k1* Secp256k1Key(
        const proto::AsymmetricKey& serializedKey);
    static crypto::key::Secp256k1* Secp256k1Key(const String& publicKey);
    static crypto::key::Secp256k1* Secp256k1Key(const proto::KeyRole role);
    static api::client::ServerAction* ServerAction(
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const ContextLockCallback& lockCallback);
    static api::Settings* Settings();
    static api::Settings* Settings(const String& path);
    static crypto::Sodium* Sodium();
    static api::storage::StorageInternal* Storage(
        const Flag& running,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
    static api::crypto::Symmetric* Symmetric(crypto::SymmetricProvider& sodium);
    static api::client::Sync* Sync(
        const Flag& running,
        const OT_API& otapi,
        const OTAPI_Exec& exec,
        const api::ContactManager& contacts,
        const api::Settings& config,
        const api::Api& api,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::crypto::Encode& encoding,
        const api::storage::Storage& storage,
        const network::zeromq::Context& zmq,
        const ContextLockCallback& lockCallback);
    static opentxs::crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>* Trezor(const opentxs::api::Native& native);
    static api::UI* UI(
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::network::ZMQ& connection,
        const api::storage::Storage& storage,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const network::zeromq::Context& zmq,
        const Flag& running);
    static api::client::Wallet* Wallet(
        const api::Native& ot,
        const network::zeromq::Context& zmq);
    static api::client::Workflow* Workflow(
        const api::Activity& activity,
        const api::ContactManager& contact,
        const api::storage::Storage& storage,
        const network::zeromq::Context& zmq);
};
}  // namespace opentxs
#endif  // OPENTXS_FACTORY_HPP
