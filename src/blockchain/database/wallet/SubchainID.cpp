// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/wallet/SubchainID.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>

#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::blockchain::database::wallet::db
{
SubchainID::SubchainID(
    const crypto::Subchain type,
    const cfilter::Type filter,
    const VersionNumber version,
    const identifier::Generic& subaccount) noexcept
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

auto SubchainID::FilterType() const noexcept -> cfilter::Type
{
    auto lock = Lock{lock_};

    if (false == filter_.has_value()) {
        auto type = cfilter::Type{};
        static constexpr auto offset = sizeof(crypto::Subchain);
        static constexpr auto size = sizeof(type);
        const auto* const start = std::next(data_.data(), offset);
        std::memcpy(&type, start, size);
        filter_ = type;
    }

    return filter_.value();
}

auto SubchainID::SubaccountID(const api::Session& api) const noexcept
    -> const identifier::Generic&
{
    auto lock = Lock{lock_};

    if (false == subaccount_.has_value()) {
        static constexpr auto offset = fixed_;
        const auto size = data_.size() - offset;
        const auto* const start = std::next(data_.data(), offset);
        auto& id = subaccount_.emplace(identifier::Generic{});

        if (0u < size) { id.Assign(start, size); }
    }

    return subaccount_.value();
}
auto SubchainID::Type() const noexcept -> crypto::Subchain
{
    auto lock = Lock{lock_};

    if (false == subchain_.has_value()) {
        auto type = crypto::Subchain{};
        static constexpr auto offset = 0_uz;
        static constexpr auto size = sizeof(type);
        const auto* const start = std::next(data_.data(), offset);
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
            sizeof(crypto::Subchain) + sizeof(cfilter::Type);
        static constexpr auto size = sizeof(type);
        const auto* const start = std::next(data_.data(), offset);
        std::memcpy(&type, start, size);
        version_ = type;
    }

    return version_.value();
}
}  // namespace opentxs::blockchain::database::wallet::db
