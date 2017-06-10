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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOENCODINGENGINE_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOENCODINGENGINE_HPP

#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace opentxs
{

class CryptoEncoding;
class CryptoEngine;
class Data;
class OTPassword;

class CryptoEncodingEngine
{
private:
    friend class CryptoEngine;

    CryptoEncoding& base58_;

    std::string IdentifierEncode(const OTPassword& input) const;

protected:
    static const std::uint8_t LineWidth{72};

    std::string Base64Encode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const;
    bool Base64Decode(
        const std::string&& input,
         RawData& output) const;
    std::string BreakLines(const std::string& input) const;

    CryptoEncodingEngine() = delete;
    CryptoEncodingEngine(CryptoEngine& parent);
    CryptoEncodingEngine(const CryptoEncodingEngine&) = delete;
    CryptoEncodingEngine& operator=(const CryptoEncodingEngine&) = delete;


public:
    static std::string SanatizeBase58(const std::string& input);
    static std::string SanatizeBase64(const std::string& input);

    std::string DataEncode(const std::string& input) const;
    std::string DataEncode(const Data& input) const;
    std::string DataDecode(const std::string& input) const;
    std::string IdentifierEncode(const Data& input) const;
    std::string IdentifierDecode(const std::string& input) const;
    bool IsBase62(const std::string& str) const;
    String Nonce(const uint32_t size) const;
    String Nonce(const uint32_t size, Data& rawOutput) const;
    std::string RandomFilename() const;

    ~CryptoEncodingEngine() = default;
};
} // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_CRYPTOENCODINGENGINE_HPP
