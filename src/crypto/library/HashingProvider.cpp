// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/crypto/library/HashingProvider.hpp"  // IWYU pragma: associated

#include "opentxs/core/String.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::crypto
{
auto HashingProvider::StringToHashType(const String& inputString)
    -> crypto::HashType
{
    if (inputString.Compare("NULL")) {
        return crypto::HashType::None;
    } else if (inputString.Compare("SHA256")) {
        return crypto::HashType::Sha256;
    } else if (inputString.Compare("SHA512")) {
        return crypto::HashType::Sha512;
    } else if (inputString.Compare("BLAKE2B160")) {
        return crypto::HashType::Blake2b160;
    } else if (inputString.Compare("BLAKE2B256")) {
        return crypto::HashType::Blake2b256;
    } else if (inputString.Compare("BLAKE2B512")) {
        return crypto::HashType::Blake2b512;
    }

    return crypto::HashType::Error;
}
auto HashingProvider::HashTypeToString(const crypto::HashType hashType)
    -> OTString

{
    auto hashTypeString = String::Factory();

    switch (hashType) {
        case crypto::HashType::None: {
            hashTypeString = String::Factory("NULL");
        } break;
        case crypto::HashType::Sha256: {
            hashTypeString = String::Factory("SHA256");
        } break;
        case crypto::HashType::Sha512: {
            hashTypeString = String::Factory("SHA512");
        } break;
        case crypto::HashType::Blake2b160: {
            hashTypeString = String::Factory("BLAKE2B160");
        } break;
        case crypto::HashType::Blake2b256: {
            hashTypeString = String::Factory("BLAKE2B256");
        } break;
        case crypto::HashType::Blake2b512: {
            hashTypeString = String::Factory("BLAKE2B512");
        } break;
        default: {
            hashTypeString = String::Factory("ERROR");
        }
    }

    return hashTypeString;
}

auto HashingProvider::HashSize(const crypto::HashType hashType) -> std::size_t
{
    switch (hashType) {
        case crypto::HashType::Sha256: {
            return 32;
        }
        case crypto::HashType::Sha512: {
            return 64;
        }
        case crypto::HashType::Blake2b160: {
            return 20;
        }
        case crypto::HashType::Blake2b256: {
            return 32;
        }
        case crypto::HashType::Blake2b512: {
            return 64;
        }
        case crypto::HashType::Ripemd160: {
            return 20;
        }
        case crypto::HashType::Sha1: {
            return 20;
        }
        case crypto::HashType::Sha256D: {
            return 32;
        }
        case crypto::HashType::Sha256DC: {
            return 4;
        }
        case crypto::HashType::Bitcoin: {
            return 20;
        }
        case crypto::HashType::SipHash24: {
            return 8;
        }
        default: {
        }
    }

    return 0;
}
}  // namespace opentxs::crypto
