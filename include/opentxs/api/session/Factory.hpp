// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>  // IWYU pragma: keep
#include <span>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Factory;
}  // namespace internal
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
class Hash;
class Header;
class Transaction;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Script;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace contract
{
namespace peer
{
namespace reply
{
class Bailment;
class BailmentNotice;
class Connection;
class Faucet;
class Outbailment;
class StoreSecret;
class Verification;
}  // namespace reply

namespace request
{
class Bailment;
class BailmentNotice;
class Connection;
class Faucet;
class Outbailment;
class StoreSecret;
class Verification;
}  // namespace request

class Reply;
class Request;
}  // namespace peer
}  // namespace contract

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key

class Key;
}  // namespace asymmetric

class Parameters;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

namespace identifier
{
class Generic;
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
namespace wot
{
class Claim;
class Verification;
}  // namespace wot

class Nym;
}  // namespace identity

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain

namespace otdht
{
class Base;
}  // namespace otdht

namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace blind
{
class Mint;
class Purse;
}  // namespace blind

namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class Armored;
class ByteArray;
class Item;
class Secret;
class PaymentCode;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
/**
 The Factory API for opentxs sessions, used for instantiating many different
 object types native to opentxs.
 */
class OPENTXS_EXPORT Factory : virtual public api::Factory
{
public:
    virtual auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto BailmentNoticeReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool value,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::BailmentNotice = 0;
    virtual auto BailmentNoticeRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& notary,
        const identifier::Generic& inReferenceToRequest,
        std::string_view description,
        const opentxs::Amount& amount,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::BailmentNotice = 0;
    virtual auto BailmentReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        std::string_view terms,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Bailment = 0;
    virtual auto BailmentRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& notary,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Bailment = 0;
    virtual auto BitcoinScriptNullData(
        const blockchain::Type chain,
        std::span<const ReadView> data,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2MS(
        const blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2PK(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2PKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2SH(
        const blockchain::Type chain,
        const blockchain::protocol::bitcoin::base::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2WPKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2WSH(
        const blockchain::Type chain,
        const blockchain::protocol::bitcoin::base::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const -> opentxs::network::blockchain::Address = 0;
    virtual auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const -> opentxs::network::blockchain::Address = 0;
    virtual auto BlockchainBlock(
        const blockchain::Type chain,
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept
        -> blockchain::block::Block = 0;
    virtual auto BlockchainSyncMessage(
        const opentxs::network::zeromq::Message& in) const noexcept
        -> std::unique_ptr<opentxs::network::otdht::Base> = 0;
    virtual auto BlockchainTransaction(
        const blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time time,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction = 0;
    virtual auto BlockchainTransaction(
        const blockchain::Type chain,
        const blockchain::block::Height height,
        std::span<blockchain::OutputBuilder> outputs,
        ReadView coinbase,
        std::int32_t version,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction = 0;
    virtual auto BlockHeaderFromNative(
        const blockchain::Type type,
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto BlockHeaderFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto Claim(
        const identifier::Nym& claimant,
        identity::wot::claim::SectionType section,
        identity::wot::claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        std::span<const identity::wot::claim::Attribute> attributes = {},
        Time start = {},
        Time stop = {},
        alloc::Strategy alloc = {}) const noexcept -> identity::wot::Claim = 0;
    virtual auto Claim(
        const identity::Nym& claimant,
        identity::wot::claim::SectionType section,
        identity::wot::claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        std::span<const identity::wot::claim::Attribute> attributes = {},
        Time start = {},
        Time stop = {},
        alloc::Strategy alloc = {}) const noexcept -> identity::wot::Claim = 0;
    virtual auto Claim(ReadView serialized, alloc::Strategy alloc = {})
        const noexcept -> identity::wot::Claim = 0;
    virtual auto ConnectionReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool accepted,
        std::string_view url,
        std::string_view login,
        std::string_view password,
        std::string_view key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Connection = 0;
    virtual auto ConnectionRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const contract::peer::ConnectionInfoType kind,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Connection = 0;
    virtual auto FaucetReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        const blockchain::block::Transaction& transaction,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Faucet = 0;
    virtual auto FaucetRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        opentxs::UnitType unit,
        std::string_view address,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Faucet = 0;
    OPENTXS_NO_EXPORT virtual auto InternalSession() const noexcept
        -> const internal::Factory& = 0;
    virtual auto Mint() const noexcept -> otx::blind::Mint = 0;
    virtual auto Mint(const otx::blind::CashType type) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto NymIDFromPaymentCode(
        const UnallocatedCString& serialized) const -> identifier::Nym = 0;
    virtual auto OutbailmentReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        std::string_view description,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Outbailment = 0;
    virtual auto OutbailmentRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& notary,
        const opentxs::Amount& amount,
        std::string_view instructions,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Outbailment = 0;
    virtual auto PasswordPrompt(std::string_view text) const
        -> opentxs::PasswordPrompt = 0;
    virtual auto PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
        -> opentxs::PasswordPrompt = 0;
    virtual auto PaymentCode(
        const opentxs::crypto::SeedID& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCodeFromBase58(const ReadView base58) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCodeFromProtobuf(const ReadView proto) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PeerReply(ReadView bytes, alloc::Strategy alloc = {})
        const noexcept -> contract::peer::Reply = 0;
    virtual auto PeerReply(
        const opentxs::network::zeromq::Frame& bytes,
        alloc::Strategy alloc = {}) const noexcept -> contract::peer::Reply = 0;
    virtual auto PeerRequest(ReadView bytes, alloc::Strategy alloc = {})
        const noexcept -> contract::peer::Request = 0;
    virtual auto PeerRequest(
        const opentxs::network::zeromq::Frame& bytes,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::Request = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const opentxs::Amount& totalValue,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const opentxs::Amount& totalValue,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto StoreSecretReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool value,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::StoreSecret = 0;
    virtual auto StoreSecretRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const contract::peer::SecretType kind,
        std::span<const std::string_view> data,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::StoreSecret = 0;
    virtual auto Verification(
        const identifier::Nym& verifier,
        const opentxs::PasswordPrompt& reason,
        identity::wot::ClaimID claim,
        identity::wot::verification::Type value,
        Time start = {},
        Time stop = {},
        std::span<const identity::wot::VerificationID> superscedes = {},
        alloc::Strategy alloc = {}) const noexcept
        -> identity::wot::Verification = 0;
    virtual auto Verification(
        const identity::Nym& verifier,
        const opentxs::PasswordPrompt& reason,
        identity::wot::ClaimID claim,
        identity::wot::verification::Type value,
        Time start = {},
        Time stop = {},
        std::span<const identity::wot::VerificationID> superscedes = {},
        alloc::Strategy alloc = {}) const noexcept
        -> identity::wot::Verification = 0;
    virtual auto Verification(ReadView serialized, alloc::Strategy alloc = {})
        const noexcept -> identity::wot::Verification = 0;
    virtual auto VerificationReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        const std::optional<identity::wot::Verification>& response,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Verification = 0;
    virtual auto VerificationRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identity::wot::Claim& claim,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Verification = 0;

    OPENTXS_NO_EXPORT virtual auto InternalSession() noexcept
        -> internal::Factory& = 0;

    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    OPENTXS_NO_EXPORT ~Factory() override = default;

protected:
    Factory() = default;
};
}  // namespace opentxs::api::session
