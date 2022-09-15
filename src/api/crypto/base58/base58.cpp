// Copyright (c) 2014 The Bitcoin developers
// Copyright (c) 2014-2022 The Open-Transactions developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"  // IWYU pragma: associated

#include <cassert>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

// NOLINTBEGIN(modernize-loop-convert)
namespace bitcoin_base58
{
/* All alphanumeric characters except for "0", "I", "O", and "l" */
static const char* pszBase58 =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

auto DecodeBase58(const char* psz, std::vector<unsigned char>& vch) -> bool
{
    if (nullptr == psz) { return false; }

    // Skip leading spaces.
    while (*psz && isspace(*psz)) { psz++; }
    // Skip and count leading '1's.
    int zeroes = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    std::vector<unsigned char> b256(
        strlen(psz) * 733 / 1000 + 1);  // log(58) / log(256), rounded up.
    // Process the characters.
    while (*psz && !isspace(*psz)) {
        // Decode base58 character
        const char* ch = strchr(pszBase58, *psz);
        if (ch == nullptr) { return false; }
        // Apply "b256 = b256 * 58 + ch".
        auto carry = static_cast<int>(ch - pszBase58);
        for (auto it = b256.rbegin(); it != b256.rend(); it++) {
            carry += static_cast<int>(58 * (*it));
            *it = static_cast<unsigned char>(carry % 256);
            carry /= 256;
        }
        assert(carry == 0);
        psz++;
    }
    // Skip trailing spaces.
    while (isspace(*psz)) { psz++; }
    if (*psz != 0) { return false; }
    // Skip leading zeroes in b256.
    auto it = b256.begin();
    while (it != b256.end() && *it == 0) { it++; }
    // Copy result into output vector.
    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end()) { vch.push_back(*(it++)); }
    return true;
}

auto EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
    -> std::string
{
    if ((nullptr == pbegin) || (nullptr == pend)) { return {}; }

    // Skip & count leading zeroes.
    int zeroes = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    std::vector<unsigned char> b58(
        (pend - pbegin) * 138 / 100 + 1);  // log(256) / log(58), rounded up.
    // Process the bytes.
    while (pbegin != pend) {
        int carry = *pbegin;
        // Apply "b58 = b58 * 256 + ch".
        for (auto it = b58.rbegin(); it != b58.rend(); it++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }
        assert(carry == 0);
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    auto it = b58.begin();
    while (it != b58.end() && *it == 0) { it++; }
    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end()) { str += pszBase58[*(it++)]; }
    return str;
}
}  // namespace bitcoin_base58
// NOLINTEND(modernize-loop-convert)
