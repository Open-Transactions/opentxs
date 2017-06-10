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

#ifndef OPENTXS_CORE_CRYPTO_VERIFICATIONCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_VERIFICATIONCREDENTIAL_HPP

#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Proto.hpp"

#include <memory>

namespace opentxs
{

class VerificationCredential : public Credential
{
private:
    typedef Credential ot_super;
    friend class Credential;

    std::unique_ptr<proto::VerificationSet> data_;

    serializedCredential serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;
    bool verify_internally(const Lock& lock) const override;

    VerificationCredential(
        CredentialSet& parent,
        const proto::Credential& credential);
    VerificationCredential(
        CredentialSet& parent,
        const NymParameters& nymParameters);
    VerificationCredential() = delete;
    VerificationCredential(const VerificationCredential&) = delete;
    VerificationCredential(VerificationCredential&&) = delete;
    VerificationCredential& operator=(const VerificationCredential&) = delete;
    VerificationCredential& operator=(VerificationCredential&&) = delete;

public:
    static proto::Verification SigningForm(const proto::Verification& item);
    static std::string VerificationID(const proto::Verification& item);

    bool GetVerificationSet(
        std::unique_ptr<proto::VerificationSet>& verificationSet) const override;

    virtual ~VerificationCredential() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_VERIFICATIONCREDENTIAL_HPP
