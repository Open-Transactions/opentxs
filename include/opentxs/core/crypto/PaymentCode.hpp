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

#ifndef OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
#define OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Proto.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{

typedef std::shared_ptr<proto::PaymentCode> SerializedPaymentCode;

class AsymmetricKeyEC;
class Credential;
class MasterCredential;
class OTPassword;
class OTPasswordData;

class PaymentCode
{
private:
    const std::uint8_t BIP47_VERSION_BYTE{0x47};

    std::uint8_t version_{1};
    std::string seed_;
    std::uint32_t index_{0};
    std::shared_ptr<AsymmetricKeyEC> pubkey_;
    std::unique_ptr<OTPassword> chain_code_;
    bool hasBitmessage_{false};
    std::uint8_t bitmessage_version_{0};
    std::uint8_t bitmessage_stream_{0};

    const Data Pubkey() const;
    void ConstructKey(const Data& pubkey);
    PaymentCode() = delete;

public:
    explicit PaymentCode(const std::string& base58);
    explicit PaymentCode(const proto::PaymentCode& paycode);
    PaymentCode(
        const std::string& seed,
        const std::uint32_t nym,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0);

    bool operator==(const proto::PaymentCode& rhs) const;

    const Identifier ID() const;
    const std::string asBase58() const;
    SerializedPaymentCode Serialize() const;
    bool VerifyInternally() const;
    bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const;
    bool Sign(
        const Credential& credential,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr) const;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
