// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/serialization/protobuf/Basic.hpp"

namespace opentxs::proto
{
auto Bip47ChainAllowedBip47Channel() noexcept -> const VersionMap&;
auto Bip47ChannelAllowedBip47Direction() noexcept -> const VersionMap&;
auto Bip47ChannelAllowedBlockchainDeterministicAccountData() noexcept
    -> const VersionMap&;
auto Bip47ChannelAllowedPaymentCode() noexcept -> const VersionMap&;
auto Bip47ContextAllowedBip47Chain() noexcept -> const VersionMap&;
auto Bip47DirectionAllowedBlockchainAddress() noexcept -> const VersionMap&;
auto BlockchainAccountDataAllowedBlockchainActivity() noexcept
    -> const VersionMap&;
auto BlockchainAddressAllowedAsymmetricKey() noexcept -> const VersionMap&;
auto BlockchainBlockHeaderAllowedBitcoinBlockHeaderFields() noexcept
    -> const VersionMap&;
auto BlockchainBlockHeaderAllowedBlockchainBlockLocalData() noexcept
    -> const VersionMap&;
auto BlockchainBlockHeaderAllowedEthereumBlockHeaderFields() noexcept
    -> const VersionMap&;
auto BlockchainDeterministicAccountDataAllowedBlockchainAccountData() noexcept
    -> const VersionMap&;
auto BlockchainDeterministicAccountDataAllowedHDPath() noexcept
    -> const VersionMap&;
auto BlockchainTransactionAllowedInput() noexcept -> const VersionMap&;
auto BlockchainTransactionAllowedOutput() noexcept -> const VersionMap&;
auto BlockchainTransactionInputAllowedBlockchainInputWitness() noexcept
    -> const VersionMap&;
auto BlockchainTransactionInputAllowedBlockchainPreviousOutput() noexcept
    -> const VersionMap&;
auto BlockchainTransactionInputAllowedBlockchainTransactionOutput() noexcept
    -> const VersionMap&;
auto BlockchainTransactionInputAllowedBlockchainWalletKey() noexcept
    -> const VersionMap&;
auto BlockchainTransactionOutputAllowedBlockchainWalletKey() noexcept
    -> const VersionMap&;
auto BlockchainTransactionProposalAllowedBlockchainTransaction() noexcept
    -> const VersionMap&;
auto BlockchainTransactionProposalAllowedBlockchainTransactionProposedNotification() noexcept
    -> const VersionMap&;
auto BlockchainTransactionProposalAllowedBlockchainTransactionProposedOutput() noexcept
    -> const VersionMap&;
auto BlockchainTransactionProposedNotificationAllowedHDPath() noexcept
    -> const VersionMap&;
auto BlockchainTransactionProposedNotificationAllowedPaymentCode() noexcept
    -> const VersionMap&;
auto BlockchainTransactionProposedOutputAllowedBlockchainOutputMultisigDetails() noexcept
    -> const VersionMap&;
auto HDAccountAllowedBlockchainAddress() noexcept -> const VersionMap&;
auto HDAccountAllowedBlockchainDeterministicAccountData() noexcept
    -> const VersionMap&;
auto HDAccountAllowedBlockchainHDAccountData() noexcept -> const VersionMap&;
}  // namespace opentxs::proto
