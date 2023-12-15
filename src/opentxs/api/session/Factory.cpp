// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/reply/Bailment.hpp"
#include "opentxs/core/contract/peer/reply/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/reply/Connection.hpp"
#include "opentxs/core/contract/peer/reply/Faucet.hpp"
#include "opentxs/core/contract/peer/reply/Outbailment.hpp"
#include "opentxs/core/contract/peer/reply/StoreSecret.hpp"
#include "opentxs/core/contract/peer/reply/Verification.hpp"
#include "opentxs/core/contract/peer/request/Bailment.hpp"
#include "opentxs/core/contract/peer/request/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/request/Connection.hpp"
#include "opentxs/core/contract/peer/request/Faucet.hpp"
#include "opentxs/core/contract/peer/request/Outbailment.hpp"
#include "opentxs/core/contract/peer/request/StoreSecret.hpp"
#include "opentxs/core/contract/peer/request/Verification.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/Verification.hpp"
#include "opentxs/network/otdht/Base.hpp"  // IWYU pragma: keep
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

namespace opentxs::api::session
{
Factory::Factory(api::internal::Factory* imp) noexcept
    : api::Factory(imp)
{
}

auto Factory::AsymmetricKey(
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(params, reason);
}

auto Factory::AsymmetricKey(
    VersionNumber version,
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(version, params, reason);
}

auto Factory::AsymmetricKey(
    opentxs::crypto::asymmetric::Role role,
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(role, params, reason);
}

auto Factory::AsymmetricKey(
    VersionNumber version,
    opentxs::crypto::asymmetric::Role role,
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(version, role, params, reason);
}

auto Factory::AsymmetricKey(
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(type, key, reason, alloc);
}

auto Factory::AsymmetricKey(
    VersionNumber version,
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(version, type, key, reason, alloc);
}

auto Factory::AsymmetricKey(
    opentxs::crypto::asymmetric::Role role,
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(role, type, key, reason, alloc);
}

auto Factory::AsymmetricKey(
    VersionNumber version,
    opentxs::crypto::asymmetric::Role role,
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    return imp_->Session().AsymmetricKey(
        version, role, type, key, reason, alloc);
}

auto Factory::BailmentNoticeReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    bool value,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::reply::BailmentNotice
{
    return imp_->Session().BailmentNoticeReply(
        responder, initiator, inReferenceToRequest, value, reason, alloc);
}

auto Factory::BailmentNoticeRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& notary,
    const identifier::Generic& inReferenceToRequest,
    std::string_view description,
    const opentxs::Amount& amount,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::request::BailmentNotice
{
    return imp_->Session().BailmentNoticeRequest(
        initiator,
        responder,
        unit,
        notary,
        inReferenceToRequest,
        description,
        amount,
        reason,
        alloc);
}

auto Factory::BailmentReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    std::string_view terms,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Bailment
{
    return imp_->Session().BailmentReply(
        responder, initiator, inReferenceToRequest, terms, reason, alloc);
}

auto Factory::BailmentRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& notary,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::request::Bailment
{
    return imp_->Session().BailmentRequest(
        initiator, responder, unit, notary, reason, alloc);
}

auto Factory::BitcoinScriptNullData(
    const blockchain::Type chain,
    std::span<const ReadView> data,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return imp_->Session().BitcoinScriptNullData(chain, data, alloc);
}

auto Factory::BitcoinScriptP2MS(
    const blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return imp_->Session().BitcoinScriptP2MS(chain, M, N, keys, alloc);
}

auto Factory::BitcoinScriptP2PK(
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return imp_->Session().BitcoinScriptP2PK(chain, key, alloc);
}

auto Factory::BitcoinScriptP2PKH(
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return imp_->Session().BitcoinScriptP2PKH(chain, key, alloc);
}

auto Factory::BitcoinScriptP2SH(
    const blockchain::Type chain,
    const blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return imp_->Session().BitcoinScriptP2SH(chain, script, alloc);
}

auto Factory::BitcoinScriptP2WPKH(
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return imp_->Session().BitcoinScriptP2WPKH(chain, key, alloc);
}

auto Factory::BitcoinScriptP2WSH(
    const blockchain::Type chain,
    const blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return imp_->Session().BitcoinScriptP2WSH(chain, script, alloc);
}

auto Factory::BlockchainBlock(
    const blockchain::Type chain,
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> blockchain::block::Block
{
    return imp_->Session().BlockchainBlock(chain, bytes, alloc);
}

auto Factory::BlockchainSyncMessage(const opentxs::network::zeromq::Message& in)
    const noexcept -> std::unique_ptr<opentxs::network::otdht::Base>
{
    return imp_->Session().BlockchainSyncMessage(in);
}

auto Factory::BlockchainTransaction(
    const blockchain::Type chain,
    const ReadView bytes,
    const bool isGeneration,
    const Time time,
    alloc::Default alloc) const noexcept -> blockchain::block::Transaction
{
    return imp_->Session().BlockchainTransaction(
        chain, bytes, isGeneration, time, alloc);
}

auto Factory::BlockchainTransaction(
    const blockchain::Type chain,
    const blockchain::block::Height height,
    std::span<blockchain::OutputBuilder> outputs,
    ReadView coinbase,
    std::int32_t version,
    alloc::Default alloc) const noexcept -> blockchain::block::Transaction
{
    return imp_->Session().BlockchainTransaction(
        chain, height, outputs, coinbase, version, alloc);
}

auto Factory::BlockHeaderFromNative(
    const blockchain::Type type,
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> blockchain::block::Header
{
    return imp_->Session().BlockHeaderFromNative(type, bytes, alloc);
}

auto Factory::BlockHeaderFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> blockchain::block::Header
{
    return imp_->Session().BlockHeaderFromProtobuf(bytes, alloc);
}

auto Factory::Claim(
    const identity::wot::Claimant& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    ReadView subtype,
    VersionNumber version,
    alloc::Strategy alloc) const noexcept -> identity::wot::Claim
{
    return imp_->Session().Claim(
        claimant,
        section,
        type,
        value,
        subtype,
        attributes,
        start,
        stop,
        version,
        alloc);
}

auto Factory::Claim(
    const identity::Nym& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    ReadView subtype,
    VersionNumber version,
    alloc::Strategy alloc) const noexcept -> identity::wot::Claim
{
    return imp_->Session().Claim(
        claimant,
        section,
        type,
        value,
        subtype,
        attributes,
        start,
        stop,
        version,
        alloc);
}

auto Factory::Claim(ReadView serialized, alloc::Strategy alloc) const noexcept
    -> identity::wot::Claim
{
    return imp_->Session().Claim(serialized, alloc);
}

auto Factory::ConnectionReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    bool accepted,
    std::string_view url,
    std::string_view login,
    std::string_view password,
    std::string_view key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Connection
{
    return imp_->Session().ConnectionReply(
        responder,
        initiator,
        inReferenceToRequest,
        accepted,
        url,
        login,
        password,
        key,
        reason,
        alloc);
}

auto Factory::ConnectionRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const contract::peer::ConnectionInfoType kind,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::request::Connection
{
    return imp_->Session().ConnectionRequest(
        initiator, responder, kind, reason, alloc);
}

auto Factory::FaucetReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    const blockchain::block::Transaction& transaction,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Faucet
{
    return imp_->Session().FaucetReply(
        responder, initiator, inReferenceToRequest, transaction, reason, alloc);
}

auto Factory::FaucetRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    opentxs::UnitType unit,
    std::string_view address,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::request::Faucet
{
    return imp_->Session().FaucetRequest(
        initiator, responder, unit, address, reason, alloc);
}

auto Factory::Mint() const noexcept -> otx::blind::Mint
{
    return imp_->Session().Mint();
}

auto Factory::Mint(const otx::blind::CashType type) const noexcept
    -> otx::blind::Mint
{
    return imp_->Session().Mint(type);
}

auto Factory::Mint(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return imp_->Session().Mint(notary, unit);
}

auto Factory::Mint(
    const otx::blind::CashType type,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return imp_->Session().Mint(type, notary, unit);
}

auto Factory::Mint(
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return imp_->Session().Mint(notary, serverNym, unit);
}

auto Factory::Mint(
    const otx::blind::CashType type,
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return imp_->Session().Mint(type, notary, serverNym, unit);
}

auto Factory::NymIDFromPaymentCode(const UnallocatedCString& serialized) const
    -> identifier::Nym
{
    return imp_->Session().NymIDFromPaymentCode(serialized);
}

auto Factory::OutbailmentReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    std::string_view description,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Outbailment
{
    return imp_->Session().OutbailmentReply(
        responder, initiator, inReferenceToRequest, description, reason, alloc);
}

auto Factory::OutbailmentRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& notary,
    const opentxs::Amount& amount,
    std::string_view instructions,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::request::Outbailment
{
    return imp_->Session().OutbailmentRequest(
        initiator,
        responder,
        unitID,
        notary,
        amount,
        instructions,
        reason,
        alloc);
}

auto Factory::PasswordPrompt(std::string_view text) const
    -> opentxs::PasswordPrompt
{
    return imp_->Session().PasswordPrompt(text);
}

auto Factory::PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
    -> opentxs::PasswordPrompt
{
    return imp_->Session().PasswordPrompt(rhs);
}

auto Factory::PaymentCode(
    const opentxs::crypto::SeedID& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const opentxs::PasswordPrompt& reason,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const noexcept -> opentxs::PaymentCode
{
    return imp_->Session().PaymentCode(
        seed, nym, version, reason, false, bitmessageVersion, bitmessageStream);
}

auto Factory::PaymentCodeFromBase58(const ReadView base58) const noexcept
    -> opentxs::PaymentCode
{
    return imp_->Session().PaymentCodeFromBase58(base58);
}

auto Factory::PaymentCodeFromProtobuf(const ReadView proto) const noexcept
    -> opentxs::PaymentCode
{
    return imp_->Session().PaymentCodeFromProtobuf(proto);
}

auto Factory::PeerReply(ReadView bytes, alloc::Strategy alloc) const noexcept
    -> contract::peer::Reply
{
    return imp_->Session().PeerReply(bytes, alloc);
}

auto Factory::PeerReply(
    const opentxs::network::zeromq::Frame& bytes,
    alloc::Strategy alloc) const noexcept -> contract::peer::Reply
{
    return imp_->Session().PeerReply(bytes, alloc);
}

auto Factory::PeerRequest(ReadView bytes, alloc::Strategy alloc) const noexcept
    -> contract::peer::Request
{
    return imp_->Session().PeerRequest(bytes, alloc);
}

auto Factory::PeerRequest(
    const opentxs::network::zeromq::Frame& bytes,
    alloc::Strategy alloc) const noexcept -> contract::peer::Request
{
    return imp_->Session().PeerRequest(bytes, alloc);
}

auto Factory::Purse(
    const otx::context::Server& context,
    const identifier::UnitDefinition& unit,
    const otx::blind::Mint& mint,
    const opentxs::Amount& totalValue,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return imp_->Session().Purse(context, unit, mint, totalValue, reason);
}

auto Factory::Purse(
    const otx::context::Server& context,
    const identifier::UnitDefinition& unit,
    const otx::blind::Mint& mint,
    const opentxs::Amount& totalValue,
    const otx::blind::CashType type,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return imp_->Session().Purse(context, unit, mint, totalValue, type, reason);
}

auto Factory::Purse(
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return imp_->Session().Purse(owner, server, unit, reason);
}

auto Factory::Purse(
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const otx::blind::CashType type,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return imp_->Session().Purse(owner, server, unit, type, reason);
}

auto Factory::StoreSecretReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    bool value,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::StoreSecret
{
    return imp_->Session().StoreSecretReply(
        responder, initiator, inReferenceToRequest, value, reason, alloc);
}

auto Factory::StoreSecretRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const contract::peer::SecretType kind,
    std::span<const std::string_view> data,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::request::StoreSecret
{
    return imp_->Session().StoreSecretRequest(
        initiator, responder, kind, data, reason, alloc);
}

auto Factory::Verification(
    const identifier::Nym& verifier,
    const opentxs::PasswordPrompt& reason,
    identity::wot::ClaimID claim,
    identity::wot::verification::Type value,
    Time start,
    Time stop,
    std::span<const identity::wot::VerificationID> superscedes,
    alloc::Strategy alloc) const noexcept -> identity::wot::Verification
{
    return imp_->Session().Verification(
        verifier, reason, claim, value, start, stop, superscedes, alloc);
}

auto Factory::Verification(
    const identity::Nym& verifier,
    const opentxs::PasswordPrompt& reason,
    identity::wot::ClaimID claim,
    identity::wot::verification::Type value,
    Time start,
    Time stop,
    std::span<const identity::wot::VerificationID> superscedes,
    alloc::Strategy alloc) const noexcept -> identity::wot::Verification
{
    return imp_->Session().Verification(
        verifier, reason, claim, value, start, stop, superscedes, alloc);
}

auto Factory::Verification(ReadView serialized, alloc::Strategy alloc)
    const noexcept -> identity::wot::Verification
{
    return imp_->Session().Verification(serialized, alloc);
}

auto Factory::VerificationReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    const std::optional<identity::wot::Verification>& response,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Verification
{
    return imp_->Session().VerificationReply(
        responder, initiator, inReferenceToRequest, response, reason, alloc);
}

auto Factory::VerificationRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const identity::wot::Claim& claim,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::request::Verification
{
    return imp_->Session().VerificationRequest(
        initiator, responder, claim, reason, alloc);
}
}  // namespace opentxs::api::session
