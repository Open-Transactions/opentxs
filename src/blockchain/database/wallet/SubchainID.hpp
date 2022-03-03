// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <optional>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet::db
{
class SubchainID
{
public:
    const Space data_;

    auto FilterType() const noexcept -> filter::Type;
    auto SubaccountID(const api::Session& api) const noexcept
        -> const Identifier&;
    auto Type() const noexcept -> crypto::Subchain;
    auto Version() const noexcept -> VersionNumber;

    SubchainID(
        const crypto::Subchain type,
        const filter::Type filter,
        const VersionNumber version,
        const Identifier& subaccount) noexcept;
    SubchainID(const ReadView bytes) noexcept(false);

    ~SubchainID() = default;

private:
    static constexpr auto fixed_ =
        sizeof(crypto::Subchain) + sizeof(filter::Type) + sizeof(VersionNumber);

    mutable std::mutex lock_;
    mutable std::optional<crypto::Subchain> subchain_;
    mutable std::optional<filter::Type> filter_;
    mutable std::optional<VersionNumber> version_;
    mutable std::optional<OTIdentifier> subaccount_;

    SubchainID() = delete;
    SubchainID(const SubchainID&) = delete;
    SubchainID(SubchainID&&) = delete;
    auto operator=(const SubchainID&) -> SubchainID& = delete;
    auto operator=(SubchainID&&) -> SubchainID& = delete;
};
}  // namespace opentxs::blockchain::database::wallet::db
