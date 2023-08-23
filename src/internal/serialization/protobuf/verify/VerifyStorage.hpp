// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/serialization/protobuf/Basic.hpp"

namespace opentxs::proto
{
auto BlindedSeriesListAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageAccountsAllowedStorageAccountIndex() noexcept -> const VersionMap&;
auto StorageAccountsAllowedStorageIDList() noexcept -> const VersionMap&;
auto StorageAccountsAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageBip47ContextsAllowedStorageBip47ChannelList() noexcept
    -> const VersionMap&;
auto StorageBip47ContextsAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageContactsAllowedAddress() noexcept -> const VersionMap&;
auto StorageContactsAllowedList() noexcept -> const VersionMap&;
auto StorageContactsAllowedStorageContactNymIndex() noexcept
    -> const VersionMap&;
auto StorageContactsAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageCredentialAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageIssuerAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageItemHashAllowedIdentifier() noexcept -> const VersionMap&;
auto StorageItemsAllowedSymmetricKey() noexcept -> const VersionMap&;
auto StorageNotaryAllowedBlindedSeriesList() noexcept -> const VersionMap&;
auto StorageNymAllowedBlockchainAccountList() noexcept -> const VersionMap&;
auto StorageNymAllowedHDAccount() noexcept -> const VersionMap&;
auto StorageNymAllowedStorageBip47AddressIndex() noexcept -> const VersionMap&;
auto StorageNymAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageNymAllowedStoragePurse() noexcept -> const VersionMap&;
auto StorageNymListAllowedIdentifier() noexcept -> const VersionMap&;
auto StorageNymListAllowedStorageBip47NymAddressIndex() noexcept
    -> const VersionMap&;
auto StorageNymListAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StoragePaymentWorkflowsAllowedStorageItemHash() noexcept
    -> const VersionMap&;
auto StoragePaymentWorkflowsAllowedStoragePaymentWorkflowType() noexcept
    -> const VersionMap&;
auto StoragePaymentWorkflowsAllowedStorageWorkflowIndex() noexcept
    -> const VersionMap&;
auto StoragePurseAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageSeedsAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageServersAllowedStorageItemHash() noexcept -> const VersionMap&;
auto StorageThreadAllowedItem() noexcept -> const VersionMap&;
auto StorageUnitsAllowedStorageItemHash() noexcept -> const VersionMap&;
}  // namespace opentxs::proto
