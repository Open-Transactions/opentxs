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
#include "stdafx.hpp"

#include "opentxs/core/crypto/Bip32.hpp"

#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/OT.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#define OT_METHOD "opentxs::Bip32::"

namespace opentxs
{

serializedAsymmetricKey Bip32::AccountChildKey(
    const proto::HDPath& rootPath,
    const BIP44Chain internal,
    const std::uint32_t index) const
{
    auto path = rootPath;
    auto fingerprint = rootPath.root();
    serializedAsymmetricKey output;
    std::int32_t notUsed;
    auto seed = OT::App().Crypto().BIP39().Seed(fingerprint, notUsed);
    path.set_root(fingerprint);

    if (false == bool(seed)) {
        return output;
    }

    const std::uint32_t change = internal ? 1 : 0;
    path.add_child(change);
    path.add_child(index);

    return GetHDKey(EcdsaCurve::SECP256K1, *seed, path);
}

std::string Bip32::Seed(const std::string& fingerprint) const
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    std::int32_t notUsed = 0;
    auto seed = OT::App().Crypto().BIP39().Seed(input, notUsed);

    if (!seed) {
        return "";
    }

    auto start = static_cast<const unsigned char*>(seed->getMemory());
    const auto end = start + seed->getMemorySize();

    std::vector<unsigned char> bytes(start, end);
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');

    for (int byte : bytes) {
        stream << std::setw(2) << byte;
    }

    return stream.str();
}

serializedAsymmetricKey Bip32::GetPaymentCode(
    std::string& fingerprint,
    const std::uint32_t nym) const
{
    serializedAsymmetricKey output;
    std::int32_t notUsed = 0;
    auto seed = OT::App().Crypto().BIP39().Seed(fingerprint, notUsed);

    if (!seed) {
        return output;
    }

    proto::HDPath path;
    path.set_root(fingerprint);
    path.add_child(
        static_cast<std::uint32_t>(Bip43Purpose::PAYCODE) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    path.add_child(
        static_cast<std::uint32_t>(Bip44Type::BITCOIN) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    path.add_child(nym | static_cast<std::uint32_t>(Bip32Child::HARDENED));

    output = GetHDKey(EcdsaCurve::SECP256K1, *seed, path);

    return output;
}

serializedAsymmetricKey Bip32::GetStorageKey(std::string& fingerprint) const
{
    std::int32_t notUsed = 0;
    auto seed = OT::App().Crypto().BIP39().Seed(fingerprint, notUsed);

    if (false == bool(seed)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load seed."
              << std::endl;

        return {};
    }

    proto::HDPath path;
    path.set_root(fingerprint);
    path.add_child(
        static_cast<std::uint32_t>(Bip43Purpose::FS) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    path.add_child(
        static_cast<std::uint32_t>(Bip32Child::ENCRYPT_KEY) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));

    return GetHDKey(EcdsaCurve::SECP256K1, *seed, path);
}

std::string Print(const proto::HDPath& node)
{
    std::stringstream output{};
    output << node.root();

    for (const auto& child : node.child()) {
        output << " / ";
        const auto max = static_cast<std::uint32_t>(Bip32Child::HARDENED);

        if (max > child) {
            output << std::to_string(child);
        } else {
            output << std::to_string(child - max) << "'";
        }
    }

    return output.str();
}
}  // namespace opentxs
#endif
