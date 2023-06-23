// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"  // IWYU pragma: associated

namespace opentxs::proto
{
auto FaucetReplyAllowedBlockchainTransaction() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
    };

    return output;
}
auto PeerObjectAllowedNym() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 3}},
        {2, {1, 3}},
        {3, {1, 3}},
        {4, {1, 4}},
        {5, {1, 4}},
        {6, {1, 5}},
        {7, {1, 6}},
    };

    return output;
}
auto PeerObjectAllowedPeerReply() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
        {5, {4, 5}},
        {6, {4, 5}},
        {7, {4, 5}},
    };

    return output;
}
auto PeerObjectAllowedPeerRequest() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
        {5, {4, 5}},
        {6, {4, 6}},
        {7, {4, 6}},
    };

    return output;
}
auto PeerObjectAllowedPurse() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {7, {1, 1}},
    };

    return output;
}
auto PeerReplyAllowedBailment() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerReplyAllowedConnectionInfo() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerReplyAllowedFaucetReply() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerReplyAllowedNotice() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
        {5, {4, 5}},
    };

    return output;
}
auto PeerReplyAllowedOutBailment() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerReplyAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
    };

    return output;
}
auto PeerRequestAllowedBailment() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerRequestAllowedConnectionInfo() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerRequestAllowedFaucet() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {0, 0}},
        {2, {0, 0}},
        {3, {0, 0}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerRequestAllowedOutBailment() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
        {5, {5, 5}},
    };

    return output;
}
auto PeerRequestAllowedPendingBailment() noexcept -> const VersionMap&
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
auto PeerRequestAllowedSignature() noexcept -> const VersionMap&
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
auto PeerRequestAllowedStoreSecret() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
auto PeerRequestAllowedVerificationOffer() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {0, 0}},
        {2, {0, 0}},
        {3, {3, 3}},
        {4, {4, 4}},
    };

    return output;
}
}  // namespace opentxs::proto
