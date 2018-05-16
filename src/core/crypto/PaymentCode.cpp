/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/
#include "opentxs/stdafx.hpp"

#include "PaymentCode.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#include "opentxs/core/crypto/MasterCredential.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

template class opentxs::Pimpl<opentxs::PaymentCode>;

#define OT_METHOD "opentxs::implementation::PaymentCode::"

namespace opentxs
{
OTPaymentCode PaymentCode::Factory(const PaymentCode& rhs)
{
    return OTPaymentCode(rhs.clone());
}

OTPaymentCode PaymentCode::Factory(const std::string& base58)
{
    return OTPaymentCode(new implementation::PaymentCode(base58));
}

OTPaymentCode PaymentCode::Factory(const proto::PaymentCode& serialized)
{
    return OTPaymentCode(new implementation::PaymentCode(serialized));
}

OTPaymentCode PaymentCode::Factory(
    const std::string& seed,
    const std::uint32_t nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream)
{
    return OTPaymentCode(new implementation::PaymentCode(
        seed, nym, version, bitmessage, bitmessageVersion, bitmessageStream));
}
}  // namespace opentxs

namespace opentxs::implementation
{
PaymentCode::PaymentCode(const std::string& base58)
    : version_(0)
    , seed_("")
    , index_(0)
    , pubkey_(nullptr)
    , chain_code_(new OTPassword)
    , hasBitmessage_(false)
    , bitmessage_version_(0)
    , bitmessage_stream_(0)
{
    std::string rawCode = OT::App().Crypto().Encode().IdentifierDecode(base58);

    if (81 == rawCode.size()) {
        version_ = rawCode[1];
        const std::uint8_t features = rawCode[2];

        if (features & 0x80) {
            hasBitmessage_ = true;
        }

        auto key = Data::Factory(&rawCode[3], 33);

        OT_ASSERT(chain_code_);

        chain_code_->setMemory(&rawCode[36], 32);

        ConstructKey(key);

        if (hasBitmessage_) {
            bitmessage_version_ = rawCode[68];
            bitmessage_stream_ = rawCode[69];
        }
    } else {
        otWarn << OT_METHOD << __FUNCTION__ << "Can not construct payment code."
               << std::endl
               << "Required size: 81" << std::endl
               << "Actual size: " << rawCode.size() << std::endl;
        chain_code_.reset();
    }
}

PaymentCode::PaymentCode(const proto::PaymentCode& paycode)
    : version_(paycode.version())
    , seed_("")
    , index_(0)
    , pubkey_(nullptr)
    , chain_code_(new OTPassword)
    , hasBitmessage_(paycode.has_bitmessage())
    , bitmessage_version_(0)
    , bitmessage_stream_(0)
{
    OT_ASSERT(chain_code_);

    chain_code_->setMemory(
        paycode.chaincode().c_str(), paycode.chaincode().size());

    auto key = Data::Factory(paycode.key().c_str(), paycode.key().size());
    ConstructKey(key);

    if (paycode.has_bitmessageversion()) {
        bitmessage_version_ = paycode.bitmessageversion();
    }

    if (paycode.has_bitmessagestream()) {
        bitmessage_stream_ = paycode.bitmessagestream();
    }
}

PaymentCode::PaymentCode(
    const std::string& seed,
    const std::uint32_t nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream)
    : version_(version)
    , seed_(seed)
    , index_(nym)
    , pubkey_(nullptr)
    , chain_code_(nullptr)
    , hasBitmessage_(bitmessage)
    , bitmessage_version_(bitmessageVersion)
    , bitmessage_stream_(bitmessageStream)
{
    serializedAsymmetricKey privatekey =
        OT::App().Crypto().BIP32().GetPaymentCode(seed_, index_);

    if (privatekey) {
        chain_code_.reset(new OTPassword);

        OT_ASSERT(chain_code_)

        OTPassword privkey;
        auto symmetricKey = OT::App().Crypto().Symmetric().Key(
            privatekey->encryptedkey().key(),
            privatekey->encryptedkey().mode());
        OTPasswordData password(__FUNCTION__);
        symmetricKey->Decrypt(privatekey->chaincode(), password, *chain_code_);
        proto::AsymmetricKey key;
#if OT_CRYPTO_USING_LIBSECP256K1
        const bool haveKey =
            static_cast<const Libsecp256k1&>(OT::App().Crypto().SECP256K1())
                .PrivateToPublic(*privatekey, key);
#endif

        if (haveKey) {
            auto pubkey = Data::Factory(key.key().c_str(), key.key().size());
            ConstructKey(pubkey);
        }
    }
}

bool PaymentCode::operator==(const proto::PaymentCode& rhs) const
{
    SerializedPaymentCode tempPaycode = Serialize();

    auto LHData = proto::ProtoAsData(*tempPaycode);
    auto RHData = proto::ProtoAsData(rhs);

    return (LHData == RHData);
}

PaymentCode* PaymentCode::clone() const
{
    return new PaymentCode(
        seed_,
        index_,
        version_,
        hasBitmessage_,
        bitmessage_version_,
        bitmessage_stream_);
}

const OTData PaymentCode::Pubkey() const
{
    auto pubkey = Data::Factory();
    pubkey->SetSize(33);

    if (pubkey_) {
#if OT_CRYPTO_USING_LIBSECP256K1
        std::dynamic_pointer_cast<AsymmetricKeySecp256k1>(pubkey_)->GetKey(
            pubkey);
#endif
    }

    OT_ASSERT(33 == pubkey->GetSize());

    return pubkey;
}

const std::string PaymentCode::asBase58() const
{
    if (chain_code_) {
        auto pubkey = Pubkey();
        std::array<std::uint8_t, 81> serialized{};
        serialized[0] = PaymentCode::BIP47_VERSION_BYTE;
        serialized[1] = version_;
        serialized[2] = hasBitmessage_ ? 0x80 : 0;
        OTPassword::safe_memcpy(
            &serialized[3], 33, pubkey->GetPointer(), pubkey->GetSize(), false);
        OTPassword::safe_memcpy(
            &serialized[36],
            32,
            chain_code_->getMemory(),
            chain_code_->getMemorySize(),
            false);
        serialized[68] = bitmessage_version_;
        serialized[69] = bitmessage_stream_;
        auto binaryVersion =
            Data::Factory(serialized.data(), serialized.size());

        return OT::App().Crypto().Encode().IdentifierEncode(binaryVersion);
    } else {

        return {};
    }
}

SerializedPaymentCode PaymentCode::Serialize() const
{
    SerializedPaymentCode serialized = std::make_shared<proto::PaymentCode>();
    serialized->set_version(version_);

    if (pubkey_) {
        auto pubkey = Pubkey();
        serialized->set_key(pubkey->GetPointer(), pubkey->GetSize());
    }

    if (chain_code_) {
        serialized->set_chaincode(
            chain_code_->getMemory(), chain_code_->getMemorySize());
    }
    serialized->set_bitmessageversion(bitmessage_version_);
    serialized->set_bitmessagestream(bitmessage_stream_);

    return serialized;
}

const Identifier& PaymentCode::ID() const
{

    std::uint8_t core[65]{};

    auto pubkey = Pubkey();
    OTPassword::safe_memcpy(
        &core[0], 33, pubkey->GetPointer(), pubkey->GetSize(), false);

    if (chain_code_) {
        if (chain_code_->getMemorySize() == 32) {
            OTPassword::safe_memcpy(
                &core[33],
                32,
                chain_code_->getMemory(),
                chain_code_->getMemorySize(),
                false);
        }
    }

    auto dataVersion = Data::Factory(core, sizeof(core));

    auto paymentCodeID = Identifier::Factory();

    paymentCodeID->CalculateDigest(dataVersion);

    return paymentCodeID;
}

bool PaymentCode::Verify(
    const proto::Credential& master,
    const proto::Signature& sourceSignature) const
{
    if (!proto::Validate<proto::Credential>(
            master,
            VERBOSE,
            proto::KEYMODE_PUBLIC,
            proto::CREDROLE_MASTERKEY,
            false)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Invalid master credential syntax." << std::endl;

        return false;
    }

    bool sameSource = (*this == master.masterdata().source().paymentcode());

    if (!sameSource) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Master credential was not derived from this source."
              << std::endl;

        return false;
    }

    if (!pubkey_) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Payment code is missing public key." << std::endl;

        return false;
    }

    proto::Credential copy;
    copy.CopyFrom(master);

    auto& signature = *copy.add_signature();
    signature.CopyFrom(sourceSignature);
    signature.clear_signature();

    return pubkey_->Verify(proto::ProtoAsData(copy), sourceSignature);
}

bool PaymentCode::Sign(
    const Credential& credential,
    proto::Signature& sig,
    const OTPasswordData* pPWData) const
{
    if (!pubkey_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Payment code not instantiated."
              << std::endl;

        return false;
    }

    std::string fingerprint = seed_;
    serializedAsymmetricKey privatekey =
        OT::App().Crypto().BIP32().GetPaymentCode(fingerprint, index_);

    if (fingerprint != seed_) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Specified seed could not be loaded." << std::endl;

        return false;
    }

    if (!privatekey) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to derive private key for payment code."
              << std::endl;

        return false;
    }

    auto existingKeyData = Data::Factory();
    auto compareKeyData = Data::Factory();
    proto::AsymmetricKey compareKey;
#if OT_CRYPTO_USING_LIBSECP256K1
    const bool haveKey =
        static_cast<const Libsecp256k1&>(OT::App().Crypto().SECP256K1())
            .PrivateToPublic(*privatekey, compareKey);
#endif

    if (!haveKey) {
        return false;
    }

    compareKey.clear_path();
    pubkey_->GetKey(existingKeyData);
    compareKeyData->Assign(compareKey.key().c_str(), compareKey.key().size());

    if (!(existingKeyData == compareKeyData)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Private key is not valid for this payment code."
              << std::endl;

        return false;
    }

    std::unique_ptr<OTAsymmetricKey> signingKey(
        OTAsymmetricKey::KeyFactory(*privatekey));

    serializedCredential serialized =
        credential.Serialized(AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();
    signature.set_role(proto::SIGROLE_NYMIDSOURCE);

    bool goodSig =
        signingKey->SignProto(*serialized, signature, String(ID()), pPWData);

    sig.CopyFrom(signature);

    return goodSig;
}

void PaymentCode::ConstructKey(const opentxs::Data& pubkey)
{
    proto::AsymmetricKey newKey;
    newKey.set_version(1);
    newKey.set_type(proto::AKEYTYPE_SECP256K1);
    newKey.set_mode(proto::KEYMODE_PUBLIC);
    newKey.set_role(proto::KEYROLE_SIGN);
    newKey.set_key(pubkey.GetPointer(), pubkey.GetSize());
    AsymmetricKeyEC* key = dynamic_cast<AsymmetricKeySecp256k1*>(
        OTAsymmetricKey::KeyFactory(newKey));

    if (nullptr != key) {
        pubkey_.reset(key);
    }
}

bool PaymentCode::VerifyInternally() const
{
    return (proto::Validate<proto::PaymentCode>(*Serialize(), SILENT));
}

PaymentCode::~PaymentCode() {}
}  // namespace opentxs::implementation
#endif
