// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/otx/blind/CashType.hpp"

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/BasketContract.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"      // TODO remove
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"  // TODO remove
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

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
namespace bitcoin
{
namespace block
{
class Block;
class Script;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
class Hash;
class Header;
}  // namespace block

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace crypto
{
class SymmetricProvider;
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

namespace network
{
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

namespace proto
{
class BlockchainPeerAddress;
}  // namespace proto

class ByteArray;
class Item;
class Secret;
class PaymentCode;
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
    virtual auto Armored() const -> OTArmored = 0;
    virtual auto Armored(const UnallocatedCString& input) const
        -> OTArmored = 0;
    virtual auto Armored(const opentxs::Data& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::String& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored = 0;
    virtual auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::Asymmetric::DefaultVersion) const
        -> OTAsymmetricKey = 0;
    virtual auto BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const identifier::Generic& requestID,
        const UnallocatedCString& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentNotice = 0;
    virtual auto BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentReply = 0;
    virtual auto BailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentRequest = 0;
    virtual auto BailmentRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTBailmentRequest = 0;
    virtual auto BasketContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTBasketContract = 0;
    virtual auto BitcoinBlock(
        const blockchain::Type chain,
        const ReadView bytes) const noexcept
        -> std::shared_ptr<const blockchain::bitcoin::block::Block> = 0;
    using Transaction_p =
        std::shared_ptr<const blockchain::bitcoin::block::Transaction>;
    using AbortFunction = std::function<bool()>;
    virtual auto BitcoinBlock(
        const blockchain::block::Header& previous,
        const Transaction_p generationTransaction,
        const std::uint32_t nBits,
        const UnallocatedVector<Transaction_p>& extraTransactions = {},
        const std::int32_t version = 2,
        const AbortFunction abort = {}) const noexcept
        -> std::shared_ptr<const blockchain::bitcoin::block::Block> = 0;
    virtual auto BitcoinGenerationTransaction(
        const blockchain::Type chain,
        const blockchain::block::Height height,
        UnallocatedVector<blockchain::OutputBuilder>&& outputs,
        const UnallocatedCString& coinbase = {},
        const std::int32_t version = 1) const noexcept -> Transaction_p = 0;
    virtual auto BitcoinScriptNullData(
        const blockchain::Type chain,
        const UnallocatedVector<ReadView>& data) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2MS(
        const blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        const UnallocatedVector<const opentxs::crypto::key::EllipticCurve*>&
            publicKeys) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2PK(
        const blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2PKH(
        const blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2SH(
        const blockchain::Type chain,
        const blockchain::bitcoin::block::Script& script) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2WPKH(
        const blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinScriptP2WSH(
        const blockchain::Type chain,
        const blockchain::bitcoin::block::Script& script) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Script> = 0;
    virtual auto BitcoinTransaction(
        const blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time& time = Clock::now()) const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Transaction> = 0;
    virtual auto BlockchainAddress(
        const blockchain::p2p::Protocol protocol,
        const blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const UnallocatedSet<blockchain::p2p::Service>& services,
        const bool incoming = false) const -> blockchain::p2p::Address = 0;
    virtual auto BlockchainAddress(
        const proto::BlockchainPeerAddress& serialized) const
        -> blockchain::p2p::Address = 0;
    virtual auto BlockchainSyncMessage(
        const opentxs::network::zeromq::Message& in) const noexcept
        -> std::unique_ptr<opentxs::network::otdht::Base> = 0;
    using BlockHeaderP = std::unique_ptr<blockchain::block::Header>;
    virtual auto BlockHeader(const ReadView protobuf) const -> BlockHeaderP = 0;
    virtual auto BlockHeader(const blockchain::Type type, const ReadView native)
        const -> BlockHeaderP = 0;
    virtual auto BlockHeader(const blockchain::block::Block& block) const
        -> BlockHeaderP = 0;
    virtual auto BlockHeaderForUnitTests(
        const blockchain::block::Hash& hash,
        const blockchain::block::Hash& parent,
        const blockchain::block::Height height) const -> BlockHeaderP = 0;
    virtual auto ConnectionReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const bool ack,
        const UnallocatedCString& url,
        const UnallocatedCString& login,
        const UnallocatedCString& password,
        const UnallocatedCString& key,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionReply = 0;
    virtual auto ConnectionRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const contract::peer::ConnectionInfoType type,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionRequest = 0;
    virtual auto CurrencyContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTCurrencyContract = 0;
    virtual auto Data() const -> ByteArray = 0;
    virtual auto Data(const opentxs::Armored& input) const -> ByteArray = 0;
    virtual auto Data(const opentxs::network::zeromq::Frame& input) const
        -> ByteArray = 0;
    virtual auto Data(const std::uint8_t input) const -> ByteArray = 0;
    virtual auto Data(const std::uint32_t input) const -> ByteArray = 0;
    virtual auto Data(const UnallocatedVector<unsigned char>& input) const
        -> ByteArray = 0;
    virtual auto Data(const UnallocatedVector<std::byte>& input) const
        -> ByteArray = 0;
    virtual auto DataFromBytes(const ReadView input) const -> ByteArray = 0;
    virtual auto DataFromHex(const ReadView input) const -> ByteArray = 0;
    virtual auto Envelope() const noexcept -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::Armored& ciphertext) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(
        const opentxs::crypto::Envelope::SerializedType& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::ReadView& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    OPENTXS_NO_EXPORT virtual auto InternalSession() const noexcept
        -> const internal::Factory& = 0;
    virtual auto Keypair(
        const opentxs::crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::crypto::key::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
    virtual auto Keypair(
        const UnallocatedCString& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::key::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
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
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentReply = 0;
    virtual auto OutbailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const Amount& amount,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentRequest = 0;
    virtual auto PasswordPrompt(const UnallocatedCString& text) const
        -> OTPasswordPrompt = 0;
    virtual auto PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
        -> OTPasswordPrompt = 0;
    virtual auto PaymentCode(const UnallocatedCString& base58) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCode(const ReadView& serialized) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCode(
        const UnallocatedCString& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& message) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& payment,
        const bool isPayment) const -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(const Nym_p& senderNym, otx::blind::Purse&& purse)
        const -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const OTPeerRequest request,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerReply() const noexcept -> OTPeerReply = 0;
    virtual auto PeerReply(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTPeerReply = 0;
    virtual auto PeerRequest() const noexcept -> OTPeerRequest = 0;
    virtual auto PeerRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTPeerRequest = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
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
    virtual auto ReplyAcknowledgement(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const contract::peer::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTReplyAcknowledgement = 0;
    virtual auto SecurityContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTSecurityContract = 0;
    virtual auto ServerContract() const noexcept(false) -> OTServerContract = 0;
    virtual auto StoreSecret(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const contract::peer::SecretType type,
        const UnallocatedCString& primary,
        const UnallocatedCString& secondary,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTStoreSecret = 0;
    /** Generate a blank, invalid key */
    virtual auto SymmetricKey() const -> OTSymmetricKey = 0;
    /** Derive a new, random symmetric key
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] password Optional key password information.
     *  \param[in] mode The symmetric algorithm for which to generate an
     *                  appropriate key
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::Error) const
        -> OTSymmetricKey = 0;
    /** Derive a symmetric key from a seed
     *
     *  \param[in] engine     Symmetric provider compatible with the specified
     *                        key type
     *  \param[in] seed       A binary or text seed to be expanded into a secret
     *                        key
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey = 0;
    /** Derive a symmetric key from a seed
     *
     *  \param[in] engine     Symmetric provider compatible with the specified
     *                        key type
     *  \param[in] seed       A binary or text seed to be expanded into a secret
     *                        key
     *  \param[in] salt       Extra data to pass to the KDF
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF
     *  \param[in] parallel   A type-specific difficulty parameter used by some
     *                        KDFs
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const ReadView salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const std::size_t size,
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey = 0;
    /** Construct a symmetric key from an existing Secret
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] raw An existing, unencrypted binary or text secret
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason) const -> OTSymmetricKey = 0;
    virtual auto UnitDefinition() const noexcept -> OTUnitDefinition = 0;

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
