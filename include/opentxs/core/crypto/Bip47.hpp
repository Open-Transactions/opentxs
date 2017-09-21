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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_CRYPTO_BIP47_HPP
#define OPENTXS_CRYPTO_BIP47_HPP

#if OT_CRYPTO_USING_LIBBITCOIN

#include <cstdint>
#include <memory>
#include <vector>

#include <bitcoin/bitcoin/utility/data.hpp>

namespace opentxs
{
class Data;

/** Extract the designated public key from a BIP-47 notification transaction.
 *
 *  This function parses the inputs of the input transaction in index order
 *  for an eligible input script.
 *
 *  An eligible input contains an input script (scriptSig) which spends a
 *  previous output script (scriptPubKey) of one of the following
 *  [standard
 * forms](https://bitcoin.org/en/developer-guide#standard-transactions):
 *
 *      - Pubkey
 *      - Pay to pubkey hash (P2PKH)
 *      - Multisig
 *      - P2SH redeeming a pubkey script
 *      - P2SH redeeming a P2PKH script
 *      - P2SH redeeming a multisig script
 *
 *  The public key (first public key for a multisig script) from the eligible
 *  input is extracted from the transaction and returned.
 *
 *  \param[in] transaction A serialized bitcoin (or equivalent) transaction
 *
 *  \returns A smart pointer to the extracted public key. The smart pointer
 *           is initialized to nullptr if the input transaction does not contain
 *           a valid input.
 *           If instantiated, the output will be 33 bytes for a compressed
 *           pubkey, or 64 bytes for an uncompressed pubkey.
 */
std::unique_ptr<Data> DesignatedPubkey(
    const Data& transaction,
    const std::vector<Data>& previous_transactions);

namespace bip47
{
const libbitcoin::data_chunk ReadData(const Data& d);
}

}  // namespace opentxs

#endif  // OT_CRYPTO_USING_LIBBITCOIN
#endif  // OPENTXS_CRYPTO_BIP47_HPP
