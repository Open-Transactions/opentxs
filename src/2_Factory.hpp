// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace internal
{
struct BalanceList;
struct BalanceNode;
struct BalanceTree;
struct Deterministic;
struct HD;
struct Imported;
struct PaymentCode;
}  // namespace internal
}  // namespace blockchain

namespace implementation
{
class UI;
}  // namespace implementation

namespace internal
{
struct Activity;
struct Blockchain;
struct Contacts;
struct Manager;
struct Pair;
}  // namespace internal
}  // namespace client

namespace crypto
{
namespace internal
{
struct Asymmetric;
}  // namespace internal
}  // namespace crypto

namespace implementation
{
class Context;
class Core;
class Storage;
}  // namespace implementation

namespace internal
{
struct Context;
struct Core;
struct Factory;
struct Log;
}  // namespace internal

namespace network
{
namespace implementation
{
class Context;
class Dht;
class ZMQ;
}  // namespace implementation
}  // namespace network

namespace server
{
namespace implementation
{
class Factory;
class Manager;
}  // namespace implementation
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace server

namespace storage
{
namespace implementation
{
class Storage;
}  // namespace implementation

class Driver;
class Multiplex;
class Plugin;
class StorageInternal;
}  // namespace storage

class Core;
}  // namespace api

namespace blind
{
namespace implementation
{
class Purse;
}  // namespace implementation

namespace token
{
namespace implementation
{
class Token;
}  // namespace implementation
}  // namespace token
}  // namespace blind

namespace crypto
{
class Pbkdf2;
class Ripemd160;

namespace implementation
{
class OpenSSL;
}  // namespace implementation
}  // namespace crypto

namespace identity
{
namespace credential
{
namespace internal
{
struct Base;
struct Contact;
struct Key;
struct Primary;
struct Secondary;
struct Verification;
}  // namespace internal
}  // namespace credential

namespace internal
{
struct Authority;
struct Nym;
}  // namespace internal

namespace wot
{
namespace verification
{
namespace internal
{
struct Group;
struct Item;
struct Nym;
struct Set;
}  // namespace internal
}  // namespace verification
}  // namespace wot
}  // namespace identity

namespace internal
{
struct NymFile;
}  // namespace internal

namespace network
{
namespace zeromq
{
namespace implementation
{
class Context;
class Proxy;
}  // namespace implementation
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace client
{
namespace internal
{
struct Operation;
struct StateMachine;
}  // namespace internal
}  // namespace client

namespace context
{
class Server;
}  // namespace context
}  // namespace otx

namespace rpc
{
namespace internal
{
struct RPC;
}  // namespace internal
}  // namespace rpc

namespace server
{
class ReplyMessage;
}  // namespace server

namespace storage
{
namespace implementation
{
class StorageMultiplex;
}  // namespace implementation

class Accounts;
class Bip47Channels;
class BlockchainTransactions;
class Contacts;
class Contexts;
class Credentials;
class Issuers;
class Mailbox;
class Notary;
class Nym;
class Nyms;
class PaymentWorkflows;
class PeerReplies;
class PeerRequests;
class Seeds;
class Servers;
class Thread;
class Threads;
class Tree;
class Txos;
class Units;
}  // namespace storage

class DhtConfig;
#if OT_CRYPTO_USING_LIBSECP256K1
class Libsecp256k1;
#endif
class Libsodium;
class LowLevelKeyGenerator;
#if OT_CRYPTO_USING_OPENSSL
class OpenSSL;
#endif
class Secret;
class StorageConfig;
}  // namespace opentxs

namespace opentxs
{
class Factory
{
public:
    static auto Armored() -> opentxs::Armored*;
    static auto Armored(const opentxs::Data& input) -> opentxs::Armored*;
    static auto Armored(const opentxs::String& input) -> opentxs::Armored*;
    static auto Armored(const opentxs::crypto::Envelope& input)
        -> opentxs::Armored*;
    static auto Authority(
        const api::internal::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const proto::KeyMode mode,
        const proto::Authority& serialized) -> identity::internal::Authority*;
    static auto Authority(
        const api::internal::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber nymVersion,
        const opentxs::PasswordPrompt& reason)
        -> identity::internal::Authority*;
    static auto Activity(
        const api::internal::Core& api,
        const api::client::Contacts& contact)
        -> api::client::internal::Activity*;
    static auto AsymmetricAPI(const api::internal::Core& api)
        -> api::crypto::internal::Asymmetric*;
    static auto BailmentNotice(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const opentxs::Identifier& requestID,
        const std::string& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::BailmentNotice>;
    static auto BailmentNotice(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::BailmentNotice>;
    static auto BailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Bailment>;
    static auto BailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Bailment>;
    static auto BailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Bailment>;
    static auto BailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Bailment>;
    static auto BasketContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static auto BasketContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static auto Bitcoin(const api::Crypto& crypto) -> crypto::Bitcoin*;
    static auto Bip39(const api::Crypto& api) noexcept
        -> std::unique_ptr<crypto::Bip39>;
    static auto BlockchainAPI(
        const api::client::internal::Manager& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const ArgList& args) noexcept
        -> std::unique_ptr<api::client::Blockchain>;
    static auto BlockchainBalanceList(
        const api::client::internal::Blockchain& parent,
        const blockchain::Type chain)
        -> api::client::blockchain::internal::BalanceList*;
    static auto BlockchainBalanceTree(
        const api::client::blockchain::internal::BalanceList& parent,
        const identifier::Nym& id,
        const std::set<OTIdentifier>& hdAccounts,
        const std::set<OTIdentifier>& importedAccounts,
        const std::set<OTIdentifier>& paymentCodeAccounts)
        -> api::client::blockchain::internal::BalanceTree*;
    static auto BlockchainHDBalanceNode(
        const api::client::blockchain::internal::BalanceTree& parent,
        const proto::HDPath& path,
        Identifier& id) -> api::client::blockchain::internal::HD*;
    static auto BlockchainHDBalanceNode(
        const api::client::blockchain::internal::BalanceTree& parent,
        const proto::HDAccount& serialized,
        Identifier& id) -> api::client::blockchain::internal::HD*;
    static auto ClientManager(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance) -> api::client::internal::Manager*;
    static auto ConnectionReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Connection>;
    static auto ConnectionReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Connection>;
    static auto ConnectionRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const proto::ConnectionInfoType type,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Connection>;
    static auto ConnectionRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Connection>;
    static auto ContactCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Contact*;
    static auto ContactCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Contact*;
    static auto ContactAPI(const api::client::internal::Manager& api)
        -> api::client::internal::Contacts*;
    template <class C>
    static auto Credential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const VersionNumber version,
        const NymParameters& parameters,
        const proto::CredentialRole role,
        const opentxs::PasswordPrompt& reason) -> C*;
    template <class C>
    static auto Credential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role) -> C*;
    static auto Crypto(const api::Settings& settings) -> api::Crypto*;
    static auto CryptoConfig(const api::Settings& settings)
        -> api::crypto::Config*;
    static auto CurrencyContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static auto CurrencyContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static auto DealerSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback)
        -> network::zeromq::socket::Dealer*;
    static auto Dht(
        const bool defaultEnable,
        const api::internal::Core& api,
        std::int64_t& nymPublishInterval,
        std::int64_t& nymRefreshInterval,
        std::int64_t& serverPublishInterval,
        std::int64_t& serverRefreshInterval,
        std::int64_t& unitPublishInterval,
        std::int64_t& unitRefreshInterval) -> api::network::Dht*;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    static auto Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey) -> crypto::key::Ed25519*;
    static auto Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) -> crypto::key::Ed25519*;
#if OT_CRYPTO_WITH_BIP32
    static auto Ed25519Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const Secret& privateKey,
        const Secret& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) -> crypto::key::Ed25519*;
#endif  // OT_CRYPTO_WITH_BIP32
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    static auto Encode(const api::Crypto& crypto) -> api::crypto::Encode*;
    static auto Envelope(const api::internal::Core& api) noexcept
        -> std::unique_ptr<crypto::Envelope>;
    static auto Envelope(
        const api::internal::Core& api,
        const proto::Envelope& serialized) noexcept(false)
        -> std::unique_ptr<crypto::Envelope>;
    static auto FactoryAPIClient(const api::client::internal::Manager& api)
        -> api::internal::Factory*;
    static auto FactoryAPIServer(const api::server::internal::Manager& api)
        -> api::internal::Factory*;
    static auto Hash(
        const api::crypto::Encode& encode,
        const crypto::HashingProvider& ssl,
        const crypto::HashingProvider& sodium,
        const crypto::Pbkdf2& pbkdf2,
        const crypto::Ripemd160& ripe) noexcept
        -> std::unique_ptr<api::crypto::Hash>;
    static auto Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const proto::Issuer& serialized) -> api::client::Issuer*;
    static auto Issuer(
        const api::Wallet& wallet,
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) -> api::client::Issuer*;
    static auto Keypair() noexcept -> std::unique_ptr<crypto::key::Keypair>;
    static auto Keypair(
        const api::internal::Core& api,
        const proto::KeyRole role,
        std::unique_ptr<crypto::key::Asymmetric> publicKey,
        std::unique_ptr<crypto::key::Asymmetric> privateKey) noexcept(false)
        -> std::unique_ptr<crypto::key::Keypair>;
#if OT_CASH
    static auto MintLucre(const api::internal::Core& core) -> blind::Mint*;
    static auto MintLucre(
        const api::internal::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) -> blind::Mint*;
    static auto MintLucre(
        const api::internal::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) -> blind::Mint*;
#endif
    static auto NoticeAcknowledgement(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const proto::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Acknowledgement>;
    static auto NoticeAcknowledgement(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Acknowledgement>;
    static auto NullCallback() -> OTCallback*;
    OPENTXS_EXPORT static auto Nym(
        const api::internal::Core& api,
        const NymParameters& nymParameters,
        const proto::ContactItemType type,
        const std::string name,
        const opentxs::PasswordPrompt& reason) -> identity::internal::Nym*;
    OPENTXS_EXPORT static auto Nym(
        const api::internal::Core& api,
        const proto::Nym& serialized,
        const std::string& alias) -> identity::internal::Nym*;
    static auto NymFile(
        const api::internal::Core& core,
        Nym_p targetNym,
        Nym_p signerNym) -> internal::NymFile*;
    static auto NymIDSource(
        const api::internal::Core& api,
        NymParameters& parameters,
        const opentxs::PasswordPrompt& reason) -> identity::Source*;
    static auto NymIDSource(
        const api::internal::Core& api,
        const proto::NymIDSource& serialized) -> identity::Source*;
    static auto OpenSSL(const api::Crypto& crypto) -> crypto::OpenSSL*;
    OPENTXS_EXPORT static auto Operation(
        const api::client::internal::Manager& api,
        const identifier::Nym& nym,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason)
        -> otx::client::internal::Operation*;
    static auto OutBailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Outbailment>;
    static auto OutBailmentReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Outbailment>;
    static auto OutbailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Outbailment>;
    static auto OutbailmentRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Outbailment>;
    static auto OTX(
        const Flag& running,
        const api::client::internal::Manager& api,
        OTClient& otclient,
        const ContextLockCallback& lockCallback) -> api::client::OTX*;
    static auto PasswordPrompt(
        const api::internal::Core& api,
        const std::string& text) -> opentxs::PasswordPrompt*;
    static auto PairAPI(
        const Flag& running,
        const api::client::internal::Manager& client)
        -> api::client::internal::Pair*;
    static auto PairSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback,
        const bool startThread) -> network::zeromq::socket::Pair*;
    static auto PairSocket(
        const network::zeromq::ListenCallback& callback,
        const network::zeromq::socket::Pair& peer,
        const bool startThread) -> network::zeromq::socket::Pair*;
    static auto PairSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback,
        const std::string& endpoint) -> network::zeromq::socket::Pair*;
    static auto PaymentCode(
        const api::internal::Core& api,
        const std::uint8_t version,
        const bool hasBitmessage,
        const ReadView pubkey,
        const ReadView chaincode,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        ,
        std::unique_ptr<crypto::key::Secp256k1> key
#endif
        ) noexcept -> std::unique_ptr<opentxs::PaymentCode>;
    static auto PeerObject(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::string& message) -> opentxs::PeerObject*;
    static auto PeerObject(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment) -> opentxs::PeerObject*;
#if OT_CASH
    static auto PeerObject(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse) -> opentxs::PeerObject*;
#endif
    static auto PeerObject(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) -> opentxs::PeerObject*;
    static auto PeerObject(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const VersionNumber version) -> opentxs::PeerObject*;
    static auto PeerObject(
        const api::client::Contacts& contacts,
        const api::internal::Core& api,
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) -> opentxs::PeerObject*;
    static auto PeerObject(
        const api::client::Contacts& contacts,
        const api::internal::Core& api,
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) -> opentxs::PeerObject*;
    static auto PeerReply(const api::Core& api) noexcept
        -> std::shared_ptr<contract::peer::Reply>;
    static auto PeerReply(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::Reply>;
    static auto PeerRequest(const api::Core& api) noexcept
        -> std::shared_ptr<contract::peer::Request>;
    static auto PeerRequest(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::Request>;
    static auto PIDFile(const std::string& path) -> opentxs::PIDFile*;
    static auto Pipeline(
        const api::internal::Core& api,
        const network::zeromq::Context& context,
        std::function<void(network::zeromq::Message&)> callback)
        -> opentxs::network::zeromq::Pipeline*;
    static auto PrimaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Primary*;
    static auto PrimaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const proto::Credential& credential)
        -> identity::credential::internal::Primary*;
#if OT_CASH
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const proto::Purse& serialized) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const otx::context::Server&,
        const proto::CashType type,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const identity::Nym& owner,
        const identifier::Server& server,
        const identity::Nym& serverNym,
        const proto::CashType type,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const blind::Purse& request,
        const identity::Nym& requester,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
    OPENTXS_EXPORT static auto Purse(
        const api::internal::Core& api,
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const proto::CashType type,
        const opentxs::PasswordPrompt& reason) -> blind::Purse*;
#endif
    static auto PublishSocket(const network::zeromq::Context& context)
        -> network::zeromq::socket::Publish*;
    static auto PullSocket(
        const network::zeromq::Context& context,
        const bool direction) -> network::zeromq::socket::Pull*;
    static auto PullSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback)
        -> network::zeromq::socket::Pull*;
    static auto PushSocket(
        const network::zeromq::Context& context,
        const bool direction) -> network::zeromq::socket::Push*;
    static auto ReplySocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ReplyCallback& callback)
        -> network::zeromq::socket::Reply*;
    static auto RequestSocket(const network::zeromq::Context& context)
        -> network::zeromq::socket::Request*;
    static auto RPC(const api::Context& native) -> rpc::internal::RPC*;
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    static auto RSAKey(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& input) noexcept
        -> std::unique_ptr<crypto::key::RSA>;
    static auto RSAKey(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::KeyRole input,
        const VersionNumber version,
        const NymParameters& options,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::unique_ptr<crypto::key::RSA>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
    static auto RouterSocket(
        const network::zeromq::Context& context,
        const bool direction,
        const network::zeromq::ListenCallback& callback)
        -> network::zeromq::socket::Router*;
    static auto SecondaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Secondary*;
    static auto SecondaryCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Secondary*;
#if OT_CRYPTO_USING_LIBSECP256K1
    static auto Secp256k1(
        const api::Crypto& crypto,
        const api::crypto::Util& util) -> crypto::Secp256k1*;
#endif  // OT_CRYPTO_USING_LIBSECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    static auto Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey) -> crypto::key::Secp256k1*;
    static auto Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) -> crypto::key::Secp256k1*;
#if OT_CRYPTO_WITH_BIP32
    static auto Secp256k1Key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const Secret& privateKey,
        const Secret& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) -> crypto::key::Secp256k1*;
#endif  // OT_CRYPTO_WITH_BIP32
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    static auto SecurityContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static auto SecurityContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static auto ServerAction(
        const api::client::internal::Manager& api,
        const ContextLockCallback& lockCallback) -> api::client::ServerAction*;
    static auto ServerContract(const api::Core& api) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::list<Endpoint>& endpoints,
        const std::string& terms,
        const std::string& name,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::ServerContract& serialized) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerManager(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance) -> api::server::Manager*;
    static auto Sodium(const api::Crypto& crypto) -> crypto::Sodium*;
    static auto Storage(
        const Flag& running,
        const api::Crypto& crypto,
        const api::Settings& config,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const String& defaultPluginCLI,
        const String& archiveDirectoryCLI,
        const std::chrono::seconds gcIntervalCLI,
        String& encryptedDirectoryCLI,
        StorageConfig& storageConfig) -> api::storage::StorageInternal*;
#if OT_STORAGE_FS
    static auto StorageFSArchive(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket,
        const std::string& folder,
        crypto::key::Symmetric& key) -> opentxs::api::storage::Plugin*;
    static auto StorageFSGC(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket) -> opentxs::api::storage::Plugin*;
#endif
    static auto StorageMemDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket) -> opentxs::api::storage::Plugin*;
#if OT_STORAGE_LMDB
    static auto StorageLMDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket) -> opentxs::api::storage::Plugin*;
#endif
    static auto StorageMultiplex(
        const api::storage::Storage& storage,
        const Flag& primaryBucket,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random) -> opentxs::api::storage::Multiplex*;
#if OT_STORAGE_SQLITE
    static auto StorageSqlite3(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket) -> opentxs::api::storage::Plugin*;
#endif
    static auto StoreSecret(
        const api::internal::Core& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const proto::SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::StoreSecret>;
    static auto StoreSecret(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::StoreSecret>;
    static auto SubscribeSocket(
        const network::zeromq::Context& context,
        const network::zeromq::ListenCallback& callback)
        -> network::zeromq::socket::Subscribe*;
    static auto Symmetric(const api::internal::Core& api)
        -> api::crypto::Symmetric*;
    static auto SymmetricKey() -> crypto::key::Symmetric*;
    static auto SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& reason,
        const proto::SymmetricMode mode) -> crypto::key::Symmetric*;
    static auto SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) -> crypto::key::Symmetric*;
    static auto SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type) -> crypto::key::Symmetric*;
    static auto SymmetricKey(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const Secret& raw,
        const opentxs::PasswordPrompt& reason) -> crypto::key::Symmetric*;
#if OT_CASH
    static auto Token(const blind::Token& token, blind::Purse& purse) noexcept
        -> std::unique_ptr<blind::Token>;
    static auto Token(
        const api::internal::Core& api,
        blind::Purse& purse,
        const proto::Token& serialized) noexcept(false)
        -> std::unique_ptr<blind::Token>;
    static auto Token(
        const api::internal::Core& api,
        const identity::Nym& owner,
        const blind::Mint& mint,
        const std::uint64_t value,
        blind::Purse& purse,
        const opentxs::PasswordPrompt& reason) noexcept(false)
        -> std::unique_ptr<blind::Token>;
#endif
    static auto UI(
        const api::client::internal::Manager& api,
        const Flag& running
#if OT_QT
        ,
        const bool qt
#endif
        ) -> api::client::UI*;
    static auto UnitDefinition(const api::Core& api) noexcept
        -> std::shared_ptr<contract::Unit>;
    static auto UnitDefinition(
        const api::internal::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::Unit>;
    static auto VerificationCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Verification*;
    static auto VerificationCredential(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Verification*;
    static auto VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const VersionNumber version,
        bool external) -> identity::wot::verification::internal::Group*;
    static auto VerificationGroup(
        identity::wot::verification::internal::Set& parent,
        const proto::VerificationGroup& serialized,
        bool external) -> identity::wot::verification::internal::Group*;
    static auto VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const opentxs::Identifier& claim,
        const identity::Nym& signer,
        const opentxs::PasswordPrompt& reason,
        const bool value,
        const Time start,
        const Time end,
        const VersionNumber version)
        -> identity::wot::verification::internal::Item*;
    static auto VerificationItem(
        const identity::wot::verification::internal::Nym& parent,
        const proto::Verification& serialized)
        -> identity::wot::verification::internal::Item*;
    static auto VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const identifier::Nym& nym,
        const VersionNumber version)
        -> identity::wot::verification::internal::Nym*;
    static auto VerificationNym(
        identity::wot::verification::internal::Group& parent,
        const proto::VerificationIdentity& serialized)
        -> identity::wot::verification::internal::Nym*;
    static auto VerificationSet(
        const api::internal::Core& api,
        const identifier::Nym& nym,
        const VersionNumber version)
        -> identity::wot::verification::internal::Set*;
    static auto VerificationSet(
        const api::internal::Core& api,
        const identifier::Nym& nym,
        const proto::VerificationSet& serialized)
        -> identity::wot::verification::internal::Set*;
    static auto Wallet(const api::client::internal::Manager& client)
        -> api::Wallet*;
    static auto Wallet(const api::server::internal::Manager& server)
        -> api::Wallet*;
    static auto Workflow(
        const api::internal::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contact) -> api::client::Workflow*;
    static auto ZAP(const network::zeromq::Context& context)
        -> api::network::ZAP*;
    static auto ZMQ(const api::internal::Core& api, const Flag& running)
        -> api::network::ZMQ*;
    static auto ZMQContext() -> network::zeromq::Context*;
    OPENTXS_EXPORT static auto ZMQFrame() -> network::zeromq::Frame*;
    OPENTXS_EXPORT static auto ZMQFrame(
        const void* data,
        const std::size_t size) -> network::zeromq::Frame*;
    OPENTXS_EXPORT static auto ZMQFrame(const ProtobufType& data)
        -> network::zeromq::Frame*;
    OPENTXS_EXPORT static auto ZMQMessage() -> network::zeromq::Message*;
    OPENTXS_EXPORT static auto ZMQMessage(
        const void* data,
        const std::size_t size) -> network::zeromq::Message*;
    OPENTXS_EXPORT static auto ZMQMessage(const ProtobufType& data)
        -> network::zeromq::Message*;
};
}  // namespace opentxs