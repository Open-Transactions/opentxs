// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"  // IWYU pragma: associated

namespace opentxs::proto
{
auto AsymmetricKeyAllowedCiphertext() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto AsymmetricKeyAllowedHDPath() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto AuthorityAllowedCredential() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 3}},
        {4, {1, 4}},
        {5, {1, 5}},
        {6, {1, 6}},
    };

    return output;
}
auto AuthorityAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto ChildCredentialParametersAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto CiphertextAllowedSymmetricKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
    };

    return output;
}
auto CredentialAllowedChildParams() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto CredentialAllowedContactData() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
        {5, {5, 5}},
        {6, {6, 6}},
    };

    return output;
}
auto CredentialAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto CredentialAllowedKeyCredential() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 2}},
    };

    return output;
}
auto CredentialAllowedMasterParams() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 2}},
    };

    return output;
}
auto CredentialAllowedSignatures() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto CredentialAllowedVerificationItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto EnvelopeAllowedAsymmetricKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto EnvelopeAllowedCiphertext() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto EnvelopeAllowedTaggedKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto KeyCredentialAllowedAsymmetricKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto MasterParamsAllowedNymIDSource() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto MasterParamsAllowedSourceProof() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto NymAllowedAuthority() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 3}},
        {4, {1, 4}},
        {5, {1, 5}},
        {6, {1, 6}},
    };

    return output;
}
auto NymAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto NymAllowedNymIDSource() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 2}},
    };

    return output;
}
auto NymIDSourceAllowedAsymmetricKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto NymIDSourceAllowedPaymentCode() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 3}},
    };

    return output;
}
auto SeedAllowedCiphertext() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto SeedAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto SymmetricKeyAllowedCiphertext() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto TaggedKeyAllowedSymmetricKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
    };

    return output;
}
}  // namespace opentxs::proto
