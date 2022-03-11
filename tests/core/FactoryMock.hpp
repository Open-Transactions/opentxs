//
// Created by adam.tkaczyk on 2/28/22.
//

#pragma once

#include <gmock/gmock.h>
#include <google/protobuf/message_lite.h>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/basket/Basket.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/crypto/OTSignedFile.hpp"
#include "internal/otx/common/trade/OTMarket.hpp"
#include "internal/otx/common/trade/OTTrade.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
#include "serialization/protobuf/BlockchainPeerAddress.pb.h"
#include "serialization/protobuf/Identifier.pb.h"
#include "serialization/protobuf/PeerObject.pb.h"

namespace opentxs
{

class FactoryMock : virtual public api::session::internal::Factory
{
public:
    FactoryMock() = default;

    MOCK_METHOD(OTArmored, Armored, (), (const, final));

    MOCK_METHOD(
        OTArmored,
        Armored,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(OTArmored, Armored, (const opentxs::Data&), (const, final));

    MOCK_METHOD(OTArmored, Armored, (const opentxs::String&), (const, final));

    MOCK_METHOD(
        OTArmored,
        Armored,
        (const opentxs::crypto::Envelope&),
        (const, final));

    MOCK_METHOD(
        OTAsymmetricKey,
        AsymmetricKeyHelper,
        (const opentxs::crypto::Parameters&,
         const opentxs::PasswordPrompt&,
         const opentxs::crypto::key::asymmetric::Role,
         const VersionNumber),
        (const));

    virtual auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::Asymmetric::DefaultVersion) const
        -> OTAsymmetricKey final
    {
        return AsymmetricKeyHelper(params, reason, role, version);
    }

    MOCK_METHOD(
        OTBailmentNotice,
        BailmentNotice,
        (const Nym_p&,
         const identifier::Nym&,
         const identifier::UnitDefinition&,
         const identifier::Notary&,
         const opentxs::Identifier&,
         const UnallocatedCString&,
         const Amount&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTBailmentReply,
        BailmentReply,
        (const Nym_p& nym,
         const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         const UnallocatedCString&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTBailmentRequest,
        BailmentRequest,
        (const Nym_p&,
         const identifier::Nym&,
         const identifier::UnitDefinition&,
         const identifier::Notary&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTBailmentRequest,
        BailmentRequest,
        (const Nym_p&, const ReadView&),
        (const, final));

    MOCK_METHOD(
        OTBasketContract,
        BasketContract,
        (const Nym_p&,
         const UnallocatedCString&,
         const UnallocatedCString&,
         const std::uint64_t,
         const UnitType,
         const VersionNumber,
         const display::Definition&,
         const Amount&),
        (const, final));

#if OT_BLOCKCHAIN

    MOCK_METHOD(
        std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>,
        BitcoinBlock,
        (const opentxs::blockchain::Type, const ReadView),
        (const, noexcept, final));

    MOCK_METHOD(
        std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>,
        BitcoinBlockHelper,
        (const opentxs::blockchain::block::Header&,
         const Transaction_p,
         const std::uint32_t,
         const UnallocatedVector<Transaction_p>&,
         const std::int32_t,
         const AbortFunction),
        (const, noexcept));

    auto BitcoinBlock(
        const opentxs::blockchain::block::Header& previous,
        const Transaction_p generationTransaction,
        const std::uint32_t nBits,
        const UnallocatedVector<Transaction_p>& extraTransactions = {},
        const std::int32_t version = 2,
        const AbortFunction abort = {}) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::block::bitcoin::Block> final
    {
        return BitcoinBlockHelper(
            previous,
            generationTransaction,
            nBits,
            extraTransactions,
            version,
            abort);
    }

    MOCK_METHOD(
        Transaction_p,
        BitcoinGenerationTransactionHelper,
        (const opentxs::blockchain::Type,
         const opentxs::blockchain::block::Height,
         UnallocatedVector<OutputBuilder>&&,
         const UnallocatedCString& coinbase,
         const std::int32_t),
        (const, noexcept));

    virtual auto BitcoinGenerationTransaction(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Height height,
        UnallocatedVector<OutputBuilder>&& outputs,
        const UnallocatedCString& coinbase = {},
        const std::int32_t version = 1) const noexcept -> Transaction_p final
    {
        return BitcoinGenerationTransactionHelper(
            chain, height, std::move(outputs), coinbase, version);
    };

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        BitcoinScriptNullData,
        (const opentxs::blockchain::Type, const UnallocatedVector<ReadView>&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        BitcoinScriptP2MS,
        (const opentxs::blockchain::Type,
         const std::uint8_t,
         const std::uint8_t,
         const UnallocatedVector<const opentxs::crypto::key::EllipticCurve*>&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        BitcoinScriptP2PK,
        (const opentxs::blockchain::Type,
         const opentxs::crypto::key::EllipticCurve&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        BitcoinScriptP2PKH,
        (const opentxs::blockchain::Type,
         const opentxs::crypto::key::EllipticCurve&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        BitcoinScriptP2SH,
        (const opentxs::blockchain::Type,
         const opentxs::blockchain::block::bitcoin::Script&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        BitcoinScriptP2WPKH,
        (const opentxs::blockchain::Type,
         const opentxs::crypto::key::EllipticCurve&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        BitcoinScriptP2WSH,
        (const opentxs::blockchain::Type,
         const opentxs::blockchain::block::bitcoin::Script&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Transaction>,
        BitcoinTransactionHelper,
        (const opentxs::blockchain::Type,
         const ReadView,
         const bool,
         const Time&),
        (const));

    virtual auto BitcoinTransaction(
        const opentxs::blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time& time = Clock::now()) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction>
    {
        return BitcoinTransactionHelper(chain, bytes, isGeneration, time);
    }

    MOCK_METHOD(
        OTBlockchainAddress,
        BlockchainAddressHelper,
        (const opentxs::blockchain::p2p::Protocol,
         const opentxs::blockchain::p2p::Network,
         const opentxs::Data&,
         const std::uint16_t,
         const opentxs::blockchain::Type,
         const Time,
         const UnallocatedSet<opentxs::blockchain::p2p::Service>&,
         const bool),
        (const));

    virtual auto BlockchainAddress(
        const opentxs::blockchain::p2p::Protocol protocol,
        const opentxs::blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const opentxs::blockchain::Type chain,
        const Time lastConnected,
        const UnallocatedSet<opentxs::blockchain::p2p::Service>& services,
        const bool incoming = false) const -> OTBlockchainAddress
    {
        return BlockchainAddressHelper(
            protocol,
            network,
            bytes,
            port,
            chain,
            lastConnected,
            services,
            incoming);
    }

    MOCK_METHOD(
        OTBlockchainAddress,
        BlockchainAddress,
        (const opentxs::blockchain::p2p::Address::SerializedType&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::network::p2p::Base>,
        BlockchainSyncMessage,
        (const opentxs::network::zeromq::Message&),
        (const, noexcept, final));

    MOCK_METHOD(BlockHeaderP, BlockHeader, (const ReadView), (const, final));

    MOCK_METHOD(
        BlockHeaderP,
        BlockHeader,
        (const opentxs::blockchain::Type, const ReadView),
        (const, final));

    MOCK_METHOD(
        BlockHeaderP,
        BlockHeader,
        (const opentxs::blockchain::block::Block&),
        (const, final));

    MOCK_METHOD(
        BlockHeaderP,
        BlockHeaderForUnitTests,
        (const opentxs::blockchain::block::Hash&,
         const opentxs::blockchain::block::Hash&,
         const opentxs::blockchain::block::Height),
        (const, final));

#endif  // OT_BLOCKCHAIN

    MOCK_METHOD(
        OTConnectionReply,
        ConnectionReply,
        (const Nym_p&,
         const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         const bool ack,
         const UnallocatedCString&,
         const UnallocatedCString&,
         const UnallocatedCString&,
         const UnallocatedCString&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTConnectionRequest,
        ConnectionRequest,
        (const Nym_p&,
         const identifier::Nym&,
         const contract::peer::ConnectionInfoType,
         const identifier::Notary&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTCurrencyContract,
        CurrencyContract,
        (const Nym_p&,
         const UnallocatedCString&,
         const UnallocatedCString&,
         const UnitType,
         const VersionNumber,
         const opentxs::PasswordPrompt&,
         const display::Definition&,
         const Amount&),
        (const, noexcept, final));

    MOCK_METHOD(OTData, Data, (const opentxs::Armored&), (const, final));

    MOCK_METHOD(OTData, Data, (), (const, final));

    MOCK_METHOD(
        OTData,
        Data,
        (const opentxs::network::zeromq::Frame&),
        (const, final));

    MOCK_METHOD(OTData, Data, (const std::uint8_t), (const, final));

    MOCK_METHOD(OTData, Data, (const std::uint32_t), (const, final));

    MOCK_METHOD(
        OTData,
        Data,
        (const UnallocatedCString&, const StringStyle),
        (const, final));

    MOCK_METHOD(
        OTData,
        Data,
        (const UnallocatedVector<unsigned char>&),
        (const, final));

    MOCK_METHOD(
        OTData,
        Data,
        (const UnallocatedVector<std::byte>&),
        (const, final));

    MOCK_METHOD(OTData, Data, (const ReadView), (const, final));

    MOCK_METHOD(OTEnvelope, Envelope, (), (const, noexcept, final));

    MOCK_METHOD(
        OTEnvelope,
        Envelope,
        (const opentxs::Armored&),
        (const, final));

    MOCK_METHOD(
        OTEnvelope,
        Envelope,
        (const opentxs::crypto::Envelope::SerializedType&),
        (const, final));

    MOCK_METHOD(
        OTEnvelope,
        Envelope,
        (const opentxs::ReadView&),
        (const, final));

    MOCK_METHOD(OTIdentifier, Identifier, (), (const, final));

    MOCK_METHOD(
        OTIdentifier,
        Identifier,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        OTIdentifier,
        Identifier,
        (const opentxs::String&),
        (const, final));

    MOCK_METHOD(
        OTIdentifier,
        Identifier,
        (const opentxs::Contract&),
        (const, final));

    MOCK_METHOD(
        OTIdentifier,
        Identifier,
        (const opentxs::Item&),
        (const, final));

    //    MOCK_METHOD(
    //        OTIdentifier,
    //        Identifier,
    //        (const ReadView),
    //        (const, final));

    MOCK_METHOD(
        OTIdentifier,
        Identifier,
        (const opentxs::network::zeromq::Frame&),
        (const, final));

    auto Identifier(const ReadView bytes) const -> OTIdentifier final
    {
        std::cout << "Identifier(const ReadView)\n";
        return opentxs::Identifier::Factory();
    }

    MOCK_METHOD(
        OTKeypair,
        Keypair,
        (const opentxs::crypto::Parameters&,
         const VersionNumber,
         const opentxs::crypto::key::asymmetric::Role,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTKeypair,
        Keypair,
        (const UnallocatedCString&,
         const Bip32Index,
         const Bip32Index,
         const Bip32Index,
         const EcdsaCurve&,
         const opentxs::crypto::key::asymmetric::Role,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(otx::blind::Mint, Mint, (), (const, noexcept.final));

    MOCK_METHOD(
        otx::blind::Mint,
        Mint,
        (const otx::blind::CashType),
        (const, noexcept, final));

    MOCK_METHOD(
        otx::blind::Mint,
        Mint,
        (const identifier::Notary&, const identifier::UnitDefinition&),
        (const, noexcept, final));

    MOCK_METHOD(
        otx::blind::Mint,
        Mint,
        (const otx::blind::CashType,
         const identifier::Notary&,
         const identifier::UnitDefinition&),
        (const, noexcept, final));

    MOCK_METHOD(
        otx::blind::Mint,
        Mint,
        (const identifier::Notary&,
         const identifier::Nym&,
         const identifier::UnitDefinition&),
        (const, noexcept, final));

    MOCK_METHOD(
        otx::blind::Mint,
        Mint,
        (const otx::blind::CashType,
         const identifier::Notary&,
         const identifier::Nym&,
         const identifier::UnitDefinition&),
        (const, noexcept, final));

    MOCK_METHOD(OTNymID, NymID, (), (const, final));

    MOCK_METHOD(OTNymID, NymID, (const UnallocatedCString&), (const, final));

    MOCK_METHOD(OTNymID, NymID, (const opentxs::String&), (const, final));

    MOCK_METHOD(
        OTNymID,
        NymID,
        (const opentxs::network::zeromq::Frame&),
        (const, final));

    MOCK_METHOD(
        OTNymID,
        NymIDFromPaymentCode,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        OTOutbailmentReply,
        OutbailmentReply,
        (const Nym_p&,
         const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         const UnallocatedCString&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTOutbailmentRequest,
        OutbailmentRequest,
        (const Nym_p&,
         const identifier::Nym&,
         const identifier::UnitDefinition&,
         const identifier::Notary&,
         const Amount&,
         const UnallocatedCString&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTPasswordPrompt,
        PasswordPrompt,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        OTPasswordPrompt,
        PasswordPrompt,
        (const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        opentxs::PaymentCode,
        PaymentCode,
        (const UnallocatedCString&),
        (const, noexcept, final));

    MOCK_METHOD(
        opentxs::PaymentCode,
        PaymentCode,
        (const ReadView&),
        (const, noexcept, final));

    MOCK_METHOD(
        opentxs::PaymentCode,
        PaymentCodeHelper,
        (const UnallocatedCString&,
         const Bip32Index,
         const std::uint8_t,
         const opentxs::PasswordPrompt&,
         const bool,
         const std::uint8_t,
         const std::uint8_t),
        (const, noexcept));

    auto PaymentCode(
        const UnallocatedCString& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const noexcept
        -> opentxs::PaymentCode final
    {
        return PaymentCodeHelper(
            seed,
            nym,
            version,
            reason,
            bitmessage,
            bitmessageVersion,
            bitmessageStream);
    }

    MOCK_METHOD(
        std::unique_ptr<opentxs::PeerObject>,
        PeerObject,
        (const Nym_p&, const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::PeerObject>,
        PeerObject,
        (const Nym_p&, const UnallocatedCString&, const bool),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::PeerObject>,
        PeerObject,
        (const Nym_p& senderNym, otx::blind::Purse&& purse),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::PeerObject>,
        PeerObject,
        (const OTPeerRequest, const OTPeerReply, const VersionNumber),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::PeerObject>,
        PeerObject,
        (const OTPeerRequest, const VersionNumber),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::PeerObject>,
        PeerObject,
        (const Nym_p&, const opentxs::Armored&, const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(OTPeerReply, PeerReply, (), (const, noexcept, final));

    MOCK_METHOD(
        OTPeerReply,
        PeerReply,
        (const Nym_p&, const ReadView&),
        (const, final));

    MOCK_METHOD(OTPeerRequest, PeerRequest, (), (const, noexcept, final));

    MOCK_METHOD(
        OTPeerRequest,
        PeerRequest,
        (const Nym_p&, const ReadView&),
        (const, final));

    MOCK_METHOD(
        otx::blind::Purse,
        Purse,
        (const otx::context::Server&,
         const identifier::UnitDefinition&,
         const otx::blind::Mint&,
         const Amount&,
         const opentxs::PasswordPrompt&),
        (const, noexcept, final));

    MOCK_METHOD(
        otx::blind::Purse,
        Purse,
        (const otx::context::Server&,
         const identifier::UnitDefinition&,
         const otx::blind::Mint&,
         const Amount&,
         const otx::blind::CashType,
         const opentxs::PasswordPrompt&),
        (const, noexcept, final));

    MOCK_METHOD(
        otx::blind::Purse,
        Purse,
        (const identity::Nym&,
         const identifier::Notary&,
         const identifier::UnitDefinition&,
         const opentxs::PasswordPrompt&),
        (const, noexcept, final));

    MOCK_METHOD(
        otx::blind::Purse,
        Purse,
        (const identity::Nym&,
         const identifier::Notary&,
         const identifier::UnitDefinition&,
         const otx::blind::CashType,
         const opentxs::PasswordPrompt&),
        (const, noexcept final));

    MOCK_METHOD(
        OTReplyAcknowledgement,
        ReplyAcknowledgement,
        (const Nym_p&,
         const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         const contract::peer::PeerRequestType,
         const bool&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        OTSecurityContract,
        SecurityContract,
        (const Nym_p&,
         const UnallocatedCString&,
         const UnallocatedCString&,
         const UnitType,
         const VersionNumber,
         const opentxs::PasswordPrompt&,
         const display::Definition&,
         const Amount&),
        (const, final));

    MOCK_METHOD(OTServerContract, ServerContract, (), (const, final));

    MOCK_METHOD(OTNotaryID, ServerID, (), (const, final));

    MOCK_METHOD(
        OTNotaryID,
        ServerID,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(OTNotaryID, ServerID, (const opentxs::String&), (const, final));

    MOCK_METHOD(
        OTNotaryID,
        ServerID,
        (const opentxs::network::zeromq::Frame&),
        (const, final));

    MOCK_METHOD(
        OTStoreSecret,
        StoreSecret,
        (const Nym_p&,
         const identifier::Nym&,
         const contract::peer::SecretType,
         const UnallocatedCString&,
         const UnallocatedCString&,
         const identifier::Notary&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(OTSymmetricKey, SymmetricKey, (), (const, final));

    MOCK_METHOD(
        OTSymmetricKey,
        SymmetricKeyHelper,
        (const opentxs::crypto::SymmetricProvider&,
         const opentxs::PasswordPrompt&,
         const opentxs::crypto::key::symmetric::Algorithm),
        (const));

    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::Error) const
        -> OTSymmetricKey final
    {
        return SymmetricKeyHelper(engine, password, mode);
    }

    MOCK_METHOD(
        OTSymmetricKey,
        SymmetricKey,
        (const opentxs::crypto::SymmetricProvider&,
         const opentxs::Secret&,
         const std::uint64_t,
         const std::uint64_t,
         const std::size_t,
         const opentxs::crypto::key::symmetric::Source),
        (const, final));

    MOCK_METHOD(
        OTSymmetricKey,
        SymmetricKey,
        (const opentxs::crypto::SymmetricProvider&,
         const opentxs::Secret&,
         const ReadView,
         const std::uint64_t,
         const std::uint64_t,
         const std::uint64_t,
         const std::size_t,
         const opentxs::crypto::key::symmetric::Source),
        (const, final));

    MOCK_METHOD(
        OTSymmetricKey,
        SymmetricKey,
        (const opentxs::crypto::SymmetricProvider&,
         const opentxs::Secret&,
         const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(OTUnitID, UnitID, (), (const, final));

    MOCK_METHOD(OTUnitID, UnitID, (const UnallocatedCString&), (const, final));

    MOCK_METHOD(OTUnitID, UnitID, (const opentxs::String&), (const, final));

    MOCK_METHOD(
        OTUnitID,
        UnitID,
        (const opentxs::network::zeromq::Frame&),
        (const, final));

    MOCK_METHOD(OTUnitDefinition, UnitDefinition, (), (const, noexcept, final));

    MOCK_METHOD(
        OTSecret,
        Secret,
        (const std::size_t),
        (const, noexcept, final));

    MOCK_METHOD(
        OTSecret,
        SecretFromBytes,
        (const ReadView),
        (const, noexcept, final));

    MOCK_METHOD(
        OTSecret,
        SecretFromText,
        (const std::string_view),
        (const, noexcept, final));

    MOCK_METHOD(
        OTArmored,
        Armored,
        (const google::protobuf::MessageLite&),
        (const, final));

    MOCK_METHOD(
        OTString,
        Armored,
        (const google::protobuf::MessageLite&, const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(const api::crypto::Asymmetric&, Asymmetric, (), (const, final));

    MOCK_METHOD(
        OTAsymmetricKey,
        AsymmetricKey,
        (const proto::AsymmetricKey&),
        (const, noexcept, final));

    MOCK_METHOD(
        OTBailmentNotice,
        BailmentNotice,
        (const Nym_p&, const proto::PeerRequest&),
        (const, final));

    MOCK_METHOD(
        OTBailmentReply,
        BailmentReply,
        (const Nym_p&, const proto::PeerReply&),
        (const, final));

    MOCK_METHOD(
        OTBailmentRequest,
        BailmentRequest,
        (const Nym_p&, const proto::PeerRequest&),
        (const, final));

    MOCK_METHOD(std::unique_ptr<opentxs::Basket>, Basket, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Basket>,
        Basket,
        (std::int32_t, const Amount&),
        (const, final));

    MOCK_METHOD(
        OTBasketContract,
        BasketContract,
        (const Nym_p&, const proto::UnitDefinition),
        (const, final));

#if OT_BLOCKCHAIN

    MOCK_METHOD(
        BlockHeaderP,
        BlockHeader,
        (const proto::BlockchainBlockHeader&),
        (const, final));

#endif  // OT_BLOCKCHAIN

    MOCK_METHOD(
        std::unique_ptr<opentxs::Cheque>,
        Cheque,
        (const OTTransaction&),
        (const, final));

    MOCK_METHOD(std::unique_ptr<opentxs::Cheque>, Cheque, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Cheque>,
        Cheque,
        (const identifier::Notary&, const identifier::UnitDefinition&),
        (const, final));

    MOCK_METHOD(
        OTConnectionReply,
        ConnectionReply,
        (const Nym_p&, const proto::PeerReply&),
        (const, final));

    MOCK_METHOD(
        OTConnectionRequest,
        ConnectionRequest,
        (const Nym_p&, const proto::PeerRequest&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Contract>,
        Contract,
        (const String&),
        (const, final));

    MOCK_METHOD(std::unique_ptr<OTCron>, Cron, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTCronItem>,
        CronItem,
        (const String&),
        (const, final));

    MOCK_METHOD(
        OTCurrencyContract,
        CurrencyContract,
        (const Nym_p&, const proto::UnitDefinition),
        (const, final));

    MOCK_METHOD(
        OTData,
        Data,
        (const google::protobuf::MessageLite&),
        (const, final));

    MOCK_METHOD(
        OTIdentifier,
        Identifier,
        (const google::protobuf::MessageLite&),
        (const, final));

    MOCK_METHOD(
        OTIdentifier,
        Identifier,
        (const proto::Identifier&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Item>,
        Item,
        (const String&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Item>,
        Item,
        (const UnallocatedCString&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Item>,
        Item,
        (const identifier::Nym&, const opentxs::Item&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Item>,
        Item,
        (const identifier::Nym&, const OTTransaction&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Item>,
        Item,
        (const identifier::Nym&,
         const OTTransaction&,
         itemType theType,
         const opentxs::Identifier&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Item>,
        Item,
        (const String&, const identifier::Notary&, std::int64_t),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Item>,
        Item,
        (const OTTransaction&, itemType, const opentxs::Identifier&),
        (const, final));

    MOCK_METHOD(
        OTKeypair,
        Keypair,
        (const proto::AsymmetricKey&, const proto::AsymmetricKey&),
        (const, final));

    MOCK_METHOD(
        OTKeypair,
        Keypair,
        (const proto::AsymmetricKey&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Ledger>,
        Ledger,
        (const opentxs::Identifier&, const identifier::Notary&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Ledger>,
        Ledger,
        (const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::Ledger>,
        LedgerHelper,
        (const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         ledgerType,
         bool),
        (const));

    auto Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const
        -> std::unique_ptr<opentxs::Ledger> final
    {
        return LedgerHelper(
            theNymID, theAcctID, theNotaryID, theType, bCreateFile);
    }

    MOCK_METHOD(std::unique_ptr<OTMarket>, Market, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTMarket>,
        Market,
        (const char*),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTMarket>,
        Market,
        (const identifier::Notary&,
         const identifier::UnitDefinition&,
         const identifier::UnitDefinition&,
         const Amount&),
        (const, final));

    MOCK_METHOD(std::unique_ptr<opentxs::Message>, Message, (), (const, final));

    MOCK_METHOD(
        OTOutbailmentReply,
        OutbailmentReply,
        (const Nym_p&, const proto::PeerReply&),
        (const, final));

    MOCK_METHOD(
        OTOutbailmentRequest,
        OutbailmentRequest,
        (const Nym_p&, const proto::PeerRequest&),
        (const, final));

    MOCK_METHOD(
        OTNymID,
        NymID,
        (const opentxs::Identifier&),
        (const, noexcept, final));

    MOCK_METHOD(
        OTNymID,
        NymID,
        (const proto::Identifier&),
        (const, noexcept, final));

    MOCK_METHOD(std::unique_ptr<OTOffer>, Offer, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTOffer>,
        Offer,
        (const identifier::Notary&,
         const identifier::UnitDefinition&,
         const identifier::UnitDefinition&,
         const Amount&),
        (const, final));

    MOCK_METHOD(std::unique_ptr<OTPayment>, Payment, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTPayment>,
        Payment,
        (const String&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTPayment>,
        Payment,
        (const opentxs::Contract&, const opentxs::PasswordPrompt&),
        (const, final));

    MOCK_METHOD(
        opentxs::PaymentCode,
        PaymentCode,
        (const proto::PaymentCode&),
        (const, noexcept, final));

    MOCK_METHOD(
        std::unique_ptr<OTPaymentPlan>,
        PaymentPlan,
        (),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTPaymentPlan>,
        PaymentPlan,
        (const identifier::Notary&, const identifier::UnitDefinition&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTPaymentPlan>,
        PaymentPlan,
        (const identifier::Notary&,
         const identifier::UnitDefinition&,
         const opentxs::Identifier&,
         const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Nym&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<opentxs::PeerObject>,
        PeerObject,
        (const Nym_p&, const proto::PeerObject&),
        (const, final));

    MOCK_METHOD(
        OTPeerReply,
        PeerReply,
        (const Nym_p&, const proto::PeerReply&),
        (const, final));

    MOCK_METHOD(
        OTPeerRequest,
        PeerRequest,
        (const Nym_p&, const proto::PeerRequest&),
        (const, final));

    MOCK_METHOD(
        otx::blind::Purse,
        Purse,
        (const proto::Purse&),
        (const, noexcept, final));

    MOCK_METHOD(
        OTReplyAcknowledgement,
        ReplyAcknowledgement,
        (const Nym_p&, const proto::PeerReply&),
        (const, final));

    MOCK_METHOD(
        OTSecurityContract,
        SecurityContract,
        (const Nym_p&, const proto::UnitDefinition),
        (const, final));

    MOCK_METHOD(
        OTNotaryID,
        ServerID,
        (const opentxs::Identifier&),
        (const, noexcept, final));

    MOCK_METHOD(
        OTNotaryID,
        ServerID,
        (const proto::Identifier&),
        (const, noexcept, final));

    MOCK_METHOD(
        OTIdentifier,
        ServerID,
        (const google::protobuf::MessageLite&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTScriptable>,
        Scriptable,
        (const String&),
        (const, final));

    MOCK_METHOD(std::unique_ptr<OTSignedFile>, SignedFile, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTSignedFile>,
        SignedFile,
        (const String&, const String&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTSignedFile>,
        SignedFile,
        (const char*, const String&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTSignedFile>,
        SignedFile,
        (const char*, const char*),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTSmartContract>,
        SmartContract,
        (),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTSmartContract>,
        SmartContract,
        (const identifier::Notary&),
        (const, final));

    MOCK_METHOD(
        OTStoreSecret,
        StoreSecret,
        (const Nym_p&, const proto::PeerRequest&),
        (const, final));

    MOCK_METHOD(const api::crypto::Symmetric&, Symmetric, (), (const, final));

    MOCK_METHOD(
        OTSymmetricKey,
        SymmetricKey,
        (const opentxs::crypto::SymmetricProvider&, const proto::SymmetricKey),
        (const, final));

    MOCK_METHOD(std::unique_ptr<OTTrade>, Trade, (), (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTTrade>,
        Trade,
        (const identifier::Notary&,
         const identifier::UnitDefinition&,
         const opentxs::Identifier&,
         const identifier::Nym&,
         const identifier::UnitDefinition&,
         const opentxs::Identifier&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTTransactionType>,
        Transaction,
        (const String&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTTransaction>,
        Transaction,
        (const opentxs::Ledger&),
        (const, final));

    MOCK_METHOD(
        std::unique_ptr<OTTransaction>,
        TransactionHelper,
        (const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         originType),
        (const));

    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final
    {
        return Transaction(theNymID, theAccountID, theNotaryID, theOriginType);
    }

    MOCK_METHOD(
        std::unique_ptr<OTTransaction>,
        TransactionHelper,
        (const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         std::int64_t,
         originType),
        (const));

    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final
    {
        return TransactionHelper(
            theNymID,
            theAccountID,
            theNotaryID,
            lTransactionNum,
            theOriginType);
    }

    //    MOCK_METHOD(
    //        std::unique_ptr<OTTransaction>,
    //        TransactionHelper,
    //        (const identifier::Nym&, const opentxs::Identifier&, const
    //        identifier::Notary&, const std::int64_t&, originType, const
    //        std::int64_t&, const std::int64_t&, const std::int64_t&,const
    //        Time, transactionType, const String&, const Amount&, const
    //        Amount&, const std::int64_t&, const std::int64_t&, bool,
    //        NumList*), (const));

    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        originType theOriginType,
        const std::int64_t& lTransactionNum,
        const std::int64_t& lInRefTo,
        const std::int64_t& lInRefDisplay,
        const Time the_DATE_SIGNED,
        transactionType theType,
        const String& strHash,
        const Amount& lAdjustment,
        const Amount& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr) const
        -> std::unique_ptr<OTTransaction> final
    {
        return nullptr;  // TransactionHelper(theNymID, theAccountID,
                         // theNotaryID, lNumberOfOrigin, theOriginType,
                         // lTransactionNum, lInRefTo, lInRefDisplay,
                         // the_DATE_SIGNED, theType, strHash, lAdjustment,
                         // lDisplayValue, lClosingNum, lRequestNum,
                         // bReplyTransSuccess, pNumList);
    }

    MOCK_METHOD(
        std::unique_ptr<OTTransaction>,
        TransactionHelper,
        (const identifier::Nym&,
         const opentxs::Identifier&,
         const identifier::Notary&,
         transactionType,
         originType,
         std::int64_t),
        (const));

    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> final
    {
        return TransactionHelper(
            theNymID,
            theAccountID,
            theNotaryID,
            theType,
            theOriginType,
            lTransactionNum);
    }

    MOCK_METHOD(
        std::unique_ptr<OTTransaction>,
        TransactionHelper,
        (const opentxs::Ledger&,
         transactionType theType,
         originType,
         std::int64_t),
        (const));

    auto Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> final
    {
        return TransactionHelper(
            theOwner, theType, theOriginType, lTransactionNum);
    }

    MOCK_METHOD(
        OTUnitDefinition,
        UnitDefinition,
        (const Nym_p&, const proto::UnitDefinition),
        (const, final));

    MOCK_METHOD(
        OTUnitID,
        UnitID,
        (const opentxs::Identifier&),
        (const, noexcept, final));

    MOCK_METHOD(
        OTUnitID,
        UnitID,
        (const proto::Identifier&),
        (const, noexcept, final));

    MOCK_METHOD(
        OTIdentifier,
        UnitID,
        (const google::protobuf::MessageLite&),
        (const, final));
};

}  // namespace opentxs