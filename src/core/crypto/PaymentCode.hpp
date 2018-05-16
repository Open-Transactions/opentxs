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

#ifndef OPENTXS_CORE_CRYPTO_PAYMENTCODE_IMPLEMENTATION_HPP
#define OPENTXS_CORE_CRYPTO_PAYMENTCODE_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"

namespace opentxs::implementation
{
class PaymentCode : virtual public opentxs::PaymentCode
{
public:
    bool operator==(const proto::PaymentCode& rhs) const override;

    const Identifier& ID() const override;
    const std::string asBase58() const override;
    SerializedPaymentCode Serialize() const override;
    bool VerifyInternally() const override;
    bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const override;
    bool Sign(
        const Credential& credential,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr) const override;

    ~PaymentCode();

private:
    friend opentxs::PaymentCode;

    const std::uint8_t BIP47_VERSION_BYTE{0x47};

    std::uint8_t version_{1};
    std::string seed_{""};
    std::uint32_t index_{0};
    std::shared_ptr<AsymmetricKeyEC> pubkey_{nullptr};
    std::unique_ptr<OTPassword> chain_code_{nullptr};
    bool hasBitmessage_{false};
    std::uint8_t bitmessage_version_{0};
    std::uint8_t bitmessage_stream_{0};

    PaymentCode* clone() const override;
    const OTData Pubkey() const;
    void ConstructKey(const Data& pubkey);

    explicit PaymentCode(const std::string& base58);
    explicit PaymentCode(const proto::PaymentCode& paycode);
    PaymentCode(
        const std::string& seed,
        const std::uint32_t nym,
        const std::uint8_t version,
        const bool bitmessage,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream);
    PaymentCode() = delete;
    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    PaymentCode& operator=(const PaymentCode&);
    PaymentCode& operator=(PaymentCode&&);
};
}  // namespace opentxs::implementation
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#endif  // OPENTXS_CORE_CRYPTO_PAYMENTCODE_IMPLEMENTATION_HPP
