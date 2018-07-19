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

#ifndef OPENTXS_API_CRYPTO_BIP47_IMPLEMENTATION_HPP
#define OPENTXS_API_CRYPTO_BIP47_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47

#include "opentxs/api/crypto/Bip47.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include <trezor-crypto/bip32.h>

namespace opentxs::api::crypto::implementation
{
class Bip47 : virtual public crypto::Bip47
{
public:
    Bip44AccountSource AccountSource(const Nym& local, const proto::ContactItemType chain)
        const override;
    std::tuple<bool, OTPassword&> LocalPaymentCode(
        const Bip44AccountSource& local,
        const std::uint32_t& index) const override;
    std::tuple<bool, OTData> RemotePaymentCode(
        const PaymentCode& remote,
        const std::uint32_t& index) const override;
    std::tuple<bool, OTPassword&> HashSecret(
        const OTPassword* secret) const override;
    std::tuple<bool, OTPassword&> SecretPoint(
        const OTPassword& privkey,
        const OTData pubkey) const override;
    proto::AsymmetricKey IncomingPubkey(
        const Nym& local,
        const PaymentCode& remote,
        const proto::ContactItemType chain,
        const std::uint32_t index) const override;
    proto::AsymmetricKey OutgoingPubkey(
        const Nym& local,
        const PaymentCode& remote,
        const proto::ContactItemType chain,
        const std::uint32_t index) const override;
    std::string NotificationAddress(
        const Nym& local,
        proto::ContactItemType chain) const override;
    virtual ~Bip47() = default;

protected:
    Bip47() = default;

    virtual bool AddSecp256k1(const Data& P, Data& Q) const = 0;    

    virtual bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const = 0;

    virtual std::shared_ptr<proto::AsymmetricKey> HDNodeToSerialized(
        const proto::AsymmetricKeyType& type,
        const HDNode& node,
        const bool privateVersion) const = 0;

    virtual bool IsSecp256k1(OTPassword& P) const = 0;

    virtual std::shared_ptr<proto::AsymmetricKey> GetHDKey(
        const EcdsaCurve& curve,
        const OTPassword& seed,
        proto::HDPath& path) const = 0;

    virtual bool ScalarBaseMultiply(
        const OTPassword& privateKey,
        Data& publicKey) const = 0;
    virtual bool ValidPrivateKey(const OTPassword& key) const = 0;

private:
    std::shared_ptr<proto::AsymmetricKey> get_privkey(
        std::string& fingerprint,
        const std::uint32_t coin,
        const std::uint32_t nym,
        const std::uint32_t index,
        const bool hardened_index) const;

    std::shared_ptr<proto::AsymmetricKey> get_xpubkey_child(
        const PaymentCode& remote,
        const std::uint32_t index) const;
  
    Bip47(const Bip47&) = delete;
    Bip47(Bip47&&) = delete;
    Bip47& operator=(const Bip47&) = delete;
    Bip47& operator=(Bip47&&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#endif  // OPENTXS_API_CRYPTO_BIP47_IMPLEMENTATION_HPP
