// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_FORWARD_HPP
#define OPENTXS_FORWARD_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Exclusive.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/SharedPimpl.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Cash;
class Issuer;
class Pair;
class ServerAction;
class Sync;
class Wallet;
class Workflow;
}  // namespace client

namespace crypto
{
class Encode;
class Hash;
class Symmetric;
class Util;
}  // namespace crypto

namespace storage
{
class Storage;
}  // namespace storage

namespace network
{
class Dht;
class ZMQ;
}  // namespace network

class Activity;
class Api;
class Blockchain;
class ContactManager;
class Crypto;
class Identity;
class Native;
class Server;
class Settings;
class UI;
}  // namespace api

namespace client
{
class ServerAction;
class Wallet;
}  // namespace client

namespace crypto
{
namespace key
{
class Asymmetric;
class Ed25519;
class EllipticCurve;
class Keypair;
class LegacySymmetric;
class RSA;
class Secp256k1;
class Symmetric;
}  // namespace key

class AsymmetricProvider;
template<typename X> class Bip32;
class Bip39;
class EcdsaProvider;
class EncodingProvider;
class LegacySymmetricProvider;
class HashingProvider;
class OpenSSL;
class Secp256k1;
class Sodium;
class SymmetricProvider;
template<typename X> class Trezor;
}  // namespace crypto

namespace network
{
namespace zeromq
{
class Context;
class DealerSocket;
class FrameIterator;
class FrameSection;
class ListenCallback;
class Frame;
class Message;
class PairEventCallback;
class PairSocket;
class Proxy;
class PublishSocket;
class PullSocket;
class PushSocket;
class ReplyCallback;
class ReplySocket;
class RequestSocket;
class RouterSocket;
class Socket;
class SubscribeSocket;
}  // namespace zeromq

class Dht;
class OpenDHT;
class ServerConnection;
class ZMQ;
}  // namespace network

namespace OTDB
{
class OfferListNym;
class OTPacker;
class TradeListMarket;
}  // namespace OTDB

namespace server
{
class MessageProcessor;
class Notary;
class Server;
class UserCommandProcessor;
}  // namespace server

namespace ui
{
class AccountActivity;
class AccountSummary;
class AccountSummaryItem;
class ActivityThread;
class ActivityThreadItem;
class ActivitySummary;
class ActivitySummaryItem;
class BalanceItem;
class Contact;
class ContactItem;
class ContactSection;
class ContactSubsection;
class ContactList;
class ContactListItem;
class IssuerItem;
class ListRow;
class MessagableList;
class PayableList;
class PayableListItem;
class Profile;
class ProfileItem;
class ProfileSection;
class ProfileSubsection;
}  // namespace ui

class Account;
class AccountList;
class AccountVisitor;
class Armored;
class Basket;
class BasketContract;
class BasketItem;
class Cheque;
class ClientContext;
class Contact;
class ContactData;
class ContactGroup;
class ContactItem;
class ContactSection;
class Context;
class Contract;
class CurrencyContract;
class Credential;
class CredentialSet;
class CryptoSymmetricDecryptOutput;
class Data;
class Flag;
class Identifier;
class Item;
class Ledger;
class Letter;
class ListenCallbackSwig;
class Log;
class MasterCredential;
class Message;
#if OT_CASH
class Mint;
#endif  // OT_CASH
class NumList;
class Nym;
class NymData;
class NymFile;
class NymIDSource;
class NymParameters;
class OT;
class OT_API;
class OTAPI_Exec;
class OTCachedKey;
class OTCallback;
class OTCaller;
class OTClient;
class OTCron;
class OTCronItem;
class OTDataFolder;
class OTEnvelope;
class OTFolders;
class OTMarket;
class OTNym_or_SymmetricKey;
class OTOffer;
class OTPartyAccount;
class OTPassword;
class OTPasswordData;
class OTPaths;
class OTPayment;
class OTPaymentPlan;
class OTRecordList;
class OTSignature;
class OTSignatureMetadata;
class OTSmartContract;
class OTTrackable;
class OTTrade;
class OTTransaction;
class OTWallet;
class PairEventCallbackSwig;
class PayDividendVisitor;
class PaymentCode;
class PeerObject;
#if OT_CASH
class Purse;
#endif  // OT_CASH
class ServerContext;
class ServerContract;
class Signals;
class StorageDriver;
class StoragePlugin;
class String;
class Tag;
#if OT_CASH
class Token;
#endif  // OT_CASH
class TransactionStatement;
class UnitDefinition;

using OTArmored = Pimpl<Armored>;
using OTAsymmetricKey = Pimpl<crypto::key::Asymmetric>;
using OTData = Pimpl<Data>;
using OTKeypair = Pimpl<crypto::key::Keypair>;
using OTFlag = Pimpl<Flag>;
using OTIdentifier = Pimpl<Identifier>;
using OTLegacySymmetricKey = Pimpl<crypto::key::LegacySymmetric>;
using OTPaymentCode = Pimpl<PaymentCode>;
using OTServerConnection = Pimpl<network::ServerConnection>;
using OTString = Pimpl<String>;
using OTSymmetricKey = Pimpl<crypto::key::Symmetric>;
using OTZMQContext = Pimpl<network::zeromq::Context>;
using OTZMQDealerSocket = Pimpl<network::zeromq::DealerSocket>;
using OTZMQListenCallback = Pimpl<network::zeromq::ListenCallback>;
using OTZMQFrame = Pimpl<network::zeromq::Frame>;
using OTZMQMessage = Pimpl<network::zeromq::Message>;
using OTZMQPairEventCallback = Pimpl<network::zeromq::PairEventCallback>;
using OTZMQPairSocket = Pimpl<network::zeromq::PairSocket>;
using OTZMQProxy = Pimpl<network::zeromq::Proxy>;
using OTZMQPublishSocket = Pimpl<network::zeromq::PublishSocket>;
using OTZMQPullSocket = Pimpl<network::zeromq::PullSocket>;
using OTZMQPushSocket = Pimpl<network::zeromq::PushSocket>;
using OTZMQReplyCallback = Pimpl<network::zeromq::ReplyCallback>;
using OTZMQReplySocket = Pimpl<network::zeromq::ReplySocket>;
using OTZMQRequestSocket = Pimpl<network::zeromq::RequestSocket>;
using OTZMQRouterSocket = Pimpl<network::zeromq::RouterSocket>;
using OTZMQSubscribeSocket = Pimpl<network::zeromq::SubscribeSocket>;

using ExclusiveAccount = Exclusive<Account>;

using SharedAccount = Shared<Account>;

using OTUIAccountSummaryItem = SharedPimpl<ui::AccountSummaryItem>;
using OTUIActivitySummaryItem = SharedPimpl<ui::ActivitySummaryItem>;
using OTUIActivityThreadItem = SharedPimpl<ui::ActivityThreadItem>;
using OTUIBalanceItem = SharedPimpl<ui::BalanceItem>;
using OTUIContactItem = SharedPimpl<ui::ContactItem>;
using OTUIContactListItem = SharedPimpl<ui::ContactListItem>;
using OTUIContactSection = SharedPimpl<ui::ContactSection>;
using OTUIContactSubsection = SharedPimpl<ui::ContactSubsection>;
using OTUIIssuerItem = SharedPimpl<ui::IssuerItem>;
using OTUIPayableListItem = SharedPimpl<ui::PayableListItem>;
using OTUIProfileItem = SharedPimpl<ui::ProfileItem>;
using OTUIProfileSection = SharedPimpl<ui::ProfileSection>;
using OTUIProfileSubsection = SharedPimpl<ui::ProfileSubsection>;
}  // namespace opentxs

namespace std
{
template <>
struct less<opentxs::OTIdentifier> {
    bool operator()(
        const opentxs::OTIdentifier& lhs,
        const opentxs::OTIdentifier& rhs) const;
};
}  // namespace std
#endif  // OPENTXS_FORWARD_HPP
