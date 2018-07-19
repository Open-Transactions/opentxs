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

#ifndef OPENTXS_API_CRYPTO_BIP47_HPP
#define OPENTXS_API_CRYPTO_BIP47_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47

extern "C" {
#include <trezor-crypto/base58.h>
#include <trezor-crypto/ripemd160.h>
#include <trezor-crypto/bip32.h>
#include <trezor-crypto/bip39.h>
#include <trezor-crypto/ecdsa.h>
}

namespace opentxs
{
namespace api
{
namespace crypto
{
using Bip44AccountSource =
    std::tuple<bool, std::uint32_t, std::uint32_t, std::string>;
/* success, nym's identity, coin type, fingerprint */

class Bip47
{
public:
    /** 
     *  Finds the BIP-44 information for deriving BIP-32 key paths
     *
     *  \usage     The local nym must already exist in the wallet and the local
     *             payment code must be one for which the private keys are
     *             available.
     * 
     *  \param[in] local             The local nym who owns the private keys to a payment code
     *  \param[in] chain             The chain to identiy the coin type
     * 
     *  \returns a tuple consisting of elements: success, identity, coin type and seed fingerprint
     */
    EXPORT virtual Bip44AccountSource AccountSource(
        const Nym& local,
        const proto::ContactItemType chain) const = 0;
    /** 
     *  Extracts a private key for ECDH using the m/47'/nym_identity'/coin_type'/index path.
     *
     *  \usage     Use the return value of AccountSource as the first parameter
     *             and provide an unused index to obtain the private key parameter
     *             useful for ECDH.
     * 
     *  \param[in] local             The tuple returned by AccountSource consisting of:
     *                               success, identity, coin type, fingerprint
     *  \param[in] index             The index to be used to generate all ephemeral keypairs
     * 
     *  \returns a tuple consisting of elements: success, private key
     */
    EXPORT virtual std::tuple<bool, OTPassword&> LocalPaymentCode(
        const Bip44AccountSource& local,
        const std::uint32_t& index) const = 0;
    /** 
     *  An extended public key for ECDH associated with a particular identity/account/index
     *
     *  \usage     Use the return value of AccountSource as the first parameter
     *             and provider an unused index to obtain the public key parameter
     *             useful for ECDH.
     * 
     *  \param[in] remote            The instantiated PaymentCode of a nym's contact
     *  \param[in] index             The index to be used to generate all ephemeral keypairs
     * 
     *  \returns a tuple consisting of 2 elements: success, 33-byte public key
     */
    EXPORT virtual std::tuple<bool, OTData> RemotePaymentCode(
        const PaymentCode& remote,
        const std::uint32_t& index) const = 0;
    /** 
     *  Hash the x coordinate of a shared secret point S
     *
     *  \usage     Use the SecretPoint to get a shared secret scalar Sx     
     *             and use this function to hash it using Sha256
     * 
     *  \param[in] secret            An scalar of 32 bytes returned by SecretPoint
     * 
     *  \returns a tuple consisting of 2 elements: success, 32-byte scalar
     */
    EXPORT virtual std::tuple<bool, OTPassword&> HashSecret(
        const OTPassword* secret) const = 0;
    /** 
     *  Derives a unique shared secret point using ECDH. 
     *
     *  \usage     Use the output of LocalPaymentCode and RemotePaymentCode
     *             to get the private and public key parameters
     * 
     *  \param[in] privKey           An scalar of 32 bytes returned by LocalPaymentCode
     *  \param[in] pubkey            A 33-byte public key returned by RemotePaymentCode
     * 
     *  \returns a tuple consisting of 2 elements: success, 32-byte scalar representing
     *  the x coordinate of the secret point. 
     */
    EXPORT virtual std::tuple<bool, OTPassword&> SecretPoint(
        const OTPassword& privkey,
        const OTData pubkey) const = 0;
    /** Calculate public keys to generate a look ahead of deposit addresses.
     *  
     *  \usage     Call this method to get a key associated
     *             with the ith deposit by remote to local.
     *
     *  \param[in] local             The local nym who owns the receive address
     *  \param[in] remote            A payment code belonging to the payer
     *  \param[in] chain             The blockchain in which the addresses will exist
     *  \param[in] index             The index associated with the next incoming deposit
     * 
     *  \returns a key corresponding to a secp256k1 public key
     */ 
    EXPORT virtual proto::AsymmetricKey IncomingPubkey(
        const Nym& local,
        const PaymentCode& remote,
        const proto::ContactItemType chain,
        const std::uint32_t index) const = 0;  
    /** Calculate public keys to generate payment addresses.
     *  
     *  \usage     Call this method to get a key associated
     *             to the remote's deposit address and 
     *             ith payment by local to remote.
     *
     *  \param[in] local             The local nym who will send a payment to a contact
     *  \param[in] remote            A payment code belonging to the payee
     *  \param[in] chain             The blockchain in which the addresses will exist
     *  \param[in] index             The index associated with the next outgoing deposit
     */
    EXPORT virtual proto::AsymmetricKey OutgoingPubkey(
        const Nym& local,
        const PaymentCode& remote,
        const proto::ContactItemType chain,
        const std::uint32_t index) const = 0;
    /** Obtain the P2PKH address associated with the 0th public key derived from a local 
     * payment code.
     *  
     *  \usage     The nym owning a payment code with associated private keys
     *             generates a BIP-47 notification addresss for a one time notification
     *             transsaction.
     *
     *  \param[in] local             The local nym who owns the notification address
     *  \param[in] chain             A blockchain in which the notification transactions
     *  are broadcasted
     */
    EXPORT virtual std::string NotificationAddress(
        const Nym& local,
        proto::ContactItemType chain) const = 0;
    EXPORT virtual ~Bip47() = default;

protected:
    Bip47() = default;

private:
    Bip47(const Bip47&) = delete;
    Bip47(Bip47&&) = delete;
    Bip47& operator=(const Bip47&) = delete;
    Bip47& operator=(Bip47&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#endif  // OPENTXS_API_CRYPTO_BIP47_HPP
