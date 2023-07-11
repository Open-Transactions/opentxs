// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/serialization/protobuf/Basic.hpp"

namespace opentxs::proto
{
auto AddClaimAllowedContactItem() noexcept -> const VersionMap&;
auto ContactEventAllowedAccountEvent() noexcept -> const VersionMap&;
auto CreateNymAllowedAddClaim() noexcept -> const VersionMap&;
auto RPCCommandAllowedAPIArgument() noexcept -> const VersionMap&;
auto RPCCommandAllowedAcceptPendingPayment() noexcept -> const VersionMap&;
auto RPCCommandAllowedAddClaim() noexcept -> const VersionMap&;
auto RPCCommandAllowedAddContact() noexcept -> const VersionMap&;
auto RPCCommandAllowedCreateInstrumentDefinition() noexcept
    -> const VersionMap&;
auto RPCCommandAllowedCreateNym() noexcept -> const VersionMap&;
auto RPCCommandAllowedGetWorkflow() noexcept -> const VersionMap&;
auto RPCCommandAllowedHDSeed() noexcept -> const VersionMap&;
auto RPCCommandAllowedModifyAccount() noexcept -> const VersionMap&;
auto RPCCommandAllowedSendMessage() noexcept -> const VersionMap&;
auto RPCCommandAllowedSendPayment() noexcept -> const VersionMap&;
auto RPCCommandAllowedServerContract() noexcept -> const VersionMap&;
auto RPCCommandAllowedVerificationItem() noexcept -> const VersionMap&;
auto RPCCommandAllowedVerifyClaim() noexcept -> const VersionMap&;
auto RPCPushAllowedAccountEvent() noexcept -> const VersionMap&;
auto RPCPushAllowedContactEvent() noexcept -> const VersionMap&;
auto RPCPushAllowedTaskComplete() noexcept -> const VersionMap&;
auto RPCResponseAllowedAccountData() noexcept -> const VersionMap&;
auto RPCResponseAllowedAccountEvent() noexcept -> const VersionMap&;
auto RPCResponseAllowedContact() noexcept -> const VersionMap&;
auto RPCResponseAllowedContactEvent() noexcept -> const VersionMap&;
auto RPCResponseAllowedHDSeed() noexcept -> const VersionMap&;
auto RPCResponseAllowedNym() noexcept -> const VersionMap&;
auto RPCResponseAllowedRPCStatus() noexcept -> const VersionMap&;
auto RPCResponseAllowedRPCTask() noexcept -> const VersionMap&;
auto RPCResponseAllowedServerContract() noexcept -> const VersionMap&;
auto RPCResponseAllowedSessionData() noexcept -> const VersionMap&;
auto RPCResponseAllowedTransactionData() noexcept -> const VersionMap&;
auto RPCResponseAllowedUnitDefinition() noexcept -> const VersionMap&;
auto RPCResponseAllowedWorkflow() noexcept -> const VersionMap&;
}  // namespace opentxs::proto
