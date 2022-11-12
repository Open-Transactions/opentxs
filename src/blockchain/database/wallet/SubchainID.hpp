// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain

#pragma once

#include <mutex>
#include <optional>

#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet::db
{
class SubchainID
{
public:
    const Space data_;

    auto FilterType() const noexcept -> cfilter::Type;
    auto SubaccountID(const api::Session& api) const noexcept
        -> const identifier::Generic&;
    auto Type() const noexcept -> crypto::Subchain;
    auto Version() const noexcept -> VersionNumber;

    SubchainID(
        const crypto::Subchain type,
        const cfilter::Type filter,
        const VersionNumber version,
        const identifier::Generic& subaccount) noexcept;
    SubchainID(const ReadView bytes) noexcept(false);
    SubchainID() = delete;
    SubchainID(const SubchainID&) = delete;
    SubchainID(SubchainID&&) = delete;
    auto operator=(const SubchainID&) -> SubchainID& = delete;
    auto operator=(SubchainID&&) -> SubchainID& = delete;

    ~SubchainID() = default;

private:
    static constexpr auto fixed_ = sizeof(crypto::Subchain) +
                                   sizeof(cfilter::Type) +
                                   sizeof(VersionNumber);

    mutable std::mutex lock_;
    mutable std::optional<crypto::Subchain> subchain_;
    mutable std::optional<cfilter::Type> filter_;
    mutable std::optional<VersionNumber> version_;
    mutable std::optional<identifier::Generic> subaccount_;
};
}  // namespace opentxs::blockchain::database::wallet::db
