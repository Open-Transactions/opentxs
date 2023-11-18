// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerifyStorage.hpp"  // IWYU pragma: associated

namespace opentxs::proto
{
auto BlindedSeriesListAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
    };

    return output;
}
auto StorageAccountsAllowedStorageAccountIndex() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto StorageAccountsAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
    };

    return output;
}
auto StorageAccountsAllowedStorageIDList() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto StorageBip47ContextsAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{{1, {1, 2}}};

    return output;
}
auto StorageBip47ContextsAllowedStorageBip47ChannelList() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{{1, {1, 1}}};

    return output;
}
auto StorageContactsAllowedAddress() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto StorageContactsAllowedList() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto StorageContactsAllowedStorageContactNymIndex() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{
        {2, {1, 1}},
    };

    return output;
}
auto StorageContactsAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {1, 2}},
    };

    return output;
}
auto StorageCredentialAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto StorageIssuerAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
    };

    return output;
}
auto StorageItemHashAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
        {7, {1, 1}},
        {8, {1, 1}},
        {9, {1, 1}},
    };

    return output;
}
auto StorageItemsAllowedSymmetricKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {6, {1, 2}},
    };

    return output;
}
auto StorageNotaryAllowedBlindedSeriesList() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto StorageNymAllowedBlockchainAccountList() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
        {7, {1, 1}},
        {8, {1, 1}},
        {9, {1, 1}},
        {10, {1, 1}},
    };

    return output;
}
auto StorageNymAllowedHDAccount() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
        {7, {1, 1}},
        {8, {1, 1}},
        {9, {1, 1}},
        {10, {1, 1}},
    };

    return output;
}
auto StorageNymAllowedBlockchainEthereumAccountData() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{
        {10, {1, 1}},
    };

    return output;
}
auto StorageNymAllowedStorageBip47AddressIndex() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {7, {1, 1}},
        {8, {1, 1}},
        {9, {1, 1}},
        {10, {1, 1}},
    };

    return output;
}
auto StorageNymAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {2, 3}},
        {4, {2, 4}},
        {5, {2, 5}},
        {6, {2, 6}},
        {7, {2, 7}},
        {8, {2, 8}},
        {9, {2, 9}},
        {10, {2, 9}},
    };

    return output;
}
auto StorageNymAllowedStoragePurse() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {8, {1, 1}},
        {9, {1, 1}},
        {10, {1, 1}},
    };

    return output;
}
auto StorageNymListAllowedStorageBip47NymAddressIndex() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{
        {4, {1, 1}},
        {5, {1, 1}},
    };

    return output;
}
auto StorageNymListAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {5, {1, 1}},
    };

    return output;
}
auto StorageNymListAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 3}},
        {4, {1, 4}},
        {5, {1, 5}},
    };

    return output;
}
auto StoragePaymentWorkflowsAllowedStorageItemHash() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
        {2, {1, 2}},
        {3, {1, 2}},
    };

    return output;
}
auto StoragePaymentWorkflowsAllowedStoragePaymentWorkflowType() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {3, 3}},
    };

    return output;
}
auto StoragePaymentWorkflowsAllowedStorageWorkflowIndex() noexcept
    -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
    };

    return output;
}
auto StoragePurseAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
    };

    return output;
}
auto StorageSeedsAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto StorageServersAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto StorageThreadAllowedItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto StorageUnitsAllowedStorageItemHash() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
}  // namespace opentxs::proto
