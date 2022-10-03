// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "opentxs/crypto/library/HashingProvider.hpp"  // IWYU pragma: associated

#include "opentxs/core/String.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::crypto
{
auto HashingProvider::StringToHashType(const String& inputString) noexcept
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
auto HashingProvider::HashTypeToString(const crypto::HashType hashType) noexcept
    -> OTString

{
    auto hashTypeString = String::Factory();
    using Type = crypto::HashType;

    switch (hashType) {
        case Type::None: {
            hashTypeString = String::Factory("NULL");
        } break;
        case Type::Sha256: {
            hashTypeString = String::Factory("SHA256");
        } break;
        case Type::Sha512: {
            hashTypeString = String::Factory("SHA512");
        } break;
        case Type::Blake2b160: {
            hashTypeString = String::Factory("BLAKE2B160");
        } break;
        case Type::Blake2b256: {
            hashTypeString = String::Factory("BLAKE2B256");
        } break;
        case Type::Blake2b512: {
            hashTypeString = String::Factory("BLAKE2B512");
        } break;
        case Type::Error:
        case Type::Ripemd160:
        case Type::Sha1:
        case Type::Sha256D:
        case Type::Sha256DC:
        case Type::Bitcoin:
        case Type::SipHash24:
        default: {
            hashTypeString = String::Factory("ERROR");
        }
    }

    return hashTypeString;
}

auto HashingProvider::HashSize(const crypto::HashType hashType) noexcept
    -> std::size_t
{
    using Type = crypto::HashType;

    switch (hashType) {
        case Type::Sha256: {
            return 32;
        }
        case Type::Sha512: {
            return 64;
        }
        case Type::Blake2b160: {
            return 20;
        }
        case Type::Blake2b256: {
            return 32;
        }
        case Type::Blake2b512: {
            return 64;
        }
        case Type::Ripemd160: {
            return 20;
        }
        case Type::Sha1: {
            return 20;
        }
        case Type::Sha256D: {
            return 32;
        }
        case Type::Sha256DC: {
            return 4;
        }
        case Type::Bitcoin: {
            return 20;
        }
        case Type::SipHash24: {
            return 8;
        }
        case Type::Error:
        case Type::None:
        default: {

            return 0;
        }
    }
}
}  // namespace opentxs::crypto
