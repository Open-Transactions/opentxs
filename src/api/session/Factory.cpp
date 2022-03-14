// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "api/session/Factory.hpp"  // IWYU pragma: associated

#include <array>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "Proto.tpp"
#include "internal/api/Crypto.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/crypto/Asymmetric.hpp"
#include "internal/api/crypto/Factory.hpp"
#if OT_BLOCKCHAIN
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/core/Factory.hpp"
#include "internal/core/contract/peer/Factory.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/core/identifier/Factory.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Envelope.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"  // TODO remove
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/bitcoin/Opcodes.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/basket/Basket.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/crypto/OTSignedFile.hpp"
#include "internal/otx/common/recurring/OTPaymentPlan.hpp"
#include "internal/otx/common/script/OTScriptable.hpp"
#include "internal/otx/common/trade/OTMarket.hpp"
#include "internal/otx/common/trade/OTOffer.hpp"
#include "internal/otx/common/trade/OTTrade.hpp"
#include "opentxs/core/PaymentCode.hpp"
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
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/network/p2p/Base.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/AsymmetricKey.pb.h"
#include "serialization/protobuf/BlockchainPeerAddress.pb.h"  // IWYU pragma: keep
#include "serialization/protobuf/Ciphertext.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Envelope.pb.h"  // IWYU pragma: keep
#include "serialization/protobuf/HDPath.pb.h"
#include "serialization/protobuf/PaymentCode.pb.h"
#include "serialization/protobuf/PeerReply.pb.h"
#include "serialization/protobuf/PeerRequest.pb.h"
#include "serialization/protobuf/UnitDefinition.pb.h"
#include "util/HDIndex.hpp"

namespace opentxs::api::session::imp
{
Factory::Factory(const api::Session& api)
    : api::internal::Factory()
    , api_(api)
    , primitives_(opentxs::Context().Factory())  // TODO pass in as argument
    , pAsymmetric_(factory::AsymmetricAPI(api_))
    , asymmetric_(*pAsymmetric_)
    , pSymmetric_(factory::Symmetric(api_))
    , symmetric_(*pSymmetric_)
{
    OT_ASSERT(pAsymmetric_);
    OT_ASSERT(pSymmetric_);
}

auto Factory::Armored() const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored()};
}

auto Factory::Armored(const UnallocatedCString& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(String::Factory(input.c_str()))};
}

auto Factory::Armored(const opentxs::Data& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

auto Factory::Armored(const opentxs::String& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

auto Factory::Armored(const opentxs::crypto::Envelope& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(input)};
}

auto Factory::Armored(const ProtobufType& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(Data(input))};
}

auto Factory::Armored(
    const ProtobufType& input,
    const UnallocatedCString& header) const -> OTString
{
    auto armored = Armored(Data(input));
    auto output = String::Factory();
    armored->WriteArmoredString(output, header);

    return output;
}

auto Factory::AsymmetricKey(
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason,
    const opentxs::crypto::key::asymmetric::Role role,
    const VersionNumber version) const -> OTAsymmetricKey
{
    auto output = asymmetric_.NewKey(params, role, version, reason).release();

    if (output) {
        return OTAsymmetricKey{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create asymmetric key");
    }
}

auto Factory::AsymmetricKey(const proto::AsymmetricKey& serialized) const
    -> OTAsymmetricKey
{
    auto output = asymmetric_.Internal().InstantiateKey(serialized).release();

    if (output) {
        return OTAsymmetricKey{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate asymmetric key");
    }
}

auto Factory::BailmentNotice(
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const opentxs::Identifier& requestID,
    const UnallocatedCString& txid,
    const Amount& amount,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentNotice
{
    auto output = opentxs::Factory::BailmentNotice(
        api_,
        nym,
        recipientID,
        unitID,
        serverID,
        requestID,
        txid,
        amount,
        reason);

    if (output) {
        return OTBailmentNotice{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment notice");
    }
}

auto Factory::BailmentNotice(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTBailmentNotice
{
    auto output = opentxs::Factory::BailmentNotice(api_, nym, serialized);

    if (output) {
        return OTBailmentNotice{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment notice");
    }
}

auto Factory::BailmentReply(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Notary& server,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentReply
{
    auto output = opentxs::Factory::BailmentReply(
        api_, nym, initiator, request, server, terms, reason);

    if (output) {
        return OTBailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::BailmentReply(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false) -> OTBailmentReply
{
    auto output = opentxs::Factory::BailmentReply(api_, nym, serialized);

    if (output) {
        return OTBailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment reply");
    }
}

auto Factory::BailmentRequest(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTBailmentRequest
{
    auto output = opentxs::Factory::BailmentRequest(
        api_, nym, recipient, unit, server, reason);

    if (output) {
        return OTBailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::BailmentRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTBailmentRequest
{
    auto output = opentxs::Factory::BailmentRequest(api_, nym, serialized);

    if (output) {
        return OTBailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

auto Factory::BailmentRequest(const Nym_p& nym, const ReadView& view) const
    noexcept(false) -> OTBailmentRequest
{
    return BailmentRequest(nym, proto::Factory<proto::PeerRequest>(view));
}

auto Factory::Basket() const -> std::unique_ptr<opentxs::Basket>
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_));

    return basket;
}

auto Factory::Basket(std::int32_t nCount, const Amount& lMinimumTransferAmount)
    const -> std::unique_ptr<opentxs::Basket>
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_, nCount, lMinimumTransferAmount));

    return basket;
}

auto Factory::BasketContract(
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const std::uint64_t weight,
    const UnitType unitOfAccount,
    const VersionNumber version,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement) const noexcept(false) -> OTBasketContract
{
    auto output = opentxs::Factory::BasketContract(
        api_,
        nym,
        shortname,
        terms,
        weight,
        unitOfAccount,
        version,
        displayDefinition,
        redemptionIncrement);

    if (output) {
        return OTBasketContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::BasketContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTBasketContract
{
    auto output = opentxs::Factory::BasketContract(api_, nym, serialized);

    if (output) {
        return OTBasketContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

#if OT_BLOCKCHAIN
auto Factory::BitcoinScriptNullData(
    const opentxs::blockchain::Type chain,
    const UnallocatedVector<ReadView>& data) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::RETURN));

    for (const auto& element : data) {
        elements.emplace_back(bb::internal::PushData(element));
    }

    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto Factory::BitcoinScriptP2MS(
    const opentxs::blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    const UnallocatedVector<const opentxs::crypto::key::EllipticCurve*>&
        publicKeys) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    if ((0u == M) || (16u < M)) {
        LogError()(OT_PRETTY_CLASS())("Invalid M").Flush();

        return {};
    }

    if ((0u == N) || (16u < N)) {
        LogError()(OT_PRETTY_CLASS())("Invalid N").Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(static_cast<bb::OP>(M + 80)));

    for (const auto& pKey : publicKeys) {
        if (nullptr == pKey) {
            LogError()(OT_PRETTY_CLASS())("Invalid key").Flush();

            return {};
        }

        const auto& key = *pKey;
        elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    }

    elements.emplace_back(bb::internal::Opcode(static_cast<bb::OP>(N + 80)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKMULTISIG));
    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto Factory::BitcoinScriptP2PK(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::key::EllipticCurve& key) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::PushData(key.PublicKey()));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKSIG));
    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto Factory::BitcoinScriptP2PKH(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::key::EllipticCurve& key) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto hash = Space{};

    if (false == b::PubkeyHash(api_, chain, key.PublicKey(), writer(hash))) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate pubkey hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::DUP));
    elements.emplace_back(bb::internal::Opcode(bb::OP::HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::EQUALVERIFY));
    elements.emplace_back(bb::internal::Opcode(bb::OP::CHECKSIG));
    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto Factory::BitcoinScriptP2SH(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::bitcoin::Script& script) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto bytes = Space{};
    auto hash = Space{};

    if (false == script.Serialize(writer(bytes))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize script").Flush();

        return {};
    }

    if (false == b::ScriptHash(api_, chain, reader(bytes), writer(hash))) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate script hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::HASH160));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    elements.emplace_back(bb::internal::Opcode(bb::OP::EQUAL));
    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto Factory::BitcoinScriptP2WPKH(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::key::EllipticCurve& key) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto hash = Space{};

    if (false == b::PubkeyHash(api_, chain, key.PublicKey(), writer(hash))) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate pubkey hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::ZERO));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto Factory::BitcoinScriptP2WSH(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::bitcoin::Script& script) const noexcept
    -> std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>
{
    namespace b = opentxs::blockchain;
    namespace bb = opentxs::blockchain::block::bitcoin;

    auto bytes = Space{};
    auto hash = Space{};

    if (false == script.Serialize(writer(bytes))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize script").Flush();

        return {};
    }

    if (false ==
        b::ScriptHashSegwit(api_, chain, reader(bytes), writer(hash))) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate script hash")
            .Flush();

        return {};
    }

    auto elements = bb::ScriptElements{};
    elements.emplace_back(bb::internal::Opcode(bb::OP::ZERO));
    elements.emplace_back(bb::internal::PushData(reader(hash)));
    using Position = opentxs::blockchain::block::bitcoin::Script::Position;

    return factory::BitcoinScript(chain, std::move(elements), Position::Output);
}

auto Factory::BlockchainAddress(
    const opentxs::blockchain::p2p::Protocol protocol,
    const opentxs::blockchain::p2p::Network network,
    const opentxs::Data& bytes,
    const std::uint16_t port,
    const opentxs::blockchain::Type chain,
    const Time lastConnected,
    const UnallocatedSet<opentxs::blockchain::p2p::Service>& services,
    const bool incoming) const -> OTBlockchainAddress
{
    return OTBlockchainAddress{factory::BlockchainAddress(
                                   api_,
                                   protocol,
                                   network,
                                   bytes,
                                   port,
                                   chain,
                                   lastConnected,
                                   services,
                                   incoming)
                                   .release()};
}

auto Factory::BlockchainAddress(
    const opentxs::blockchain::p2p::Address::SerializedType& serialized) const
    -> OTBlockchainAddress
{
    return OTBlockchainAddress{
        factory::BlockchainAddress(api_, serialized).release()};
}

auto Factory::BlockchainSyncMessage(const opentxs::network::zeromq::Message& in)
    const noexcept -> std::unique_ptr<opentxs::network::p2p::Base>
{
    return factory::BlockchainSyncMessage(api_, in);
}
#endif  // OT_BLOCKCHAIN

auto Factory::Cheque(const OTTransaction& receipt) const
    -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> output{new opentxs::Cheque{api_}};

    OT_ASSERT(output)

    auto serializedItem = String::Factory();
    receipt.GetReferenceString(serializedItem);
    std::unique_ptr<opentxs::Item> item{Item(
        serializedItem,
        receipt.GetRealNotaryID(),
        receipt.GetReferenceToNum())};

    OT_ASSERT(false != bool(item));

    auto serializedCheque = String::Factory();
    item->GetAttachment(serializedCheque);
    const auto loaded = output->LoadContractFromString(serializedCheque);

    if (false == loaded) {
        LogError()(OT_PRETTY_CLASS())("Failed to load cheque.").Flush();
    }

    return output;
}

auto Factory::Cheque() const -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(new opentxs::Cheque(api_));

    return cheque;
}

auto Factory::Cheque(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
    -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(
        new opentxs::Cheque(api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return cheque;
}

auto Factory::ConnectionReply(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Notary& server,
    const bool ack,
    const UnallocatedCString& url,
    const UnallocatedCString& login,
    const UnallocatedCString& password,
    const UnallocatedCString& key,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTConnectionReply
{
    auto output = opentxs::Factory::ConnectionReply(
        api_,
        nym,
        initiator,
        request,
        server,
        ack,
        url,
        login,
        password,
        key,
        reason);

    if (output) {
        return OTConnectionReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create connection reply");
    }
}

auto Factory::ConnectionReply(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false)
    -> OTConnectionReply
{
    auto output = opentxs::Factory::ConnectionReply(api_, nym, serialized);

    if (output) {
        return OTConnectionReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate connection reply");
    }
}

auto Factory::ConnectionRequest(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const contract::peer::ConnectionInfoType type,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTConnectionRequest
{
    auto output = opentxs::Factory::ConnectionRequest(
        api_, nym, recipient, type, server, reason);

    if (output) {
        return OTConnectionRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::ConnectionRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTConnectionRequest
{
    auto output = opentxs::Factory::ConnectionRequest(api_, nym, serialized);

    if (output) {
        return OTConnectionRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment reply");
    }
}

auto Factory::Contract(const opentxs::String& strInput) const
    -> std::unique_ptr<opentxs::Contract>
{

    using namespace opentxs;
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed = DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {

        std::unique_ptr<opentxs::Contract> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                          // chars long.
        {
            pContract.reset(new OTSmartContract(api_));
            OT_ASSERT(false != bool(pContract));
        }

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED PAYMENT PLAN-----"))  // this string is 35
                                                         // chars long.
        {
            pContract.reset(new OTPaymentPlan(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRADE-----"))  // this string is 28
                                                         // chars long.
        {
            pContract.reset(new OTTrade(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED OFFER-----")) {
            pContract.reset(new OTOffer(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED INVOICE-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract.reset(new opentxs::Message(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MINT-----")) {
            auto mint = Mint();
            pContract.reset(mint.Release());
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED FILE-----")) {
            OT_ASSERT(false != bool(pContract));
        }

        // The string didn't match any of the options in the factory.
        //
        if (!pContract) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Object type not yet supported by class "
                "factory: ")(strFirstLine)
                .Flush();
            // Does the contract successfully load from the string passed in?
        } else if (!pContract->LoadContractFromString(strContract)) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed loading contract from string (first "
                "line): ")(strFirstLine)
                .Flush();
        } else {
            return pContract;
        }
    }
    return nullptr;
}

auto Factory::Cron() const -> std::unique_ptr<OTCron> { return {}; }

auto Factory::CronItem(const String& strCronItem) const
    -> std::unique_ptr<OTCronItem>
{
    std::array<char, 45> buf{};

    if (!strCronItem.Exists()) {
        LogError()(OT_PRETTY_CLASS())(
            "Empty string was passed in (returning nullptr).")
            .Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strCronItem.Get());

    if (!strContract->DecodeIfArmored(false)) {
        LogError()(OT_PRETTY_CLASS())(
            "Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strCronItem)(".")
            .Flush();
        return nullptr;
    }

    strContract->reset();  // for sgets
    bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) return nullptr;

    auto strFirstLine = String::Factory(buf.data());
    // set the "file" pointer within this string back to index 0.
    strContract->reset();

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -"))
        return nullptr;

    // By this point we know already that it's not escaped.
    // BUT it might still be ARMORED!

    std::unique_ptr<OTCronItem> pItem;
    // this string is 35 chars long.
    if (strFirstLine->Contains("-----BEGIN SIGNED PAYMENT PLAN-----")) {
        pItem.reset(new OTPaymentPlan(api_));
    }
    // this string is 28 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED TRADE-----")) {
        pItem.reset(new OTTrade(api_));
    }
    // this string is 36 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED SMARTCONTRACT-----")) {
        pItem.reset(new OTSmartContract(api_));
    } else {
        return nullptr;
    }

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) { return pItem; }

    return nullptr;
}

auto Factory::CurrencyContract(
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement) const noexcept(false)
    -> OTCurrencyContract
{
    auto output = opentxs::Factory::CurrencyContract(
        api_,
        nym,
        shortname,
        terms,
        unitOfAccount,
        version,
        reason,
        displayDefinition,
        redemptionIncrement);

    if (output) {
        return OTCurrencyContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create currency contract");
    }
}

auto Factory::CurrencyContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTCurrencyContract
{
    auto output = opentxs::Factory::CurrencyContract(api_, nym, serialized);

    if (output) {
        return OTCurrencyContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate currency contract");
    }
}

auto Factory::Data() const -> OTData { return Data::Factory(); }

auto Factory::Data(const opentxs::Armored& input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const ProtobufType& input) const -> OTData
{
    auto output = Data::Factory();
    const auto size{input.ByteSize()};
    output->SetSize(size);
    input.SerializeToArray(output->data(), size);

    return output;
}

auto Factory::Data(const opentxs::network::zeromq::Frame& input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const std::uint8_t input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const std::uint32_t input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const UnallocatedCString& input, const StringStyle mode)
    const -> OTData
{
    return Data::Factory(input, static_cast<Data::Mode>(mode));
}

auto Factory::Data(const UnallocatedVector<unsigned char>& input) const
    -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(const UnallocatedVector<std::byte>& input) const -> OTData
{
    return Data::Factory(input);
}

auto Factory::Data(ReadView input) const -> OTData
{
    return Data::Factory(input.data(), input.size());
}

auto Factory::Envelope() const noexcept -> OTEnvelope
{
    return OTEnvelope{opentxs::Factory::Envelope(api_).release()};
}

auto Factory::Envelope(const opentxs::Armored& in) const noexcept(false)
    -> OTEnvelope
{
    auto data = Data();

    if (false == in.GetData(data)) {
        throw std::runtime_error("Invalid armored envelope");
    }

    return Envelope(
        proto::Factory<opentxs::crypto::Envelope::SerializedType>(data));
}

auto Factory::Envelope(
    const opentxs::crypto::Envelope::SerializedType& serialized) const
    noexcept(false) -> OTEnvelope
{
    if (false == proto::Validate(serialized, VERBOSE)) {
        throw std::runtime_error("Invalid serialized envelope");
    }

    return OTEnvelope{opentxs::Factory::Envelope(api_, serialized).release()};
}

auto Factory::Envelope(const opentxs::ReadView& serialized) const
    noexcept(false) -> OTEnvelope
{
    return OTEnvelope{opentxs::Factory::Envelope(api_, serialized).release()};
}

auto Factory::Identifier() const -> OTIdentifier
{
    return Identifier::Factory();
}

auto Factory::Identifier(const UnallocatedCString& serialized) const
    -> OTIdentifier
{
    return Identifier::Factory(serialized);
}

auto Factory::Identifier(const opentxs::String& serialized) const
    -> OTIdentifier
{
    return Identifier::Factory(serialized);
}

auto Factory::Identifier(const opentxs::Contract& contract) const
    -> OTIdentifier
{
    return Identifier::Factory(contract);
}

auto Factory::Identifier(const opentxs::Item& item) const -> OTIdentifier
{
    return Identifier::Factory(item);
}

auto Factory::Identifier(const ReadView bytes) const -> OTIdentifier
{
    auto output = this->Identifier();
    output->CalculateDigest(bytes);

    return output;
}

auto Factory::Identifier(const ProtobufType& proto) const -> OTIdentifier
{
    const auto bytes = Data(proto);

    return Identifier(bytes->Bytes());
}

auto Factory::Identifier(const opentxs::network::zeromq::Frame& bytes) const
    -> OTIdentifier
{
    auto out = Identifier();
    out->Assign(bytes.data(), bytes.size());

    return out;
}

auto Factory::Identifier(const proto::Identifier& in) const noexcept
    -> OTIdentifier
{
    return factory::IdentifierGeneric(in);
}

auto Factory::instantiate_secp256k1(
    const ReadView key,
    const ReadView chaincode) const noexcept
    -> std::unique_ptr<opentxs::crypto::key::Secp256k1>
{
    using Type = opentxs::crypto::key::asymmetric::Algorithm;

    if (crypto::HaveHDKeys()) {
        static const auto blank = Secret(0);
        static const auto path = proto::HDPath{};

        return factory::Secp256k1Key(
            api_,
            api_.Crypto().Internal().EllipticProvider(Type::Secp256k1),
            blank,
            SecretFromBytes(chaincode),
            Data(key),
            path,
            {},
            opentxs::crypto::key::asymmetric::Role::Sign,
            opentxs::crypto::key::EllipticCurve::DefaultVersion);
    } else {
        using ReturnType = opentxs::crypto::key::Secp256k1;

        auto serialized = ReturnType::Serialized{};
        serialized.set_version(ReturnType::DefaultVersion);
        serialized.set_type(proto::AKEYTYPE_SECP256K1);
        serialized.set_mode(proto::KEYMODE_PUBLIC);
        serialized.set_role(proto::KEYROLE_SIGN);
        serialized.set_key(key.data(), key.size());
        auto output = factory::Secp256k1Key(
            api_,
            api_.Crypto().Internal().EllipticProvider(Type::Secp256k1),
            serialized);

        if (false == bool(output)) {
            output = std::make_unique<opentxs::crypto::key::blank::Secp256k1>();
        }

        OT_ASSERT(output);

        return output;
    }
}

auto Factory::Item(const UnallocatedCString& serialized) const
    -> std::unique_ptr<opentxs::Item>
{
    return Item(String::Factory(serialized));
}

auto Factory::Item(const String& serialized) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> output{new opentxs::Item(api_)};

    if (output) {
        const auto loaded = output->LoadContractFromString(serialized);

        if (false == loaded) {
            LogError()(OT_PRETTY_CLASS())("Unable to deserialize.").Flush();
            output.reset();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate.").Flush();
    }

    return output;
}

auto Factory::Item(
    const identifier::Nym& theNymID,
    const opentxs::Item& theOwner) const -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

auto Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner) const -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

auto Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner,
    itemType theType,
    const opentxs::Identifier& pDestinationAcctID) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(
        api_, theNymID, theOwner, theType, pDestinationAcctID));

    return item;
}

// Sometimes I don't know user ID of the originator, or the account ID of the
// originator,
// until after I have loaded the item. It's simply impossible to set those
// values ahead
// of time, sometimes. In those cases, we set the values appropriately but then
// we need
// to verify that the user ID is actually the owner of the AccountID. TOdo that.
auto Factory::Item(
    const String& strItem,
    const identifier::Notary& theNotaryID,
    std::int64_t lTransactionNumber) const -> std::unique_ptr<opentxs::Item>
{
    if (!strItem.Exists()) {
        LogError()(OT_PRETTY_CLASS())("strItem is empty. (Expected an "
                                      "item).")
            .Flush();
        return nullptr;
    }

    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(api_)};

    // So when it loads its own server ID, we can compare to this one.
    pItem->SetRealNotaryID(theNotaryID);

    // This loads up the purported account ID and the user ID.
    if (pItem->LoadContractFromString(strItem)) {
        const opentxs::Identifier& ACCOUNT_ID = pItem->GetPurportedAccountID();
        pItem->SetRealAccountID(ACCOUNT_ID);  // I do this because it's all
                                              // we've got in this case. It's
                                              // what's in the
        // xml, so it must be right. If it's a lie, the signature will fail or
        // the
        // user will not show as the owner of that account. But remember, the
        // server
        // sent the message in the first place.

        pItem->SetTransactionNum(lTransactionNumber);

        if (pItem->VerifyContractID())  // this compares purported and real
                                        // account IDs, as well as server IDs.
        {
            return pItem;
        }
    }

    return nullptr;
}

// Let's say you have created a transaction, and you are creating an item to put
// into it.
// Well in that case, you don't care to verify that the real IDs match the
// purported IDs, since
// you are creating this item yourself, not verifying it from someone else.
// Use this function to create the new Item before you add it to your new
// Transaction.
auto Factory::Item(
    const OTTransaction& theOwner,
    itemType theType,
    const opentxs::Identifier& pDestinationAcctID) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(
        api_, theOwner.GetNymID(), theOwner, theType, pDestinationAcctID)};

    if (false != bool(pItem)) {
        pItem->SetPurportedAccountID(theOwner.GetPurportedAccountID());
        pItem->SetPurportedNotaryID(theOwner.GetPurportedNotaryID());
        return pItem;
    }
    return nullptr;
}

auto Factory::Keypair(
    const opentxs::crypto::Parameters& params,
    const VersionNumber version,
    const opentxs::crypto::key::asymmetric::Role role,
    const opentxs::PasswordPrompt& reason) const -> OTKeypair
{
    auto pPrivateKey = asymmetric_.NewKey(params, role, version, reason);

    if (false == bool(pPrivateKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to derive private key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    auto& privateKey = *pPrivateKey;
    auto pPublicKey = privateKey.asPublic();

    if (false == bool(pPublicKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to derive public key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_, role, std::move(pPublicKey), std::move(pPrivateKey))};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto Factory::Keypair(
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair
{
    auto pPrivateKey = asymmetric_.Internal().InstantiateKey(serializedPrivkey);

    if (false == bool(pPrivateKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate private key")
            .Flush();

        return OTKeypair{factory::Keypair()};
    }

    auto pPublicKey = asymmetric_.Internal().InstantiateKey(serializedPubkey);

    if (false == bool(pPublicKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate public key")
            .Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_,
            translate(serializedPrivkey.role()),
            std::move(pPublicKey),
            std::move(pPrivateKey))};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto Factory::Keypair(const proto::AsymmetricKey& serializedPubkey) const
    -> OTKeypair
{
    auto pPublicKey = asymmetric_.Internal().InstantiateKey(serializedPubkey);

    if (false == bool(pPublicKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate public key")
            .Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_,
            translate(serializedPubkey.role()),
            std::move(pPublicKey),
            std::make_unique<opentxs::crypto::key::blank::Asymmetric>())};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto Factory::Keypair(
    const UnallocatedCString& fingerprint,
    const Bip32Index nym,
    const Bip32Index credset,
    const Bip32Index credindex,
    const EcdsaCurve& curve,
    const opentxs::crypto::key::asymmetric::Role role,
    const opentxs::PasswordPrompt& reason) const -> OTKeypair
{
    auto input(fingerprint);
    auto roleIndex = Bip32Index{0};

    switch (role) {
        case opentxs::crypto::key::asymmetric::Role::Auth: {
            roleIndex = HDIndex{Bip32Child::AUTH_KEY, Bip32Child::HARDENED};
        } break;
        case opentxs::crypto::key::asymmetric::Role::Encrypt: {
            roleIndex = HDIndex{Bip32Child::ENCRYPT_KEY, Bip32Child::HARDENED};
        } break;
        case opentxs::crypto::key::asymmetric::Role::Sign: {
            roleIndex = HDIndex{Bip32Child::SIGN_KEY, Bip32Child::HARDENED};
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid key role").Flush();

            return OTKeypair{factory::Keypair()};
        }
    }

    const auto path = UnallocatedVector<Bip32Index>{
        HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED},
        HDIndex{nym, Bip32Child::HARDENED},
        HDIndex{credset, Bip32Child::HARDENED},
        HDIndex{credindex, Bip32Child::HARDENED},
        roleIndex};
    auto pPrivateKey =
        api_.Crypto().Seed().GetHDKey(input, curve, path, role, reason);

    if (false == bool(pPrivateKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to derive private key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    auto& privateKey = *pPrivateKey;
    auto pPublicKey = privateKey.asPublic();

    if (false == bool(pPublicKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to derive public key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_, role, std::move(pPublicKey), std::move(pPrivateKey))};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto Factory::Ledger(
    const opentxs::Identifier& theAccountID,
    const identifier::Notary& theNotaryID) const
    -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theAccountID, theNotaryID));

    return ledger;
}

auto Factory::Ledger(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Notary& theNotaryID) const
    -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(
        new opentxs::Ledger(api_, theNymID, theAccountID, theNotaryID));

    return ledger;
}

auto Factory::Ledger(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAcctID,
    const identifier::Notary& theNotaryID,
    ledgerType theType,
    bool bCreateFile) const -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theNymID, theAcctID, theNotaryID));

    ledger->generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);

    return ledger;
}

auto Factory::Market() const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_));

    return market;
}

auto Factory::Market(const char* szFilename) const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_, szFilename));

    return market;
}

auto Factory::Market(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_TYPE_ID,
    const Amount& lScale) const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(
        api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_TYPE_ID, lScale));

    return market;
}

auto Factory::Message() const -> std::unique_ptr<opentxs::Message>
{
    std::unique_ptr<opentxs::Message> message;
    message.reset(new opentxs::Message(api_));

    return message;
}

auto Factory::Mint(const otx::blind::CashType type) const noexcept
    -> otx::blind::Mint
{
    switch (type) {
        case otx::blind::CashType::Lucre: {

            return factory::MintLucre(api_);
        }
        default: {
            LogError()(OT_PRETTY_CLASS())("unsupported cash type: ")(
                opentxs::print(type))
                .Flush();

            return otx::blind::Mint{api_};
        }
    }
}

auto Factory::Mint() const noexcept -> otx::blind::Mint
{
    return Mint(otx::blind::CashType::Lucre);
}

auto Factory::Mint(
    const otx::blind::CashType type,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    switch (type) {
        case otx::blind::CashType::Lucre: {

            return factory::MintLucre(api_, notary, unit);
        }
        default: {
            LogError()(OT_PRETTY_CLASS())("unsupported cash type: ")(
                opentxs::print(type))
                .Flush();

            return otx::blind::Mint{api_};
        }
    }
}

auto Factory::Mint(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return Mint(otx::blind::CashType::Lucre, notary, unit);
}

auto Factory::Mint(
    const otx::blind::CashType type,
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    switch (type) {
        case otx::blind::CashType::Lucre: {

            return factory::MintLucre(api_, notary, serverNym, unit);
        }
        default: {
            LogError()(OT_PRETTY_CLASS())("unsupported cash type: ")(
                opentxs::print(type))
                .Flush();

            return otx::blind::Mint{api_};
        }
    }
}

auto Factory::Mint(
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return Mint(otx::blind::CashType::Lucre, notary, serverNym, unit);
}

auto Factory::NymID() const -> OTNymID { return identifier::Nym::Factory(); }

auto Factory::NymID(const UnallocatedCString& serialized) const -> OTNymID
{
    return identifier::Nym::Factory(serialized);
}

auto Factory::NymID(const opentxs::String& serialized) const -> OTNymID
{
    return identifier::Nym::Factory(serialized);
}

auto Factory::NymID(const opentxs::network::zeromq::Frame& bytes) const
    -> OTNymID
{
    auto out = NymID();
    out->Assign(bytes.data(), bytes.size());

    return out;
}

auto Factory::NymID(const proto::Identifier& in) const noexcept -> OTNymID
{
    return factory::IdentifierNym(in);
}

auto Factory::NymID(const opentxs::Identifier& in) const noexcept -> OTNymID
{
    auto out = NymID();

    if ((identifier::Type::nym == in.Type() ||
         (identifier::Type::generic == in.Type()))) {
        out->Assign(in);
    }

    return out;
}

auto Factory::NymIDFromPaymentCode(const UnallocatedCString& input) const
    -> OTNymID
{
    const auto code = PaymentCode(input);

    if (0 == code.Version()) { return NymID(); }

    return code.ID();
}

auto Factory::Offer() const -> std::unique_ptr<OTOffer>
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(api_));

    return offer;
}

auto Factory::Offer(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_ID,
    const Amount& MARKET_SCALE) const -> std::unique_ptr<OTOffer>
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(
        api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_ID, MARKET_SCALE));

    return offer;
}

auto Factory::OutbailmentReply(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Notary& server,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTOutbailmentReply
{
    auto output = opentxs::Factory::OutBailmentReply(
        api_, nym, initiator, request, server, terms, reason);

    if (output) {
        return OTOutbailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create outbailment reply");
    }
}

auto Factory::OutbailmentReply(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false)
    -> OTOutbailmentReply
{
    auto output = opentxs::Factory::OutBailmentReply(api_, nym, serialized);

    if (output) {
        return OTOutbailmentReply{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate outbailment reply");
    }
}

auto Factory::OutbailmentRequest(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& server,
    const Amount& amount,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTOutbailmentRequest
{
    auto output = opentxs::Factory::OutbailmentRequest(
        api_, nym, recipient, unit, server, amount, terms, reason);

    if (output) {
        return OTOutbailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create outbailment reply");
    }
}

auto Factory::OutbailmentRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false)
    -> OTOutbailmentRequest
{
    auto output = opentxs::Factory::OutbailmentRequest(api_, nym, serialized);

    if (output) {
        return OTOutbailmentRequest{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate outbailment request");
    }
}

auto Factory::PasswordPrompt(const UnallocatedCString& text) const
    -> OTPasswordPrompt
{
    return OTPasswordPrompt{opentxs::Factory::PasswordPrompt(api_, text)};
}

auto Factory::Payment() const -> std::unique_ptr<OTPayment>
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_));

    return payment;
}

auto Factory::Payment(const String& strPayment) const
    -> std::unique_ptr<OTPayment>
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_, strPayment));

    return payment;
}

auto Factory::Payment(
    const opentxs::Contract& contract,
    const opentxs::PasswordPrompt& reason) const -> std::unique_ptr<OTPayment>
{
    auto payment = Factory::Payment(String::Factory(contract));

    if (payment) { payment->SetTempValues(reason); }

    return payment;
}

auto Factory::PaymentCode(const UnallocatedCString& base58) const noexcept
    -> opentxs::PaymentCode
{
    return factory::PaymentCode(api_, base58);
}

auto Factory::PaymentCode(const proto::PaymentCode& serialized) const noexcept
    -> opentxs::PaymentCode
{
    return factory::PaymentCode(api_, serialized);
}

auto Factory::PaymentCode(const ReadView& serialized) const noexcept
    -> opentxs::PaymentCode
{
    return PaymentCode(proto::Factory<proto::PaymentCode>(serialized));
}

auto Factory::PaymentCode(
    const UnallocatedCString& seed,
    const Bip32Index nym,
    const std::uint8_t version,
    const opentxs::PasswordPrompt& reason,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const noexcept -> opentxs::PaymentCode
{
    return factory::PaymentCode(
        api_,
        seed,
        nym,
        version,
        bitmessage,
        bitmessageVersion,
        bitmessageStream,
        reason);
}

auto Factory::PaymentPlan() const -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(api_));

    return paymentplan;
}

auto Factory::PaymentPlan(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
    -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(
        new OTPaymentPlan(api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return paymentplan;
}

auto Factory::PaymentPlan(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const opentxs::Identifier& SENDER_ACCT_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const opentxs::Identifier& RECIPIENT_ACCT_ID,
    const identifier::Nym& RECIPIENT_NYM_ID) const
    -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(
        api_,
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID));

    return paymentplan;
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const UnallocatedCString& message) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()(OT_PRETTY_CLASS())(
        "Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const UnallocatedCString& payment,
    [[maybe_unused]] const bool isPayment) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()(OT_PRETTY_CLASS())(
        "Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] otx::blind::Purse&& purse) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()(OT_PRETTY_CLASS())(
        "Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const OTPeerRequest request,
    [[maybe_unused]] const OTPeerReply reply,
    [[maybe_unused]] const VersionNumber version) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()(OT_PRETTY_CLASS())(
        "Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const OTPeerRequest request,
    [[maybe_unused]] const VersionNumber version) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()(OT_PRETTY_CLASS())(
        "Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& signerNym,
    [[maybe_unused]] const proto::PeerObject& serialized) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()(OT_PRETTY_CLASS())(
        "Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerObject(
    [[maybe_unused]] const Nym_p& recipientNym,
    [[maybe_unused]] const opentxs::Armored& encrypted,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()(OT_PRETTY_CLASS())(
        "Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

auto Factory::PeerReply() const noexcept -> OTPeerReply
{
    return OTPeerReply{opentxs::factory::PeerReply(api_)};
}

auto Factory::PeerReply(const Nym_p& nym, const proto::PeerReply& serialized)
    const noexcept(false) -> OTPeerReply
{
    switch (translate(serialized.type())) {
        case contract::peer::PeerRequestType::Bailment: {
            return BailmentReply(nym, serialized).as<contract::peer::Reply>();
        }
        case contract::peer::PeerRequestType::ConnectionInfo: {
            return ConnectionReply(nym, serialized).as<contract::peer::Reply>();
        }
        case contract::peer::PeerRequestType::OutBailment: {
            return OutbailmentReply(nym, serialized)
                .as<contract::peer::Reply>();
        }
        case contract::peer::PeerRequestType::PendingBailment:
        case contract::peer::PeerRequestType::StoreSecret: {
            return ReplyAcknowledgement(nym, serialized)
                .as<contract::peer::Reply>();
        }
        case contract::peer::PeerRequestType::VerificationOffer:
        case contract::peer::PeerRequestType::Faucet:
        case contract::peer::PeerRequestType::Error:
        default: {
            throw std::runtime_error("Unsupported reply type");
        }
    }
}

auto Factory::PeerReply(const Nym_p& nym, const ReadView& view) const
    noexcept(false) -> OTPeerReply
{
    return PeerReply(nym, proto::Factory<proto::PeerReply>(view));
}

auto Factory::PeerRequest() const noexcept -> OTPeerRequest
{
    return OTPeerRequest{opentxs::factory::PeerRequest(api_)};
}

auto Factory::PeerRequest(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false) -> OTPeerRequest
{
    switch (translate(serialized.type())) {
        case contract::peer::PeerRequestType::Bailment: {
            return BailmentRequest(nym, serialized)
                .as<contract::peer::Request>();
        }
        case contract::peer::PeerRequestType::OutBailment: {
            return OutbailmentRequest(nym, serialized)
                .as<contract::peer::Request>();
        }
        case contract::peer::PeerRequestType::PendingBailment: {
            return BailmentNotice(nym, serialized)
                .as<contract::peer::Request>();
        }
        case contract::peer::PeerRequestType::ConnectionInfo: {
            return ConnectionRequest(nym, serialized)
                .as<contract::peer::Request>();
        }
        case contract::peer::PeerRequestType::StoreSecret: {
            return StoreSecret(nym, serialized).as<contract::peer::Request>();
        }
        case contract::peer::PeerRequestType::VerificationOffer:
        case contract::peer::PeerRequestType::Faucet:
        case contract::peer::PeerRequestType::Error:
        default: {
            throw std::runtime_error("Unsupported reply type");
        }
    }
}

auto Factory::PeerRequest(const Nym_p& nym, const ReadView& view) const
    noexcept(false) -> OTPeerRequest
{
    return PeerRequest(nym, proto::Factory<proto::PeerRequest>(view));
}

auto Factory::Purse(
    const otx::context::Server& context,
    const identifier::UnitDefinition& unit,
    const otx::blind::Mint& mint,
    const Amount& totalValue,
    const otx::blind::CashType type,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return factory::Purse(api_, context, type, mint, totalValue, reason);
}

auto Factory::Purse(
    const otx::context::Server& context,
    const identifier::UnitDefinition& unit,
    const otx::blind::Mint& mint,
    const Amount& totalValue,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return Purse(
        context, unit, mint, totalValue, otx::blind::CashType::Lucre, reason);
}

auto Factory::Purse(const proto::Purse& serialized) const noexcept
    -> otx::blind::Purse
{
    return factory::Purse(api_, serialized);
}

auto Factory::Purse(
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const otx::blind::CashType type,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return factory::Purse(api_, owner, server, unit, type, reason);
}

auto Factory::Purse(
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return Purse(owner, server, unit, otx::blind::CashType::Lucre, reason);
}

auto Factory::ReplyAcknowledgement(
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const opentxs::Identifier& request,
    const identifier::Notary& server,
    const contract::peer::PeerRequestType type,
    const bool& ack,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTReplyAcknowledgement
{
    auto output = opentxs::Factory::NoticeAcknowledgement(
        api_, nym, initiator, request, server, type, ack, reason);

    if (output) {
        return OTReplyAcknowledgement{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create peer acknowledgement");
    }
}

auto Factory::ReplyAcknowledgement(
    const Nym_p& nym,
    const proto::PeerReply& serialized) const noexcept(false)
    -> OTReplyAcknowledgement
{
    auto output =
        opentxs::Factory::NoticeAcknowledgement(api_, nym, serialized);

    if (output) {
        return OTReplyAcknowledgement{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate peer acknowledgement");
    }
}

auto Factory::Scriptable(const String& strInput) const
    -> std::unique_ptr<OTScriptable>
{
    std::array<char, 45> buf{};

    if (!strInput.Exists()) {
        LogError()(OT_PRETTY_CLASS())("Failure: Input string is empty.")
            .Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strInput.Get());

    if (!strContract->DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                               // by default.
    {
        LogError()(OT_PRETTY_CLASS())(
            "Input string apparently was encoded and then failed decoding. "
            "Contents: ")(strInput)
            .Flush();
        return nullptr;
    }

    // At this point, strContract contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    strContract->reset();  // for sgets
    bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) return nullptr;

    std::unique_ptr<OTScriptable> pItem;

    auto strFirstLine = String::Factory(buf.data());
    strContract->reset();  // set the "file" pointer within this string back to
                           // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -"))
        return nullptr;

    // There are actually two factories that load smart contracts. See
    // OTCronItem.
    //
    else if (strFirstLine->Contains(
                 "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                           // chars long.
    {
        pItem.reset(new OTSmartContract(api_));
        OT_ASSERT(false != bool(pItem));
    }

    // The string didn't match any of the options in the factory.
    if (false == bool(pItem)) return nullptr;

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) return pItem;

    return nullptr;
}

auto Factory::SecurityContract(
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement) const noexcept(false)
    -> OTSecurityContract
{
    auto output = opentxs::Factory::SecurityContract(
        api_,
        nym,
        shortname,
        terms,
        unitOfAccount,
        version,
        reason,
        displayDefinition,
        redemptionIncrement);

    if (output) {
        return OTSecurityContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create currency contract");
    }
}

auto Factory::SecurityContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTSecurityContract
{
    auto output = opentxs::Factory::SecurityContract(api_, nym, serialized);

    if (output) {
        return OTSecurityContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate currency contract");
    }
}

auto Factory::ServerContract() const noexcept(false) -> OTServerContract
{
    return OTServerContract{opentxs::Factory::ServerContract(api_)};
}

auto Factory::ServerID() const -> OTNotaryID
{
    return identifier::Notary::Factory();
}

auto Factory::ServerID(const UnallocatedCString& serialized) const -> OTNotaryID
{
    return identifier::Notary::Factory(serialized);
}

auto Factory::ServerID(const opentxs::String& serialized) const -> OTNotaryID
{
    return identifier::Notary::Factory(serialized);
}

auto Factory::ServerID(const opentxs::network::zeromq::Frame& bytes) const
    -> OTNotaryID
{
    auto out = ServerID();
    out->Assign(bytes.data(), bytes.size());

    return out;
}

auto Factory::ServerID(const proto::Identifier& in) const noexcept -> OTNotaryID
{
    return factory::IdentifierNotary(in);
}

auto Factory::ServerID(const opentxs::Identifier& in) const noexcept
    -> OTNotaryID
{
    auto out = ServerID();

    if ((identifier::Type::notary == in.Type() ||
         (identifier::Type::generic == in.Type()))) {
        out->Assign(in);
    }

    return out;
}

auto Factory::ServerID(const google::protobuf::MessageLite& proto) const
    -> OTIdentifier
{
    const auto id = [&] {
        const auto bytes = Data(proto);
        auto out = ServerID();
        out->CalculateDigest(bytes->Bytes());

        return out;
    }();

    return Identifier(id->str());
}

auto Factory::SignedFile() const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_));

    return signedfile;
}

auto Factory::SignedFile(const String& LOCAL_SUBDIR, const String& FILE_NAME)
    const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}
auto Factory::SignedFile(const char* LOCAL_SUBDIR, const String& FILE_NAME)
    const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

auto Factory::SignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME) const
    -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

auto Factory::SmartContract() const -> std::unique_ptr<OTSmartContract>
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_));

    return smartcontract;
}

auto Factory::SmartContract(const identifier::Notary& NOTARY_ID) const
    -> std::unique_ptr<OTSmartContract>
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_, NOTARY_ID));

    return smartcontract;
}

auto Factory::StoreSecret(
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const contract::peer::SecretType type,
    const UnallocatedCString& primary,
    const UnallocatedCString& secondary,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> OTStoreSecret
{
    auto output = opentxs::Factory::StoreSecret(
        api_, nym, recipient, type, primary, secondary, server, reason);

    if (output) {
        return OTStoreSecret{std::move(output)};
    } else {
        throw std::runtime_error("Failed to create bailment reply");
    }
}

auto Factory::StoreSecret(
    const Nym_p& nym,
    const proto::PeerRequest& serialized) const noexcept(false) -> OTStoreSecret
{
    auto output = opentxs::Factory::StoreSecret(api_, nym, serialized);

    if (output) {
        return OTStoreSecret{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

auto Factory::SymmetricKey() const -> OTSymmetricKey
{
    return OTSymmetricKey{factory::SymmetricKey()};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::PasswordPrompt& password,
    const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> OTSymmetricKey
{
    return OTSymmetricKey{factory::SymmetricKey(api_, engine, password, mode)};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized) const -> OTSymmetricKey
{
    return OTSymmetricKey{factory::SymmetricKey(api_, engine, serialized)};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const opentxs::crypto::key::symmetric::Source type) const -> OTSymmetricKey
{
    return OTSymmetricKey{factory::SymmetricKey(
        api_, engine, seed, operations, difficulty, size, type)};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t size,
    const opentxs::crypto::key::symmetric::Source type) const -> OTSymmetricKey
{
    return OTSymmetricKey{factory::SymmetricKey(
        api_,
        engine,
        seed,
        salt,
        operations,
        difficulty,
        parallel,
        size,
        type)};
}

auto Factory::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& raw,
    const opentxs::PasswordPrompt& reason) const -> OTSymmetricKey
{
    return OTSymmetricKey{factory::SymmetricKey(api_, engine, raw, reason)};
}

auto Factory::Trade() const -> std::unique_ptr<OTTrade>
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(api_));

    return trade;
}

auto Factory::Trade(
    const identifier::Notary& notaryID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const opentxs::Identifier& assetAcctId,
    const identifier::Nym& nymID,
    const identifier::UnitDefinition& currencyId,
    const opentxs::Identifier& currencyAcctId) const -> std::unique_ptr<OTTrade>
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(
        api_,
        notaryID,
        instrumentDefinitionID,
        assetAcctId,
        nymID,
        currencyId,
        currencyAcctId));

    return trade;
}

auto Factory::Transaction(const String& strInput) const
    -> std::unique_ptr<OTTransactionType>
{
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed = DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<OTTransactionType> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED TRANSACTION-----"))  // this string is 34
                                                        // chars long.
        {
            pContract.reset(new OTTransaction(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRANSACTION ITEM-----"))  // this
                                                                    // string is
                                                                    // 39 chars
                                                                    // long.
        {
            pContract.reset(new opentxs::Item(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED LEDGER-----"))  // this string is 29
                                                          // chars long.
        {
            pContract.reset(new opentxs::Ledger(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED ACCOUNT-----"))  // this string is 30
                                                           // chars long.
        {
            OT_FAIL;
        }

        // The string didn't match any of the options in the factory.
        //

        //        const char* szFunc = "OTTransactionType::TransactionFactory";
        // The string didn't match any of the options in the factory.
        if (nullptr == pContract) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Object type not yet supported by class "
                "factory: ")(strFirstLine)
                .Flush();
            return nullptr;
        }

        // This causes pItem to load ASSUMING that the PurportedAcctID and
        // PurportedNotaryID are correct.
        // The object is still expected to be internally consistent with its
        // sub-items, regarding those IDs,
        // but the big difference is that it will SET the Real Acct and Real
        // Notary IDs based on the purported
        // values. This way you can load a transaction without knowing the
        // account in advance.
        //
        pContract->SetLoadInsecure();

        // Does the contract successfully load from the string passed in?
        if (pContract->LoadContractFromString(strContract)) {
            // NOTE: this already happens in OTTransaction::ProcessXMLNode and
            // OTLedger::ProcessXMLNode.
            // Specifically, it happens when m_bLoadSecurely is set to false.
            //
            //          pContract->SetRealNotaryID(pItem->GetPurportedNotaryID());
            //          pContract->SetRealAccountID(pItem->GetPurportedAccountID());

            return pContract;
        } else {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed loading contract from string (first "
                "line): ")(strFirstLine)
                .Flush();
        }
    }

    return nullptr;
}

auto Factory::Transaction(const opentxs::Ledger& theOwner) const
    -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(api_, theOwner));

    return transaction;
}

auto Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Notary& theNotaryID,
    originType theOriginType) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_, theNymID, theAccountID, theNotaryID, theOriginType));

    return transaction;
}

auto Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Notary& theNotaryID,
    std::int64_t lTransactionNum,
    originType theOriginType) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));

    return transaction;
}
// THIS factory only used when loading an abbreviated box receipt
// (inbox, nymbox, or outbox receipt).
// The full receipt is loaded only after the abbreviated ones are loaded,
// and verified against them.
auto Factory::Transaction(
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
    NumList* pNumList) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lNumberOfOrigin,
        theOriginType,
        lTransactionNum,
        lInRefTo,
        lInRefDisplay,
        the_DATE_SIGNED,
        theType,
        strHash,
        lAdjustment,
        lDisplayValue,
        lClosingNum,
        lRequestNum,
        bReplyTransSuccess,
        pNumList));

    return transaction;
}

auto Factory::Transaction(
    const opentxs::Ledger& theOwner,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const -> std::unique_ptr<OTTransaction>
{
    auto pTransaction = Transaction(
        theOwner.GetNymID(),
        theOwner.GetPurportedAccountID(),
        theOwner.GetPurportedNotaryID(),
        theType,
        theOriginType,
        lTransactionNum);
    if (false != bool(pTransaction)) pTransaction->SetParent(theOwner);

    return pTransaction;
}

auto Factory::Transaction(
    const identifier::Nym& theNymID,
    const opentxs::Identifier& theAccountID,
    const identifier::Notary& theNotaryID,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));
    OT_ASSERT(false != bool(transaction));

    transaction->m_Type = theType;

    // Since we're actually generating this transaction, then we can go ahead
    // and set the purported account and server IDs (we have already set the
    // real ones in the constructor). Now both sets are fill with matching data.
    // No need to security check the IDs since we are creating this transaction
    // versus loading and inspecting it.
    transaction->SetPurportedAccountID(theAccountID);
    transaction->SetPurportedNotaryID(theNotaryID);

    return transaction;
}

auto Factory::UnitDefinition() const noexcept -> OTUnitDefinition
{
    return OTUnitDefinition{opentxs::Factory::UnitDefinition(api_)};
}

auto Factory::UnitDefinition(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTUnitDefinition
{
    auto output = opentxs::Factory::UnitDefinition(api_, nym, serialized);

    if (output) {
        return OTUnitDefinition{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate unit definition");
    }
}

auto Factory::UnitID() const -> OTUnitID
{
    return identifier::UnitDefinition::Factory();
}

auto Factory::UnitID(const UnallocatedCString& serialized) const -> OTUnitID
{
    return identifier::UnitDefinition::Factory(serialized);
}

auto Factory::UnitID(const opentxs::String& serialized) const -> OTUnitID
{
    return identifier::UnitDefinition::Factory(serialized);
}

auto Factory::UnitID(const opentxs::network::zeromq::Frame& bytes) const
    -> OTUnitID
{
    auto out = UnitID();
    out->Assign(bytes.data(), bytes.size());

    return out;
}

auto Factory::UnitID(const proto::Identifier& in) const noexcept -> OTUnitID
{
    return factory::IdentifierUnit(in);
}

auto Factory::UnitID(const opentxs::Identifier& in) const noexcept -> OTUnitID
{
    auto out = UnitID();

    if ((identifier::Type::unitdefinition == in.Type() ||
         (identifier::Type::generic == in.Type()))) {
        out->Assign(in);
    }

    return out;
}

auto Factory::UnitID(const google::protobuf::MessageLite& proto) const
    -> OTIdentifier
{
    const auto id = [&] {
        const auto bytes = Data(proto);
        auto out = UnitID();
        out->CalculateDigest(bytes->Bytes());

        return out;
    }();

    return Identifier(id->str());
}
}  // namespace opentxs::api::session::imp
