// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "blockchain/database/wallet/SubchainID.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>

#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::database::wallet::db
{
SubchainID::SubchainID(
    const crypto::Subchain type,
    const filter::Type filter,
    const VersionNumber version,
    const Identifier& subaccount) noexcept
    : data_([&] {
        auto out = space(fixed_ + subaccount.size());
        auto* it = reinterpret_cast<std::byte*>(out.data());

        std::memcpy(it, &type, sizeof(type));
        std::advance(it, sizeof(type));
        std::memcpy(it, &filter, sizeof(filter));
        std::advance(it, sizeof(filter));
        std::memcpy(it, &version, sizeof(version));
        std::advance(it, sizeof(version));

        if (0u < subaccount.size()) {
            std::memcpy(it, subaccount.data(), subaccount.size());
        }

        return out;
    }())
    , lock_()
    , subchain_(type)
    , filter_(filter)
    , version_(version)
    , subaccount_(subaccount)
{
    static_assert(9u == fixed_);
}

SubchainID::SubchainID(const ReadView bytes) noexcept(false)
    : data_(space(bytes))
    , lock_()
    , subchain_()
    , filter_()
    , version_()
    , subaccount_()
{
    if (data_.size() <= fixed_) {
        throw std::runtime_error{"Input byte range too small for SubchainID"};
    }
}

auto SubchainID::FilterType() const noexcept -> filter::Type
{
    auto lock = Lock{lock_};

    if (false == filter_.has_value()) {
        auto type = filter::Type{};
        static constexpr auto offset = sizeof(crypto::Subchain);
        static constexpr auto size = sizeof(type);
        const auto start = std::next(data_.data(), offset);
        std::memcpy(&type, start, size);
        filter_ = type;
    }

    return filter_.value();
}

auto SubchainID::SubaccountID(const api::Session& api) const noexcept
    -> const Identifier&
{
    auto lock = Lock{lock_};

    if (false == subaccount_.has_value()) {
        static constexpr auto offset = fixed_;
        const auto size = data_.size() - offset;
        const auto start = std::next(data_.data(), offset);
        auto& id = subaccount_.emplace(api.Factory().Identifier());

        if (0u < size) { id->Assign(start, size); }
    }

    return subaccount_.value();
}
auto SubchainID::Type() const noexcept -> crypto::Subchain
{
    auto lock = Lock{lock_};

    if (false == subchain_.has_value()) {
        auto type = crypto::Subchain{};
        static constexpr auto offset = std::size_t{0};
        static constexpr auto size = sizeof(type);
        const auto start = std::next(data_.data(), offset);
        std::memcpy(&type, start, size);
        subchain_ = type;
    }

    return subchain_.value();
}

auto SubchainID::Version() const noexcept -> VersionNumber
{
    auto lock = Lock{lock_};

    if (false == version_.has_value()) {
        auto type = VersionNumber{};
        static constexpr auto offset =
            sizeof(crypto::Subchain) + sizeof(filter::Type);
        static constexpr auto size = sizeof(type);
        const auto start = std::next(data_.data(), offset);
        std::memcpy(&type, start, size);
        version_ = type;
    }

    return version_.value();
}
}  // namespace opentxs::blockchain::database::wallet::db
