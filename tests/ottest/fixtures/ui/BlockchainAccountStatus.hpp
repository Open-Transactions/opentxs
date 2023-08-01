// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT BlockchainSubchainData {
    ot::UnallocatedCString name_;
    ot::blockchain::crypto::Subchain type_;
};

struct OPENTXS_EXPORT BlockchainSubaccountData {
    ot::UnallocatedCString name_;
    ot::UnallocatedCString id_;
    ot::UnallocatedVector<BlockchainSubchainData> rows_;
};

struct OPENTXS_EXPORT BlockchainSubaccountSourceData {
    ot::UnallocatedCString name_;
    ot::identifier::Generic id_;
    ot::blockchain::crypto::SubaccountType type_;
    ot::UnallocatedVector<BlockchainSubaccountData> rows_;
};

struct OPENTXS_EXPORT BlockchainAccountStatusData {
    ot::UnallocatedCString owner_;
    ot::blockchain::Type chain_;
    ot::UnallocatedVector<BlockchainSubaccountSourceData> rows_;
};

OPENTXS_EXPORT auto check_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_blockchain_account_status_qt(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;
OPENTXS_EXPORT auto init_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    Counter& counter) noexcept -> void;
}  // namespace ottest
