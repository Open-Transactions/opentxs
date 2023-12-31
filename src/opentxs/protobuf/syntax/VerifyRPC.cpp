// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/VerifyRPC.hpp"  // IWYU pragma: associated

namespace opentxs::protobuf::inline syntax
{
auto AddClaimAllowedContactItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 5}},
        {2, {1, 6}},
    };

    return output;
}

auto ContactEventAllowedAccountEvent() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}

auto CreateNymAllowedAddClaim() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}

auto RPCCommandAllowedAPIArgument() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedAcceptPendingPayment() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedAddClaim() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCCommandAllowedAddContact() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedCreateInstrumentDefinition() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedCreateNym() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCCommandAllowedGetWorkflow() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedHDSeed() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedModifyAccount() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedSendMessage() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedSendPayment() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedServerContract() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCCommandAllowedVerificationItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCCommandAllowedVerifyClaim() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCPushAllowedAccountEvent() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCPushAllowedContactEvent() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCPushAllowedTaskComplete() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCResponseAllowedAccountData() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCResponseAllowedAccountEvent() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCResponseAllowedContact() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {1, 3}},
        {3, {1, 3}},
    };

    return output;
}

auto RPCResponseAllowedContactEvent() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCResponseAllowedHDSeed() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCResponseAllowedNym() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 5}},
        {2, {1, 6}},
        {3, {1, 6}},
    };

    return output;
}

auto RPCResponseAllowedRPCStatus() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCResponseAllowedRPCTask() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCResponseAllowedServerContract() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCResponseAllowedSessionData() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCResponseAllowedTransactionData() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}

auto RPCResponseAllowedUnitDefinition() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {2, {1, 1}},
        {3, {1, 2}},
    };

    return output;
}

auto RPCResponseAllowedWorkflow() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}
}  // namespace opentxs::protobuf::inline syntax
