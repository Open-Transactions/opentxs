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

#ifndef OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP
#define OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/core/crypto/OTPassword.hpp"
#endif
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <string>
#include <memory>

namespace opentxs
{

class OTPassword;

class NymParameters
{
private:
    proto::SourceType sourceType_{proto::SOURCETYPE_PUBKEY};
    proto::SourceProofType sourceProofType_{
        proto::SOURCEPROOFTYPE_SELF_SIGNATURE};
    std::shared_ptr<proto::ContactData> contact_data_;
    std::shared_ptr<proto::VerificationSet> verification_set_;

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    NymParameterType nymType_{NymParameterType::ED25519};
#elif OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    NymParameterType nymType_{NymParameterType::SECP256K1};
#elif OT_CRYPTO_SUPPORTED_KEY_RSA
    NymParameterType nymType_{NymParameterType::RSA};
#else
    NymParameterType nymType_{NymParameterType::ERROR};
#endif

#if OT_CRYPTO_SUPPORTED_KEY_HD
    proto::CredentialType credentialType_{proto::CREDTYPE_HD};
    std::unique_ptr<OTPassword> entropy_;
    std::string seed_;
    std::int32_t nym_{0};
    std::uint32_t credset_{0};
    std::uint32_t cred_index_{0};
    bool default_{true};
#else
    proto::CredentialType credentialType_{proto::CREDTYPE_LEGACY};
#endif

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    std::int32_t nBits_{1024};
#endif

public:
    explicit NymParameters(proto::CredentialType theCredentialtype);
    NymParameters() = default;
    NymParameters(const NymParameters& rhs);

    NymParameterType nymParameterType();

    proto::AsymmetricKeyType AsymmetricKeyType() const;

    void setNymParameterType(NymParameterType theKeytype);

    proto::CredentialType credentialType() const;

    void setCredentialType(proto::CredentialType theCredentialtype);

    inline proto::SourceType SourceType() const { return sourceType_; }

    inline void SetSourceType(proto::SourceType sType) { sourceType_ = sType; }

    inline proto::SourceProofType SourceProofType() const
    {
        return sourceProofType_;
    }

    inline void SetSourceProofType(proto::SourceProofType sType)
    {
        sourceProofType_ = sType;
    }

    inline std::shared_ptr<proto::ContactData> ContactData() const
    {
        return contact_data_;
    }

    inline std::shared_ptr<proto::VerificationSet> VerificationSet() const
    {
        return verification_set_;
    }

    void SetContactData(const proto::ContactData& contactData);
    void SetVerificationSet(const proto::VerificationSet& verificationSet);

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    std::int32_t keySize();

    void setKeySize(std::int32_t keySize);
    explicit NymParameters(const std::int32_t keySize);
#endif

#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::unique_ptr<OTPassword>& Entropy() const;
    void SetEntropy(const OTPassword& entropy);

    inline std::string Seed() const { return seed_; }
    inline void SetSeed(const std::string& seed) { seed_ = seed; }

    inline std::int32_t Nym() const { return nym_; }
    inline void SetNym(const std::int32_t path) { nym_ = path; }

    inline std::uint32_t Credset() const { return credset_; }
    inline void SetCredset(const std::uint32_t path) { credset_ = path; }

    inline std::uint32_t CredIndex() const { return cred_index_; }
    inline void SetCredIndex(const std::uint32_t path) { cred_index_ = path; }

    inline bool Default() const { return default_; }
    inline void SetDefault(const bool in) { default_ = in; }
#endif

    ~NymParameters() = default;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP
