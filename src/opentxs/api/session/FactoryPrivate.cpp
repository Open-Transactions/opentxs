// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/FactoryPrivate.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <BlockchainBlockHeader.pb.h>
#include <BlockchainPeerAddress.pb.h>  // IWYU pragma: keep
#include <Ciphertext.pb.h>
#include <Claim.pb.h>
#include <Envelope.pb.h>  // IWYU pragma: keep
#include <PaymentCode.pb.h>
#include <PeerEnums.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <UnitDefinition.pb.h>
#include <Verification.pb.h>
#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/transaction/TransactionPrivate.hpp"
#include "internal/api/Crypto.hpp"
#include "internal/api/crypto/Asymmetric.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/CurrencyContract.hpp"
#include "internal/core/contract/SecurityContract.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/reply/Factory.hpp"
#include "internal/core/contract/peer/request/Factory.hpp"
#include "internal/crypto/asymmetric/Factory.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/library/Secp256k1.hpp"
#include "internal/crypto/symmetric/Factory.hpp"
#include "internal/identity/wot/verification/Factory.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/basket/Basket.hpp"
#include "internal/otx/common/cron/OTCron.hpp"  // IWYU pragma: keep
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/crypto/OTSignedFile.hpp"
#include "internal/otx/common/recurring/OTPaymentPlan.hpp"
#include "internal/otx/common/script/OTScriptable.hpp"
#include "internal/otx/common/trade/OTMarket.hpp"
#include "internal/otx/common/trade/OTOffer.hpp"
#include "internal/otx/common/trade/OTTrade.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/BlockchainBlockHeader.hpp"
#include "internal/serialization/protobuf/verify/Envelope.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Category.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Type.hpp"      // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
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
#include "opentxs/crypto/Bip32Child.hpp"    // IWYU pragma: keep
#include "opentxs/crypto/Bip43Purpose.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/Verification.hpp"
#include "opentxs/identity/wot/internal.factory.hpp"
#include "opentxs/internal.factory.hpp"
#include "opentxs/network/otdht/Base.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/HDIndex.hpp"
#include "util/PasswordPromptPrivate.hpp"

namespace opentxs::api::session
{
using namespace std::literals;

FactoryPrivate::FactoryPrivate(
    const api::internal::Session& api,
    const api::Factory& parent)
    : api::internal::Factory()
    , api_(api)
    , parent_(parent)
    , p_asymmetric_(factory::AsymmetricAPI(api_))
    , asymmetric_(*p_asymmetric_)
    , p_symmetric_(factory::Symmetric(api_))
    , symmetric_(*p_symmetric_)
    , self_(this)
{
    assert_false(nullptr == p_asymmetric_);
    assert_false(nullptr == p_symmetric_);
}

auto FactoryPrivate::AsymmetricKey(
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    return AsymmetricKey(
        opentxs::crypto::asymmetric::Key::DefaultVersion(),
        opentxs::crypto::asymmetric::Role::Sign,
        params,
        reason);
}

auto FactoryPrivate::AsymmetricKey(
    VersionNumber version,
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    return AsymmetricKey(
        version, opentxs::crypto::asymmetric::Role::Sign, params, reason);
}

auto FactoryPrivate::AsymmetricKey(
    opentxs::crypto::asymmetric::Role role,
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    return AsymmetricKey(
        opentxs::crypto::asymmetric::Key::DefaultVersion(),
        role,
        params,
        reason);
}

auto FactoryPrivate::AsymmetricKey(
    VersionNumber version,
    opentxs::crypto::asymmetric::Role role,
    const opentxs::crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> opentxs::crypto::asymmetric::Key
{
    auto output = asymmetric_.NewKey(params, role, version, reason);

    if (output.IsValid()) {

        return output;
    } else {
        throw std::runtime_error("Failed to create asymmetric key");
    }
}

auto FactoryPrivate::AsymmetricKey(const proto::AsymmetricKey& serialized)
    const noexcept -> opentxs::crypto::asymmetric::Key
{
    auto output = asymmetric_.Internal().InstantiateKey(serialized);

    if (output.IsValid()) {

        return opentxs::crypto::asymmetric::Key{std::move(output)};
    } else {
        LogError()()("Failed to instantiate asymmetric key").Flush();

        return {};
    }
}

auto FactoryPrivate::AsymmetricKey(
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    return AsymmetricKey(
        opentxs::crypto::asymmetric::Key::DefaultVersion(),
        opentxs::crypto::asymmetric::Role::Sign,
        type,
        key,
        reason,
        alloc);
}

auto FactoryPrivate::AsymmetricKey(
    VersionNumber version,
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    return AsymmetricKey(
        version,
        opentxs::crypto::asymmetric::Role::Sign,
        type,
        key,
        reason,
        alloc);
}

auto FactoryPrivate::AsymmetricKey(
    opentxs::crypto::asymmetric::Role role,
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    return AsymmetricKey(
        opentxs::crypto::asymmetric::Key::DefaultVersion(),
        role,
        type,
        key,
        reason,
        alloc);
}

auto FactoryPrivate::AsymmetricKey(
    VersionNumber version,
    opentxs::crypto::asymmetric::Role role,
    opentxs::crypto::asymmetric::Algorithm type,
    const opentxs::Secret& key,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> opentxs::crypto::asymmetric::Key
{
    try {
        auto pubkey = ByteArray{};

        switch (type) {
            using enum opentxs::crypto::asymmetric::Algorithm;

            case Secp256k1: {
                const auto& ecdsa = api_.Crypto().Internal().Libsecp256k1();

                if (false ==
                    ecdsa.ScalarMultiplyBase(key.Bytes(), pubkey.WriteInto())) {
                    throw std::runtime_error{"failed to calculate pubkey"};
                }

                return factory::Secp256k1Key(
                    api_.Self(),
                    ecdsa,
                    key,
                    pubkey,
                    role,
                    version,
                    reason,
                    alloc.result_);
            }
            case ED25519:
            case Error:
            case Null:
            case Legacy:
            default: {
                throw std::runtime_error{
                    "unsupported key type: "s.append(print(type))};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what())(print(type)).Flush();

        return {};
    }
}

auto FactoryPrivate::BailmentNoticeReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    bool value,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::reply::BailmentNotice
{
    return factory::BailmentNoticeReply(
        api_.Self(),
        responder,
        initiator,
        inReferenceToRequest,
        value,
        reason,
        alloc);
}

auto FactoryPrivate::BailmentNoticeRequest(
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
    return factory::BailmentNoticeRequest(
        api_.Self(),
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

auto FactoryPrivate::BailmentReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    std::string_view terms,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Bailment
{
    return factory::BailmentReply(
        api_.Self(),
        responder,
        initiator,
        inReferenceToRequest,
        terms,
        reason,
        alloc);
}

auto FactoryPrivate::BailmentRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& notary,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::request::Bailment
{
    return factory::BailmentRequest(
        api_.Self(), initiator, responder, unit, notary, reason, alloc);
}

auto FactoryPrivate::Basket() const -> std::unique_ptr<opentxs::Basket>
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_.Self()));

    return basket;
}

auto FactoryPrivate::Basket(
    std::int32_t nCount,
    const opentxs::Amount& lMinimumTransferAmount) const
    -> std::unique_ptr<opentxs::Basket>
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(
        new opentxs::Basket(api_.Self(), nCount, lMinimumTransferAmount));

    return basket;
}

auto FactoryPrivate::BasketContract(
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const std::uint64_t weight,
    const UnitType unitOfAccount,
    const VersionNumber version,
    const display::Definition& displayDefinition,
    const opentxs::Amount& redemptionIncrement) const noexcept(false)
    -> OTBasketContract
{
    auto output = opentxs::Factory::BasketContract(
        api_.Self(),
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

auto FactoryPrivate::BasketContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTBasketContract
{
    auto output =
        opentxs::Factory::BasketContract(api_.Self(), nym, serialized);

    if (output) {
        return OTBasketContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate bailment request");
    }
}

auto FactoryPrivate::BitcoinBlock(
    const blockchain::block::Header& previous,
    blockchain::block::Transaction generationTransaction,
    std::uint32_t nBits,
    std::span<blockchain::block::Transaction> extraTransactions,
    std::int32_t version,
    AbortFunction abort,
    alloc::Default alloc) const noexcept -> blockchain::block::Block
{
    auto extra = [&] {
        auto& in = extraTransactions;
        auto out =
            Vector<blockchain::protocol::bitcoin::base::block::Transaction>{};
        out.reserve(in.size());
        out.clear();
        std::ranges::transform(in, std::back_inserter(out), [](auto& tx) {
            return std::move(tx).asBitcoin();
        });

        return out;
    }();

    return factory::BitcoinBlock(
        api_.Crypto(),
        previous,
        std::move(generationTransaction).asBitcoin(),
        nBits,
        extra,
        version,
        abort,
        alloc);
}

auto FactoryPrivate::BitcoinScriptNullData(
    const blockchain::Type chain,
    std::span<const ReadView> data,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return factory::BitcoinScriptNullData(chain, data, alloc);
}

auto FactoryPrivate::BitcoinScriptP2MS(
    const blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return factory::BitcoinScriptP2MS(chain, M, N, keys, alloc);
}

auto FactoryPrivate::BitcoinScriptP2PK(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return factory::BitcoinScriptP2PK(chain, key, alloc);
}

auto FactoryPrivate::BitcoinScriptP2PKH(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return factory::BitcoinScriptP2PKH(api_.Crypto(), chain, key, alloc);
}

auto FactoryPrivate::BitcoinScriptP2SH(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return factory::BitcoinScriptP2SH(api_.Crypto(), chain, script, alloc);
}

auto FactoryPrivate::BitcoinScriptP2WPKH(
    const opentxs::blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return factory::BitcoinScriptP2WPKH(api_.Crypto(), chain, key, alloc);
}

auto FactoryPrivate::BitcoinScriptP2WSH(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Default alloc) const noexcept
    -> blockchain::protocol::bitcoin::base::block::Script
{
    return factory::BitcoinScriptP2WSH(api_.Crypto(), chain, script, alloc);
}

auto FactoryPrivate::BlockchainBlock(
    const opentxs::blockchain::Type chain,
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> blockchain::block::Block
{
    return factory::BitcoinBlock(api_.Crypto(), chain, bytes, alloc);
}

auto FactoryPrivate::BlockchainSyncMessage(
    const opentxs::network::zeromq::Message& in) const noexcept
    -> std::unique_ptr<opentxs::network::otdht::Base>
{
    return factory::BlockchainSyncMessage(api_.Self(), in);
}

auto FactoryPrivate::BlockchainTransaction(
    const blockchain::Type chain,
    const ReadView bytes,
    const bool isGeneration,
    const Time time,
    alloc::Default alloc) const noexcept -> blockchain::block::Transaction
{
    return factory::BitcoinTransaction(
        api_.Crypto(),
        chain,
        isGeneration ? 0_uz : std::numeric_limits<std::size_t>::max(),
        time,
        bytes,
        alloc);
}

auto FactoryPrivate::BlockchainTransaction(
    const blockchain::Type chain,
    const blockchain::block::Height height,
    std::span<blockchain::OutputBuilder> scripts,
    ReadView coinbase,
    std::int32_t version,
    alloc::Default alloc) const noexcept -> blockchain::block::Transaction
{
    return factory::BitcoinTransaction(
        api_.Crypto(),
        chain,
        height,
        std::move(scripts),
        coinbase,
        version,
        alloc);
}

auto FactoryPrivate::BlockchainTransaction(
    const proto::BlockchainTransaction& serialized,
    alloc::Default alloc) const noexcept -> blockchain::block::Transaction
{
    return factory::BitcoinTransaction(
        api_.Crypto().Blockchain(), api_.Factory(), serialized, alloc);
}

auto FactoryPrivate::BlockHeader(
    const proto::BlockchainBlockHeader& proto,
    alloc::Default alloc) const noexcept -> blockchain::block::Header
{
    try {
        if (false == proto::Validate(proto, VERBOSE)) {

            throw std::runtime_error{"invalid protobuf"};
        }

        const auto type(static_cast<blockchain::Type>(proto.type()));
        using enum blockchain::Category;

        switch (blockchain::category(type)) {
            case output_based: {

                return factory::BitcoinBlockHeader(api_.Crypto(), proto, alloc);
            }
            case unknown_category:
            case balance_based:
            default: {

                throw std::runtime_error{
                    "unsupported header type: "s.append(print(type))};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what())().Flush();

        return {alloc};
    }
}

auto FactoryPrivate::BlockHeaderForUnitTests(
    const opentxs::blockchain::block::Hash& hash,
    const opentxs::blockchain::block::Hash& parent,
    const opentxs::blockchain::block::Height height,
    alloc::Default alloc) const noexcept -> blockchain::block::Header
{
    return factory::BitcoinBlockHeader(
        api_.Crypto(),
        opentxs::blockchain::Type::UnitTest,
        hash,
        parent,
        height,
        alloc);
}

auto FactoryPrivate::BlockHeaderFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> blockchain::block::Header
{
    return BlockHeader(
        proto::Factory<proto::BlockchainBlockHeader>(bytes), alloc);
}

auto FactoryPrivate::BlockHeaderFromNative(
    const opentxs::blockchain::Type type,
    const ReadView raw,
    alloc::Default alloc) const noexcept -> blockchain::block::Header
{
    try {
        using enum blockchain::Category;

        switch (blockchain::category(type)) {
            case output_based: {

                return factory::BitcoinBlockHeader(
                    api_.Crypto(), type, raw, alloc);
            }
            case unknown_category:
            case balance_based:
            default: {

                throw std::runtime_error{
                    "unsupported header type: "s.append(print(type))};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what())().Flush();

        return {alloc};
    }
}

auto FactoryPrivate::Cheque(const OTTransaction& receipt) const
    -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> output{new opentxs::Cheque{api_.Self()}};

    assert_false(nullptr == output);

    auto serializedItem = String::Factory();
    receipt.GetReferenceString(serializedItem);
    std::unique_ptr<opentxs::Item> item{Item(
        serializedItem,
        receipt.GetRealNotaryID(),
        receipt.GetReferenceToNum())};

    assert_true(false != bool(item));

    auto serializedCheque = String::Factory();
    item->GetAttachment(serializedCheque);
    const auto loaded = output->LoadContractFromString(serializedCheque);

    if (false == loaded) { LogError()()("Failed to load cheque.").Flush(); }

    return output;
}

auto FactoryPrivate::Cheque() const -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(new opentxs::Cheque(api_.Self()));

    return cheque;
}

auto FactoryPrivate::Cheque(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
    -> std::unique_ptr<opentxs::Cheque>
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(
        new opentxs::Cheque(api_.Self(), NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return cheque;
}

auto FactoryPrivate::Claim(
    const identity::wot::Claimant& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    ReadView subtype,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    VersionNumber version,
    alloc::Strategy alloc) const noexcept -> identity::wot::Claim
{
    return factory::Claim(
        api_.Self(),
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

auto FactoryPrivate::Claim(
    const identity::Nym& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    ReadView subtype,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    VersionNumber version,
    alloc::Strategy alloc) const noexcept -> identity::wot::Claim
{
    return Claim(
        claimant.ID(),
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

auto FactoryPrivate::Claim(ReadView serialized, alloc::Strategy alloc)
    const noexcept -> identity::wot::Claim
{
    return Claim(proto::Factory<proto::Claim>(serialized), alloc);
}

auto FactoryPrivate::Claim(
    const identity::wot::Claimant& claimant,
    const identity::wot::claim::SectionType section,
    const proto::ContactItem& proto,
    alloc::Strategy alloc) const noexcept -> identity::wot::Claim
{
    return factory::Claim(api_.Self(), claimant, section, proto, alloc);
}

auto FactoryPrivate::Claim(const proto::Claim& proto, alloc::Strategy alloc)
    const noexcept -> identity::wot::Claim
{
    return factory::Claim(api_.Self(), proto, alloc);
}

auto FactoryPrivate::ConnectionReply(
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
    return factory::ConnectionReply(
        api_.Self(),
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

auto FactoryPrivate::ConnectionRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const contract::peer::ConnectionInfoType kind,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::request::Connection
{
    return factory::ConnectionRequest(
        api_.Self(), initiator, responder, kind, reason, alloc);
}

auto FactoryPrivate::Contract(const opentxs::String& strInput) const
    -> std::unique_ptr<opentxs::Contract>
{
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed =
        DearmorAndTrim(api_.Crypto(), strInput, strContract, strFirstLine);

    if (bProcessed) {

        std::unique_ptr<opentxs::Contract> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                          // chars long.
        {
            pContract.reset(new OTSmartContract(api_.Self()));
            assert_true(false != bool(pContract));
        }

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED PAYMENT PLAN-----"))  // this string is 35
                                                         // chars long.
        {
            pContract.reset(new OTPaymentPlan(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRADE-----"))  // this string is 28
                                                         // chars long.
        {
            pContract.reset(new OTTrade(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED OFFER-----")) {
            pContract.reset(new OTOffer(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED INVOICE-----")) {
            pContract.reset(new opentxs::Cheque(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract.reset(new opentxs::Cheque(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract.reset(new opentxs::Cheque(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract.reset(new opentxs::Message(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MINT-----")) {
            auto mint = Mint();
            pContract.reset(mint.Release());
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED FILE-----")) {
            assert_true(false != bool(pContract));
        }

        // The string didn't match any of the options in the factory.
        //
        if (!pContract) {
            LogConsole()()("Object type not yet supported by class "
                           "factory: ")(strFirstLine.get())
                .Flush();
            // Does the contract successfully load from the string passed in?
        } else if (!pContract->LoadContractFromString(strContract)) {
            LogConsole()()("Failed loading contract from string (first "
                           "line): ")(strFirstLine.get())
                .Flush();
        } else {
            return pContract;
        }
    }
    return nullptr;
}

auto FactoryPrivate::Cron() const -> std::unique_ptr<OTCron> { return {}; }

auto FactoryPrivate::CronItem(const String& strCronItem) const
    -> std::unique_ptr<OTCronItem>
{
    std::array<char, 45> buf{};

    if (!strCronItem.Exists()) {
        LogError()()("Empty string was passed in (returning nullptr).").Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strCronItem.Get());

    if (!strContract->DecodeIfArmored(api_.Crypto(), false)) {
        LogError()()("Input string apparently was encoded and "
                     "then failed decoding. Contents: ")(strCronItem)(".")
            .Flush();
        return nullptr;
    }

    strContract->reset();  // for sgets
    const bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) { return nullptr; }

    auto strFirstLine = String::Factory(buf.data());
    // set the "file" pointer within this string back to index 0.
    strContract->reset();

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -")) {
        return nullptr;
    }

    // By this point we know already that it's not escaped.
    // BUT it might still be ARMORED!

    std::unique_ptr<OTCronItem> pItem;
    // this string is 35 chars long.
    if (strFirstLine->Contains("-----BEGIN SIGNED PAYMENT PLAN-----")) {
        pItem.reset(new OTPaymentPlan(api_.Self()));
    }
    // this string is 28 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED TRADE-----")) {
        pItem.reset(new OTTrade(api_.Self()));
    }
    // this string is 36 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED SMARTCONTRACT-----")) {
        pItem.reset(new OTSmartContract(api_.Self()));
    } else {
        return nullptr;
    }

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) { return pItem; }

    return nullptr;
}

auto FactoryPrivate::CurrencyContract(
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const opentxs::Amount& redemptionIncrement) const noexcept(false)
    -> OTCurrencyContract
{
    auto output = opentxs::Factory::CurrencyContract(
        api_.Self(),
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

auto FactoryPrivate::CurrencyContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTCurrencyContract
{
    auto output =
        opentxs::Factory::CurrencyContract(api_.Self(), nym, serialized);

    if (output) {
        return OTCurrencyContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate currency contract");
    }
}

auto FactoryPrivate::Envelope() const noexcept -> OTEnvelope
{
    return OTEnvelope{opentxs::Factory::Envelope(api_.Self()).release()};
}

auto FactoryPrivate::Envelope(const opentxs::Armored& in) const noexcept(false)
    -> OTEnvelope
{
    auto data = Data();

    if (false == in.GetData(data)) {
        throw std::runtime_error("Invalid armored envelope");
    }

    return Envelope(
        proto::Factory<opentxs::crypto::Envelope::SerializedType>(data));
}

auto FactoryPrivate::Envelope(
    const opentxs::crypto::Envelope::SerializedType& serialized) const
    noexcept(false) -> OTEnvelope
{
    if (false == proto::Validate(serialized, VERBOSE)) {
        throw std::runtime_error("Invalid serialized envelope");
    }

    return OTEnvelope{
        opentxs::Factory::Envelope(api_.Self(), serialized).release()};
}

auto FactoryPrivate::Envelope(const opentxs::ReadView& serialized) const
    noexcept(false) -> OTEnvelope
{
    return OTEnvelope{
        opentxs::Factory::Envelope(api_.Self(), serialized).release()};
}

auto FactoryPrivate::FaucetReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    const blockchain::block::Transaction& transaction,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Faucet
{
    return factory::FaucetReply(
        api_.Self(),
        responder,
        initiator,
        inReferenceToRequest,
        transaction,
        reason,
        alloc);
}

auto FactoryPrivate::FaucetRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    opentxs::UnitType unit,
    std::string_view address,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::request::Faucet
{
    return factory::FaucetRequest(
        api_.Self(), initiator, responder, unit, address, reason, alloc);
}

auto FactoryPrivate::Identifier(
    const opentxs::Contract& contract,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return parent_.Internal().Identifier(contract, std::move(alloc));
}

auto FactoryPrivate::Identifier(
    const opentxs::Cheque& cheque,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return parent_.Internal().Identifier(cheque, std::move(alloc));
}

auto FactoryPrivate::Identifier(const opentxs::Item& item, alloc::Default alloc)
    const noexcept -> identifier::Generic
{
    return parent_.Internal().Identifier(item, std::move(alloc));
}

auto FactoryPrivate::IdentifierFromPreimage(
    const ProtobufType& proto,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return parent_.Internal().IdentifierFromPreimage(proto, std::move(alloc));
}

auto FactoryPrivate::IdentifierFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return parent_.Internal().IdentifierFromPreimage(
        proto, type, std::move(alloc));
}

auto FactoryPrivate::Identifier(
    const proto::Identifier& in,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return parent_.Internal().Identifier(in, std::move(alloc));
}

auto FactoryPrivate::Item(const UnallocatedCString& serialized) const
    -> std::unique_ptr<opentxs::Item>
{
    return Item(String::Factory(serialized));
}

auto FactoryPrivate::Item(const String& serialized) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> output{new opentxs::Item(api_.Self())};

    if (output) {
        const auto loaded = output->LoadContractFromString(serialized);

        if (false == loaded) {
            LogError()()("Unable to deserialize.").Flush();
            output.reset();
        }
    } else {
        LogError()()("Unable to instantiate.").Flush();
    }

    return output;
}

auto FactoryPrivate::Item(
    const identifier::Nym& theNymID,
    const opentxs::Item& theOwner) const -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_.Self(), theNymID, theOwner));

    return item;
}

auto FactoryPrivate::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner) const -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_.Self(), theNymID, theOwner));

    return item;
}

auto FactoryPrivate::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner,
    otx::itemType theType,
    const identifier::Account& pDestinationAcctID) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(
        api_.Self(), theNymID, theOwner, theType, pDestinationAcctID));

    return item;
}

// Sometimes I don't know user ID of the originator, or the account ID of the
// originator,
// until after I have loaded the item. It's simply impossible to set those
// values ahead
// of time, sometimes. In those cases, we set the values appropriately but then
// we need
// to verify that the user ID is actually the owner of the AccountID. TOdo that.
auto FactoryPrivate::Item(
    const String& strItem,
    const identifier::Notary& theNotaryID,
    std::int64_t lTransactionNumber) const -> std::unique_ptr<opentxs::Item>
{
    if (!strItem.Exists()) {
        LogError()()("strItem is empty. (Expected an "
                     "item).")
            .Flush();
        return nullptr;
    }

    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(api_.Self())};

    // So when it loads its own server ID, we can compare to this one.
    pItem->SetRealNotaryID(theNotaryID);

    // This loads up the purported account ID and the user ID.
    if (pItem->LoadContractFromString(strItem)) {
        const auto& ACCOUNT_ID = pItem->GetPurportedAccountID();
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
auto FactoryPrivate::Item(
    const OTTransaction& theOwner,
    otx::itemType theType,
    const identifier::Account& pDestinationAcctID) const
    -> std::unique_ptr<opentxs::Item>
{
    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(
        api_.Self(),
        theOwner.GetNymID(),
        theOwner,
        theType,
        pDestinationAcctID)};

    if (false != bool(pItem)) {
        pItem->SetPurportedAccountID(theOwner.GetPurportedAccountID());
        pItem->SetPurportedNotaryID(theOwner.GetPurportedNotaryID());
        return pItem;
    }
    return nullptr;
}

auto FactoryPrivate::Keypair(
    const opentxs::crypto::Parameters& params,
    const VersionNumber version,
    const opentxs::crypto::asymmetric::Role role,
    const opentxs::PasswordPrompt& reason) const -> OTKeypair
{
    auto privateKey = asymmetric_.NewKey(params, role, version, reason);

    if (false == privateKey.IsValid()) {
        LogError()()("Failed to derive private key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    auto publicKey = privateKey.asPublic();

    if (false == publicKey.IsValid()) {
        LogError()()("Failed to derive public key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_.Self(), role, std::move(publicKey), std::move(privateKey))};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto FactoryPrivate::Keypair(
    const proto::AsymmetricKey& serializedPubkey,
    const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair
{
    auto pPrivateKey = asymmetric_.Internal().InstantiateKey(serializedPrivkey);

    if (false == pPrivateKey.IsValid()) {
        LogError()()("Failed to instantiate private key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    auto pPublicKey = asymmetric_.Internal().InstantiateKey(serializedPubkey);

    if (false == pPublicKey.IsValid()) {
        LogError()()("Failed to instantiate public key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_.Self(),
            translate(serializedPrivkey.role()),
            std::move(pPublicKey),
            std::move(pPrivateKey))};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto FactoryPrivate::Keypair(const proto::AsymmetricKey& serializedPubkey) const
    -> OTKeypair
{
    auto pPublicKey = asymmetric_.Internal().InstantiateKey(serializedPubkey);

    if (false == pPublicKey.IsValid()) {
        LogError()()("Failed to instantiate public key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_.Self(),
            translate(serializedPubkey.role()),
            std::move(pPublicKey),
            opentxs::crypto::asymmetric::Key{})};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto FactoryPrivate::Keypair(
    const opentxs::crypto::SeedID& fingerprint,
    const opentxs::crypto::Bip32Index nym,
    const opentxs::crypto::Bip32Index credset,
    const opentxs::crypto::Bip32Index credindex,
    const opentxs::crypto::EcdsaCurve& curve,
    const opentxs::crypto::asymmetric::Role role,
    const opentxs::PasswordPrompt& reason) const -> OTKeypair
{
    auto roleIndex = opentxs::crypto::Bip32Index{0};

    switch (role) {
        case opentxs::crypto::asymmetric::Role::Auth: {
            roleIndex = HDIndex{
                opentxs::crypto::Bip32Child::AUTH_KEY,
                opentxs::crypto::Bip32Child::HARDENED};
        } break;
        case opentxs::crypto::asymmetric::Role::Encrypt: {
            roleIndex = HDIndex{
                opentxs::crypto::Bip32Child::ENCRYPT_KEY,
                opentxs::crypto::Bip32Child::HARDENED};
        } break;
        case opentxs::crypto::asymmetric::Role::Sign: {
            roleIndex = HDIndex{
                opentxs::crypto::Bip32Child::SIGN_KEY,
                opentxs::crypto::Bip32Child::HARDENED};
        } break;
        case opentxs::crypto::asymmetric::Role::Error:
        default: {
            LogError()()("Invalid key role").Flush();

            return OTKeypair{factory::Keypair()};
        }
    }

    const auto path = UnallocatedVector<opentxs::crypto::Bip32Index>{
        HDIndex{
            opentxs::crypto::Bip43Purpose::NYM,
            opentxs::crypto::Bip32Child::HARDENED},
        HDIndex{nym, opentxs::crypto::Bip32Child::HARDENED},
        HDIndex{credset, opentxs::crypto::Bip32Child::HARDENED},
        HDIndex{credindex, opentxs::crypto::Bip32Child::HARDENED},
        roleIndex};
    auto privateKey =
        api_.Crypto().Seed().GetHDKey(fingerprint, curve, path, role, reason);

    if (false == privateKey.IsValid()) {
        LogError()()("Failed to derive private key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    auto publicKey = privateKey.asPublic();

    if (false == publicKey.IsValid()) {
        LogError()()("Failed to derive public key").Flush();

        return OTKeypair{factory::Keypair()};
    }

    try {
        return OTKeypair{factory::Keypair(
            api_.Self(), role, std::move(publicKey), std::move(privateKey))};
    } catch (...) {
        return OTKeypair{factory::Keypair()};
    }
}

auto FactoryPrivate::Ledger(
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID) const
    -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_.Self(), theAccountID, theNotaryID));

    return ledger;
}

auto FactoryPrivate::Ledger(
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID) const
    -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(
        new opentxs::Ledger(api_.Self(), theNymID, theAccountID, theNotaryID));

    return ledger;
}

auto FactoryPrivate::Ledger(
    const identifier::Nym& theNymID,
    const identifier::Account& theAcctID,
    const identifier::Notary& theNotaryID,
    otx::ledgerType theType,
    bool bCreateFile) const -> std::unique_ptr<opentxs::Ledger>
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(
        new opentxs::Ledger(api_.Self(), theNymID, theAcctID, theNotaryID));

    ledger->generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);

    return ledger;
}

auto FactoryPrivate::Ledger(
    const identifier::Nym& theNymID,
    const identifier::Nym& nymAsAccount,
    const identifier::Notary& theNotaryID) const
    -> std::unique_ptr<opentxs::Ledger>
{
    using enum identifier::AccountSubtype;

    return Ledger(
        theNymID,
        AccountIDFromHash(nymAsAccount.Bytes(), custodial_account, {}),
        theNotaryID);
}

auto FactoryPrivate::Ledger(
    const identifier::Nym& theNymID,
    const identifier::Nym& nymAsAccount,
    const identifier::Notary& theNotaryID,
    otx::ledgerType theType,
    bool bCreateFile) const -> std::unique_ptr<opentxs::Ledger>
{
    using enum identifier::AccountSubtype;

    return Ledger(
        theNymID,
        AccountIDFromHash(nymAsAccount.Bytes(), custodial_account, {}),
        theNotaryID,
        theType,
        bCreateFile);
}

auto FactoryPrivate::Market() const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_.Self()));

    return market;
}

auto FactoryPrivate::Market(const char* szFilename) const
    -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_.Self(), szFilename));

    return market;
}

auto FactoryPrivate::Market(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_TYPE_ID,
    const opentxs::Amount& lScale) const -> std::unique_ptr<OTMarket>
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(
        api_.Self(),
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        CURRENCY_TYPE_ID,
        lScale));

    return market;
}

auto FactoryPrivate::Message() const -> std::unique_ptr<opentxs::Message>
{
    std::unique_ptr<opentxs::Message> message;
    message.reset(new opentxs::Message(api_.Self()));

    return message;
}

auto FactoryPrivate::Mint(const otx::blind::CashType type) const noexcept
    -> otx::blind::Mint
{
    switch (type) {
        case otx::blind::CashType::Lucre: {

            return factory::MintLucre(api_.Self());
        }
        case otx::blind::CashType::Error:
        default: {
            LogError()()("unsupported cash type: ")(print(type)).Flush();

            return otx::blind::Mint{api_.Self()};
        }
    }
}

auto FactoryPrivate::Mint() const noexcept -> otx::blind::Mint
{
    return Mint(otx::blind::CashType::Lucre);
}

auto FactoryPrivate::Mint(
    const otx::blind::CashType type,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    switch (type) {
        case otx::blind::CashType::Lucre: {

            return factory::MintLucre(api_.Self(), notary, unit);
        }
        case otx::blind::CashType::Error:
        default: {
            LogError()()("unsupported cash type: ")(print(type)).Flush();

            return otx::blind::Mint{api_.Self()};
        }
    }
}

auto FactoryPrivate::Mint(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return Mint(otx::blind::CashType::Lucre, notary, unit);
}

auto FactoryPrivate::Mint(
    const otx::blind::CashType type,
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    switch (type) {
        case otx::blind::CashType::Lucre: {

            return factory::MintLucre(api_.Self(), notary, serverNym, unit);
        }
        case otx::blind::CashType::Error:
        default: {
            LogError()()("unsupported cash type: ")(print(type)).Flush();

            return otx::blind::Mint{api_.Self()};
        }
    }
}

auto FactoryPrivate::Mint(
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) const noexcept -> otx::blind::Mint
{
    return Mint(otx::blind::CashType::Lucre, notary, serverNym, unit);
}

auto FactoryPrivate::NotaryID(const proto::Identifier& in, alloc::Default alloc)
    const noexcept -> identifier::Notary
{
    return parent_.Internal().NotaryID(in, std::move(alloc));
}

auto FactoryPrivate::NotaryIDConvertSafe(
    const identifier::Generic& in,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return parent_.Internal().NotaryIDConvertSafe(in, std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return parent_.Internal().NotaryIDFromPreimage(
        proto, type, std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromPreimage(
    const ProtobufType& proto,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return parent_.Internal().NotaryIDFromPreimage(proto, std::move(alloc));
}

auto FactoryPrivate::NymIDFromPaymentCode(const UnallocatedCString& input) const
    -> identifier::Nym
{
    const auto code = PaymentCodeFromBase58(input);

    if (0 == code.Version()) { return identifier::Nym{}; }

    return code.ID();
}

auto FactoryPrivate::Offer() const -> std::unique_ptr<OTOffer>
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(api_.Self()));

    return offer;
}

auto FactoryPrivate::Offer(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_ID,
    const opentxs::Amount& MARKET_SCALE) const -> std::unique_ptr<OTOffer>
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(
        api_.Self(),
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        CURRENCY_ID,
        MARKET_SCALE));

    return offer;
}

auto FactoryPrivate::OutbailmentReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    std::string_view description,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Outbailment
{
    return factory::OutbailmentReply(
        api_.Self(),
        responder,
        initiator,
        inReferenceToRequest,
        description,
        reason,
        alloc);
}

auto FactoryPrivate::OutbailmentRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& notary,
    const opentxs::Amount& amount,
    std::string_view instructions,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::request::Outbailment
{
    return factory::OutbailmentRequest(
        api_.Self(),
        initiator,
        responder,
        unit,
        notary,
        amount,
        instructions,
        reason,
        alloc);
}

auto FactoryPrivate::PasswordPrompt(std::string_view text) const
    -> opentxs::PasswordPrompt
{
    auto alloc = alloc::Strategy{};  // TODO function argument

    return pmr::construct<PasswordPromptPrivate>(
        alloc.result_, api_.Self(), text);
}

auto FactoryPrivate::PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
    -> opentxs::PasswordPrompt
{
    return PasswordPrompt(rhs.GetDisplayString());
}

auto FactoryPrivate::Payment() const -> std::unique_ptr<OTPayment>
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_.Self()));

    return payment;
}

auto FactoryPrivate::Payment(const String& strPayment) const
    -> std::unique_ptr<OTPayment>
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_.Self(), strPayment));

    return payment;
}

auto FactoryPrivate::Payment(
    const opentxs::Contract& contract,
    const opentxs::PasswordPrompt& reason) const -> std::unique_ptr<OTPayment>
{
    auto payment = FactoryPrivate::Payment(String::Factory(contract));

    if (payment) { payment->SetTempValues(reason); }

    return payment;
}

auto FactoryPrivate::PaymentCode(
    const proto::PaymentCode& serialized) const noexcept -> opentxs::PaymentCode
{
    return factory::PaymentCode(api_.Self(), serialized);
}

auto FactoryPrivate::PaymentCode(
    const opentxs::crypto::SeedID& seed,
    const opentxs::crypto::Bip32Index nym,
    const std::uint8_t version,
    const opentxs::PasswordPrompt& reason,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const noexcept -> opentxs::PaymentCode
{
    return factory::PaymentCode(
        api_.Self(),
        seed,
        nym,
        version,
        bitmessage,
        bitmessageVersion,
        bitmessageStream,
        reason);
}

auto FactoryPrivate::PaymentCodeFromBase58(const ReadView base58) const noexcept
    -> opentxs::PaymentCode
{
    return factory::PaymentCode(api_.Self(), UnallocatedCString{base58});
}

auto FactoryPrivate::PaymentCodeFromProtobuf(
    const ReadView proto) const noexcept -> opentxs::PaymentCode
{
    return PaymentCode(proto::Factory<proto::PaymentCode>(proto));
}

auto FactoryPrivate::PaymentPlan() const -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(api_.Self()));

    return paymentplan;
}

auto FactoryPrivate::PaymentPlan(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
    -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(
        new OTPaymentPlan(api_.Self(), NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return paymentplan;
}

auto FactoryPrivate::PaymentPlan(
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::Account& SENDER_ACCT_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const identifier::Account& RECIPIENT_ACCT_ID,
    const identifier::Nym& RECIPIENT_NYM_ID) const
    -> std::unique_ptr<OTPaymentPlan>
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(
        api_.Self(),
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID));

    return paymentplan;
}

auto FactoryPrivate::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const UnallocatedCString& message) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()()("Peer objects are only supported in client sessions").Flush();

    return {};
}

auto FactoryPrivate::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] const UnallocatedCString& payment,
    [[maybe_unused]] const bool isPayment) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()()("Peer objects are only supported in client sessions").Flush();

    return {};
}

auto FactoryPrivate::PeerObject(
    [[maybe_unused]] const Nym_p& senderNym,
    [[maybe_unused]] otx::blind::Purse&& purse) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()()("Peer objects are only supported in client sessions").Flush();

    return {};
}

auto FactoryPrivate::PeerObject(
    [[maybe_unused]] const contract::peer::Request& request,
    [[maybe_unused]] const contract::peer::Reply& reply,
    [[maybe_unused]] const VersionNumber version) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()()("Peer objects are only supported in client sessions").Flush();

    return {};
}

auto FactoryPrivate::PeerObject(
    [[maybe_unused]] const contract::peer::Request& request,
    [[maybe_unused]] const VersionNumber version) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()()("Peer objects are only supported in client sessions").Flush();

    return {};
}

auto FactoryPrivate::PeerObject(
    [[maybe_unused]] const Nym_p& signerNym,
    [[maybe_unused]] const proto::PeerObject& serialized) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()()("Peer objects are only supported in client sessions").Flush();

    return {};
}

auto FactoryPrivate::PeerObject(
    [[maybe_unused]] const Nym_p& recipientNym,
    [[maybe_unused]] const opentxs::Armored& encrypted,
    [[maybe_unused]] const opentxs::PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::PeerObject>
{
    LogError()()("Peer objects are only supported in client sessions").Flush();

    return {};
}

auto FactoryPrivate::PeerReply(ReadView bytes, alloc::Strategy alloc)
    const noexcept -> contract::peer::Reply
{
    return PeerReply(proto::Factory<proto::PeerReply>(bytes), alloc);
}

auto FactoryPrivate::PeerReply(
    const opentxs::network::zeromq::Frame& bytes,
    alloc::Strategy alloc) const noexcept -> contract::peer::Reply
{
    return PeerReply(bytes.Bytes(), alloc);
}

auto FactoryPrivate::PeerReply(
    const proto::PeerReply& proto,
    alloc::Strategy alloc) const noexcept -> contract::peer::Reply
{
    const auto signerID = NymID(proto.recipient(), alloc.work_);
    auto signer = api_.Wallet().Nym(signerID);

    if (false == signer.operator bool()) { return {alloc.result_}; }

    using enum proto::PeerRequestType;

    switch (proto.type()) {
        case PEERREQUEST_BAILMENT: {

            return factory::BailmentReply(api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_OUTBAILMENT: {

            return factory::OutbailmentReply(api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_PENDINGBAILMENT: {

            return factory::BailmentNoticeReply(
                api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_CONNECTIONINFO: {

            return factory::ConnectionReply(api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_STORESECRET: {

            return factory::StoreSecretReply(api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_FAUCET: {

            return factory::FaucetReply(api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_VERIFICATION: {

            return factory::VerificationReply(
                api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_VERIFIEDCLAIM:
        default: {

            return {alloc.result_};
        }
    }
}

auto FactoryPrivate::PeerRequest(ReadView bytes, alloc::Strategy alloc)
    const noexcept -> contract::peer::Request
{
    return PeerRequest(proto::Factory<proto::PeerRequest>(bytes), alloc);
}

auto FactoryPrivate::PeerRequest(
    const opentxs::network::zeromq::Frame& bytes,
    alloc::Strategy alloc) const noexcept -> contract::peer::Request
{
    return PeerRequest(bytes.Bytes(), alloc);
}
auto FactoryPrivate::PeerRequest(
    const proto::PeerRequest& proto,
    alloc::Strategy alloc) const noexcept -> contract::peer::Request
{
    const auto signerID = NymID(proto.initiator(), alloc.work_);
    auto signer = api_.Wallet().Nym(signerID);

    if (false == signer.operator bool()) { return {alloc.result_}; }

    using enum proto::PeerRequestType;

    switch (proto.type()) {
        case PEERREQUEST_BAILMENT: {

            return factory::BailmentRequest(api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_OUTBAILMENT: {

            return factory::OutbailmentRequest(
                api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_PENDINGBAILMENT: {

            return factory::BailmentNoticeRequest(
                api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_CONNECTIONINFO: {

            return factory::ConnectionRequest(
                api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_STORESECRET: {

            return factory::StoreSecretRequest(
                api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_FAUCET: {

            return factory::FaucetRequest(api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_VERIFICATION: {

            return factory::VerificationRequest(
                api_.Self(), signer, proto, alloc);
        }
        case PEERREQUEST_VERIFIEDCLAIM:
        default: {

            return {alloc.result_};
        }
    }
}

auto FactoryPrivate::Purse(
    const otx::context::Server& context,
    const identifier::UnitDefinition& unit,
    const otx::blind::Mint& mint,
    const opentxs::Amount& totalValue,
    const otx::blind::CashType type,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return factory::Purse(api_.Self(), context, type, mint, totalValue, reason);
}

auto FactoryPrivate::Purse(
    const otx::context::Server& context,
    const identifier::UnitDefinition& unit,
    const otx::blind::Mint& mint,
    const opentxs::Amount& totalValue,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return Purse(
        context, unit, mint, totalValue, otx::blind::CashType::Lucre, reason);
}

auto FactoryPrivate::Purse(const proto::Purse& serialized) const noexcept
    -> otx::blind::Purse
{
    return factory::Purse(api_.Self(), serialized);
}

auto FactoryPrivate::Purse(
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const otx::blind::CashType type,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return factory::Purse(api_.Self(), owner, server, unit, type, reason);
}

auto FactoryPrivate::Purse(
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const opentxs::PasswordPrompt& reason) const noexcept -> otx::blind::Purse
{
    return Purse(owner, server, unit, otx::blind::CashType::Lucre, reason);
}

auto FactoryPrivate::Scriptable(const String& strInput) const
    -> std::unique_ptr<OTScriptable>
{
    std::array<char, 45> buf{};

    if (!strInput.Exists()) {
        LogError()()("Failure: Input string is empty.").Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strInput.Get());

    if (!strContract->DecodeIfArmored(
            api_.Crypto(), false))  // bEscapedIsAllowed=true
                                    // by default.
    {
        LogError()()(
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
    const bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) { return nullptr; }

    std::unique_ptr<OTScriptable> pItem;

    auto strFirstLine = String::Factory(buf.data());
    strContract->reset();  // set the "file" pointer within this string back to
                           // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -")) {
        return nullptr;

        // There are actually two factories that load smart contracts. See
        // OTCronItem.
        //
    } else if (strFirstLine->Contains(
                   "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is
                                                             // 36 chars long.
    {
        pItem.reset(new OTSmartContract(api_.Self()));
        assert_true(false != bool(pItem));
    }

    // The string didn't match any of the options in the factory.
    if (false == bool(pItem)) { return nullptr; }

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) { return pItem; }

    return nullptr;
}

auto FactoryPrivate::SecurityContract(
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const UnitType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const opentxs::Amount& redemptionIncrement) const noexcept(false)
    -> OTSecurityContract
{
    auto output = opentxs::Factory::SecurityContract(
        api_.Self(),
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

auto FactoryPrivate::SecurityContract(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTSecurityContract
{
    auto output =
        opentxs::Factory::SecurityContract(api_.Self(), nym, serialized);

    if (output) {
        return OTSecurityContract{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate currency contract");
    }
}

auto FactoryPrivate::ServerContract() const noexcept(false) -> OTServerContract
{
    return OTServerContract{opentxs::Factory::ServerContract(api_.Self())};
}

auto FactoryPrivate::SignedFile() const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_.Self()));

    return signedfile;
}

auto FactoryPrivate::SignedFile(
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME) const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_.Self(), LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}
auto FactoryPrivate::SignedFile(
    const char* LOCAL_SUBDIR,
    const String& FILE_NAME) const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_.Self(), LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

auto FactoryPrivate::SignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME)
    const -> std::unique_ptr<OTSignedFile>
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_.Self(), LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

auto FactoryPrivate::SmartContract() const -> std::unique_ptr<OTSmartContract>
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_.Self()));

    return smartcontract;
}

auto FactoryPrivate::SmartContract(const identifier::Notary& NOTARY_ID) const
    -> std::unique_ptr<OTSmartContract>
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_.Self(), NOTARY_ID));

    return smartcontract;
}

auto FactoryPrivate::StoreSecretReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    bool value,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::StoreSecret
{
    return factory::StoreSecretReply(
        api_.Self(),
        responder,
        initiator,
        inReferenceToRequest,
        value,
        reason,
        alloc);
}

auto FactoryPrivate::StoreSecretRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const contract::peer::SecretType kind,
    std::span<const std::string_view> data,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::request::StoreSecret
{
    return factory::StoreSecretRequest(
        api_.Self(), initiator, responder, kind, data, reason, alloc);
}

auto FactoryPrivate::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::crypto::symmetric::Algorithm mode,
    const opentxs::PasswordPrompt& password,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return {factory::SymmetricKey(api_.Self(), engine, mode, password, alloc)};
}

auto FactoryPrivate::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::PasswordPrompt& password,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return SymmetricKey(
        engine, opentxs::crypto::symmetric::Algorithm::Error, password, alloc);
}

auto FactoryPrivate::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const opentxs::crypto::symmetric::Source type,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return {factory::SymmetricKey(
        api_.Self(), engine, seed, operations, difficulty, size, type, alloc)};
}

auto FactoryPrivate::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t size,
    const opentxs::crypto::symmetric::Source type,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return {factory::SymmetricKey(
        api_.Self(),
        engine,
        seed,
        salt,
        operations,
        difficulty,
        parallel,
        size,
        type,
        alloc)};
}

auto FactoryPrivate::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const opentxs::Secret& raw,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return {factory::SymmetricKey(api_.Self(), engine, raw, reason, alloc)};
}

auto FactoryPrivate::SymmetricKey(
    const opentxs::crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized,
    alloc::Default alloc) const noexcept -> opentxs::crypto::symmetric::Key
{
    return {factory::SymmetricKey(api_.Self(), engine, serialized, alloc)};
}

auto FactoryPrivate::Trade() const -> std::unique_ptr<OTTrade>
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(api_.Self()));

    return trade;
}

auto FactoryPrivate::Trade(
    const identifier::Notary& notaryID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identifier::Account& assetAcctId,
    const identifier::Nym& nymID,
    const identifier::UnitDefinition& currencyId,
    const identifier::Account& currencyAcctId) const -> std::unique_ptr<OTTrade>
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(
        api_.Self(),
        notaryID,
        instrumentDefinitionID,
        assetAcctId,
        nymID,
        currencyId,
        currencyAcctId));

    return trade;
}

auto FactoryPrivate::Transaction(const String& strInput) const
    -> std::unique_ptr<OTTransactionType>
{
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed =
        DearmorAndTrim(api_.Crypto(), strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<OTTransactionType> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED TRANSACTION-----"))  // this string is 34
                                                        // chars long.
        {
            pContract.reset(new OTTransaction(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRANSACTION ITEM-----"))  // this
                                                                    // string is
                                                                    // 39 chars
                                                                    // long.
        {
            pContract.reset(new opentxs::Item(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED LEDGER-----"))  // this string is 29
                                                          // chars long.
        {
            pContract.reset(new opentxs::Ledger(api_.Self()));
            assert_true(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED ACCOUNT-----"))  // this string is 30
                                                           // chars long.
        {
            LogAbort()().Abort();
        }

        // The string didn't match any of the options in the factory.
        //

        //        const char* szFunc = "OTTransactionType::TransactionFactory";
        // The string didn't match any of the options in the factory.
        if (nullptr == pContract) {
            LogConsole()()("Object type not yet supported by class "
                           "factory: ")(strFirstLine.get())
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
            // Specifically, it happens when load_securely_ is set to false.
            //
            //          pContract->SetRealNotaryID(pItem->GetPurportedNotaryID());
            //          pContract->SetRealAccountID(pItem->GetPurportedAccountID());

            return pContract;
        } else {
            LogConsole()()("Failed loading contract from string (first "
                           "line): ")(strFirstLine.get())
                .Flush();
        }
    }

    return nullptr;
}

auto FactoryPrivate::Transaction(const opentxs::Ledger& theOwner) const
    -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(api_.Self(), theOwner));

    return transaction;
}

auto FactoryPrivate::Transaction(
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID,
    otx::originType theOriginType) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_.Self(), theNymID, theAccountID, theNotaryID, theOriginType));

    return transaction;
}

auto FactoryPrivate::Transaction(
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID,
    std::int64_t lTransactionNum,
    otx::originType theOriginType) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_.Self(),
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
auto FactoryPrivate::Transaction(
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID,
    const std::int64_t& lNumberOfOrigin,
    otx::originType theOriginType,
    const std::int64_t& lTransactionNum,
    const std::int64_t& lInRefTo,
    const std::int64_t& lInRefDisplay,
    const Time the_DATE_SIGNED,
    otx::transactionType theType,
    const String& strHash,
    const opentxs::Amount& lAdjustment,
    const opentxs::Amount& lDisplayValue,
    const std::int64_t& lClosingNum,
    const std::int64_t& lRequestNum,
    bool bReplyTransSuccess,
    NumList* pNumList) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_.Self(),
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

auto FactoryPrivate::Transaction(
    const opentxs::Ledger& theOwner,
    otx::transactionType theType,
    otx::originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const -> std::unique_ptr<OTTransaction>
{
    auto pTransaction = Transaction(
        theOwner.GetNymID(),
        theOwner.GetPurportedAccountID(),
        theOwner.GetPurportedNotaryID(),
        theType,
        theOriginType,
        lTransactionNum);
    if (false != bool(pTransaction)) { pTransaction->SetParent(theOwner); }

    return pTransaction;
}

auto FactoryPrivate::Transaction(
    const identifier::Nym& theNymID,
    const identifier::Account& theAccountID,
    const identifier::Notary& theNotaryID,
    otx::transactionType theType,
    otx::originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const -> std::unique_ptr<OTTransaction>
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_.Self(),
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));
    assert_true(false != bool(transaction));

    transaction->type_ = theType;

    // Since we're actually generating this transaction, then we can go ahead
    // and set the purported account and server IDs (we have already set the
    // real ones in the constructor). Now both sets are fill with matching data.
    // No need to security check the IDs since we are creating this transaction
    // versus loading and inspecting it.
    transaction->SetPurportedAccountID(theAccountID);
    transaction->SetPurportedNotaryID(theNotaryID);

    return transaction;
}

auto FactoryPrivate::UnitDefinition() const noexcept -> OTUnitDefinition
{
    return OTUnitDefinition{opentxs::Factory::UnitDefinition(api_.Self())};
}

auto FactoryPrivate::UnitDefinition(
    const Nym_p& nym,
    const proto::UnitDefinition serialized) const noexcept(false)
    -> OTUnitDefinition
{
    auto output =
        opentxs::Factory::UnitDefinition(api_.Self(), nym, serialized);

    if (output) {
        return OTUnitDefinition{std::move(output)};
    } else {
        throw std::runtime_error("Failed to instantiate unit definition");
    }
}

auto FactoryPrivate::UnitID(const proto::Identifier& in, alloc::Default alloc)
    const noexcept -> identifier::UnitDefinition
{
    return parent_.Internal().UnitID(in, std::move(alloc));
}

auto FactoryPrivate::UnitIDConvertSafe(
    const identifier::Generic& in,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return parent_.Internal().UnitIDConvertSafe(in, std::move(alloc));
}

auto FactoryPrivate::UnitIDFromPreimage(
    const ProtobufType& proto,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return parent_.Internal().UnitIDFromPreimage(proto, std::move(alloc));
}

auto FactoryPrivate::UnitIDFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return parent_.Internal().UnitIDFromPreimage(proto, type, std::move(alloc));
}

auto FactoryPrivate::Verification(
    const identifier::Nym& verifier,
    const opentxs::PasswordPrompt& reason,
    identity::wot::ClaimID claim,
    identity::wot::verification::Type value,
    Time start,
    Time stop,
    std::span<const identity::wot::VerificationID> superscedes,
    alloc::Strategy alloc) const noexcept -> identity::wot::Verification
{
    return factory::Verification(
        api_.Self(),
        verifier,
        reason,
        claim,
        value,
        start,
        stop,
        superscedes,
        alloc);
}

auto FactoryPrivate::Verification(
    const identity::Nym& verifier,
    const opentxs::PasswordPrompt& reason,
    identity::wot::ClaimID claim,
    identity::wot::verification::Type value,
    Time start,
    Time stop,
    std::span<const identity::wot::VerificationID> superscedes,
    alloc::Strategy alloc) const noexcept -> identity::wot::Verification
{
    return factory::Verification(
        api_.Self(),
        verifier.ID(),
        reason,
        claim,
        value,
        start,
        stop,
        superscedes,
        alloc);
}

auto FactoryPrivate::Verification(ReadView serialized, alloc::Strategy alloc)
    const noexcept -> identity::wot::Verification
{
    return Verification(proto::Factory<proto::Verification>(serialized), alloc);
}

auto FactoryPrivate::Verification(
    const identifier::Nym& verifier,
    const proto::VerificationItem& proto,
    alloc::Strategy alloc) const noexcept -> identity::wot::Verification
{
    return factory::Verification(api_.Self(), verifier, proto, alloc);
}

auto FactoryPrivate::Verification(
    const proto::Verification& proto,
    alloc::Strategy alloc) const noexcept -> identity::wot::Verification
{
    return factory::Verification(api_.Self(), proto, alloc);
}

auto FactoryPrivate::VerificationReply(
    const Nym_p& responder,
    const identifier::Nym& initiator,
    const identifier::Generic& inReferenceToRequest,
    const std::optional<identity::wot::Verification>& response,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept -> contract::peer::reply::Verification
{
    return factory::VerificationReply(
        api_.Self(),
        responder,
        initiator,
        inReferenceToRequest,
        response,
        reason,
        alloc);
}

auto FactoryPrivate::VerificationRequest(
    const Nym_p& initiator,
    const identifier::Nym& responder,
    const identity::wot::Claim& claim,
    const opentxs::PasswordPrompt& reason,
    alloc::Strategy alloc) const noexcept
    -> contract::peer::request::Verification
{
    return factory::VerificationRequest(
        api_.Self(), initiator, responder, claim, reason, alloc);
}

FactoryPrivate::~FactoryPrivate() { Detach(self_); }
}  // namespace opentxs::api::session
