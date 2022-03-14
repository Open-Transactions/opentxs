// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/Enums.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
namespace internal
{
class Asymmetric;
}  // namespace internal

class Blockchain;
}  // namespace crypto

namespace implementation
{
class Context;
}  // namespace implementation

namespace internal
{
class Context;
class Log;
}  // namespace internal

namespace network
{
namespace implementation
{
class Context;
class Dht;
class ZMQ;
}  // namespace implementation

class Dht;
class ZAP;
class ZMQ;
}  // namespace network

namespace session
{
class Client;
class Storage;
class Wallet;
}  // namespace session

class Context;
class Session;
class Crypto;
class Legacy;
class Settings;
}  // namespace api

namespace contract
{
namespace peer
{
namespace reply
{
class Acknowledgement;
class Bailment;
class Connection;
class Outbailment;
}  // namespace reply

namespace request
{
class Bailment;
class BailmentNotice;
class Connection;
class Outbailment;
class StoreSecret;
}  // namespace request

class Reply;
class Request;
}  // namespace peer

namespace unit
{
class Basket;
class Currency;
class Security;
}  // namespace unit

class Server;
class Signable;
class Unit;
}  // namespace contract

namespace crypto
{
namespace key
{
class Asymmetric;
class Ed25519;
class EllipticCurve;
class HD;
class Keypair;
class RSA;
class Secp256k1;
class Symmetric;
}  // namespace key

namespace implementation
{
class OpenSSL;
}  // namespace implementation

class Bip39;
class Bitcoin;
class Envelope;
class Parameters;
class Pbkdf2;
class Ripemd160;
class Scrypt;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

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

class Context;
class Message;
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

namespace identifier
{
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Source;
}  // namespace identity

namespace proto
{
class AsymmetricKey;
class Authority;
class Credential;
class Envelope;
class HDAccount;
class Issuer;
class Nym;
class NymIDSource;
class PeerObject;
class Purse;
class ServerContract;
class SymmetricKey;
class Token;
class UnitDefinition;
class Verification;
class VerificationGroup;
class VerificationIdentity;
class VerificationSet;
}  // namespace proto

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
class Flag;
class Libsecp256k1;
class Libsodium;
class PasswordCallback;
class OpenSSL;
class Options;
class PasswordPrompt;
class PeerObject;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const proto::KeyMode mode,
        const proto::Authority& serialized) -> identity::internal::Authority*;
    static auto Authority(
        const api::Session& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber nymVersion,
        const opentxs::PasswordPrompt& reason)
        -> identity::internal::Authority*;
    static auto BailmentNotice(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const opentxs::Identifier& requestID,
        const UnallocatedCString& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::BailmentNotice>;
    static auto BailmentNotice(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::BailmentNotice>;
    static auto BailmentReply(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Bailment>;
    static auto BailmentReply(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Bailment>;
    static auto BailmentRequest(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Bailment>;
    static auto BailmentRequest(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Bailment>;
    static auto BasketContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static auto BasketContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Basket>;
    static auto Bitcoin(const api::Crypto& crypto) -> crypto::Bitcoin*;
    static auto Bip39(const api::Crypto& api) noexcept
        -> std::unique_ptr<crypto::Bip39>;
    static auto ConnectionReply(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const bool ack,
        const UnallocatedCString& url,
        const UnallocatedCString& login,
        const UnallocatedCString& password,
        const UnallocatedCString& key,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Connection>;
    static auto ConnectionReply(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Connection>;
    static auto ConnectionRequest(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const contract::peer::ConnectionInfoType type,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Connection>;
    static auto ConnectionRequest(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Connection>;
    static auto ContactCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Contact*;
    static auto ContactCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Contact*;
    template <class C>
    static auto Credential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const VersionNumber version,
        const crypto::Parameters& parameters,
        const proto::CredentialRole role,
        const opentxs::PasswordPrompt& reason) -> C*;
    template <class C>
    static auto Credential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role) -> C*;
    static auto CurrencyContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static auto CurrencyContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Currency>;
    static auto Envelope(const api::Session& api) noexcept
        -> std::unique_ptr<crypto::Envelope>;
    static auto Envelope(
        const api::Session& api,
        const proto::Envelope& serialized) noexcept(false)
        -> std::unique_ptr<crypto::Envelope>;
    static auto Envelope(
        const api::Session& api,
        const ReadView& serialized) noexcept(false)
        -> std::unique_ptr<crypto::Envelope>;
    static auto NoticeAcknowledgement(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const contract::peer::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Acknowledgement>;
    static auto NoticeAcknowledgement(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Acknowledgement>;
    static auto NullCallback() -> PasswordCallback*;
    static auto Nym(
        const api::Session& api,
        const crypto::Parameters& nymParameters,
        const identity::Type type,
        const UnallocatedCString name,
        const opentxs::PasswordPrompt& reason) -> identity::internal::Nym*;
    static auto Nym(
        const api::Session& api,
        const proto::Nym& serialized,
        const UnallocatedCString& alias) -> identity::internal::Nym*;
    static auto Nym(
        const api::Session& api,
        const ReadView& serialized,
        const UnallocatedCString& alias) -> identity::internal::Nym*;
    static auto NymFile(
        const api::Session& api,
        Nym_p targetNym,
        Nym_p signerNym) -> internal::NymFile*;
    static auto NymIDSource(
        const api::Session& api,
        crypto::Parameters& parameters,
        const opentxs::PasswordPrompt& reason) -> identity::Source*;
    static auto NymIDSource(
        const api::Session& api,
        const proto::NymIDSource& serialized) -> identity::Source*;
    static auto Operation(
        const api::session::Client& api,
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason)
        -> otx::client::internal::Operation*;
    static auto OutBailmentReply(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::reply::Outbailment>;
    static auto OutBailmentReply(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerReply& serialized) noexcept
        -> std::shared_ptr<contract::peer::reply::Outbailment>;
    static auto OutbailmentRequest(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const Amount& amount,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::Outbailment>;
    static auto OutbailmentRequest(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::Outbailment>;
    static auto PasswordPrompt(
        const api::Session& api,
        const UnallocatedCString& text) -> opentxs::PasswordPrompt*;
    static auto PrimaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Primary*;
    static auto PrimaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const proto::Credential& credential)
        -> identity::credential::internal::Primary*;
    static auto RPC(const api::Context& native) -> rpc::internal::RPC*;
    static auto SecondaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Secondary*;
    static auto SecondaryCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& credential)
        -> identity::credential::internal::Secondary*;
    static auto SecurityContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static auto SecurityContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::unit::Security>;
    static auto ServerContract(const api::Session& api) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::Session& api,
        const Nym_p& nym,
        const UnallocatedList<Endpoint>& endpoints,
        const UnallocatedCString& terms,
        const UnallocatedCString& name,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto ServerContract(
        const api::Session& api,
        const Nym_p& nym,
        const proto::ServerContract& serialized) noexcept
        -> std::unique_ptr<contract::Server>;
    static auto StoreSecret(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const contract::peer::SecretType type,
        const UnallocatedCString& primary,
        const UnallocatedCString& secondary,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) noexcept
        -> std::shared_ptr<contract::peer::request::StoreSecret>;
    static auto StoreSecret(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized) noexcept
        -> std::shared_ptr<contract::peer::request::StoreSecret>;
    static auto UnitDefinition(const api::Session& api) noexcept
        -> std::shared_ptr<contract::Unit>;
    static auto UnitDefinition(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized) noexcept
        -> std::shared_ptr<contract::Unit>;
    static auto VerificationCredential(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Verification*;
    static auto VerificationCredential(
        const api::Session& api,
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
        const api::Session& api,
        const identifier::Nym& nym,
        const VersionNumber version)
        -> identity::wot::verification::internal::Set*;
    static auto VerificationSet(
        const api::Session& api,
        const identifier::Nym& nym,
        const proto::VerificationSet& serialized)
        -> identity::wot::verification::internal::Set*;
    static auto ZAP(const network::zeromq::Context& context)
        -> api::network::ZAP*;
    static auto ZMQ(const api::Session& api, const Flag& running)
        -> api::network::ZMQ*;
};
}  // namespace opentxs
