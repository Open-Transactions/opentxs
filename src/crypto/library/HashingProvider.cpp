// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/library/HashingProvider.hpp"  // IWYU pragma: associated

#include "internal/core/String.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Hasher.hpp"
#include "opentxs/crypto/Types.hpp"

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
    } else if (inputString.Compare("X11")) {
        return crypto::HashType::X11;
    } else if (inputString.Compare("KECCAK256")) {
        return crypto::HashType::Keccak256;
    }

    return crypto::HashType::Error;
}
auto HashingProvider::HashTypeToString(const crypto::HashType hashType) noexcept
    -> OTString

{
    auto hashTypeString = String::Factory();
    using enum crypto::HashType;

    switch (hashType) {
        case None: {
            hashTypeString = String::Factory("NULL");
        } break;
        case Sha256: {
            hashTypeString = String::Factory("SHA256");
        } break;
        case Sha512: {
            hashTypeString = String::Factory("SHA512");
        } break;
        case Blake2b160: {
            hashTypeString = String::Factory("BLAKE2B160");
        } break;
        case Blake2b256: {
            hashTypeString = String::Factory("BLAKE2B256");
        } break;
        case Blake2b512: {
            hashTypeString = String::Factory("BLAKE2B512");
        } break;
        case X11: {
            hashTypeString = String::Factory("X11");
        } break;
        case Keccak256: {
            hashTypeString = String::Factory("KECCAK256");
        } break;
        case Error:
        case Ripemd160:
        case Sha1:
        case Sha256D:
        case Sha256DC:
        case Bitcoin:
        case SipHash24:
        case Ethereum:
        default: {
            hashTypeString = String::Factory("ERROR");
        }
    }

    return hashTypeString;
}

auto HashingProvider::HashSize(const crypto::HashType hashType) noexcept
    -> std::size_t
{
    using enum crypto::HashType;

    switch (hashType) {
        case Sha256: {
            return 32;
        }
        case Sha512: {
            return 64;
        }
        case Blake2b160: {
            return 20;
        }
        case Blake2b256: {
            return 32;
        }
        case Blake2b512: {
            return 64;
        }
        case Ripemd160: {
            return 20;
        }
        case Sha1: {
            return 20;
        }
        case Sha256D: {
            return 32;
        }
        case Sha256DC: {
            return 4;
        }
        case Bitcoin: {
            return 20;
        }
        case SipHash24: {
            return 8;
        }
        case X11: {
            return 32;
        }
        case Keccak256: {
            return 32;
        }
        case Ethereum: {
            return 20;
        }
        case Error:
        case None:
        default: {

            return 0;
        }
    }
}

auto HashingProvider::Hasher(const crypto::HashType) const noexcept
    -> opentxs::crypto::Hasher
{
    return {};
}
}  // namespace opentxs::crypto
