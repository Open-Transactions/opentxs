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
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/core/crypto/Bip32.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"

#include <stdint.h>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

namespace opentxs
{

std::string Bip32::Seed(const std::string& fingerprint) const
{
    //TODO: make fingerprint non-const
    std::string input (fingerprint);
    auto seed = App::Me().Crypto().BIP39().Seed(input);

    if (!seed) { return ""; }

    auto start = static_cast<const unsigned char*>(seed->getMemory());
    const auto end = start + seed->getMemorySize();

    std::vector<unsigned char> bytes(start, end);
    std::ostringstream stream;
    stream << std::hex << std::setfill( '0' );

    for (int byte : bytes ) {
        stream << std::setw(2) << byte;
    }

    return stream.str();
}

serializedAsymmetricKey Bip32::GetPaymentCode(const uint32_t nym) const
{
    proto::HDPath path;
    path.add_child(
        static_cast<std::uint32_t>(Bip43Purpose::PAYCODE) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    path.add_child(
        static_cast<std::uint32_t>(Bip44Type::BITCOIN) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    path.add_child(
        nym |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));

    return GetHDKey(EcdsaCurve::SECP256K1, path);
}

} // namespace opentxs
# endif
