// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
class Subaccount;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT HD : public Deterministic
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> HD&;

    auto Standard() const noexcept -> HDProtocol;

    OPENTXS_NO_EXPORT HD(std::shared_ptr<internal::Subaccount> imp) noexcept;
    HD() = delete;
    HD(const HD& rhs) noexcept;
    HD(HD&& rhs) noexcept;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;

    ~HD() override;
};
}  // namespace opentxs::blockchain::crypto
