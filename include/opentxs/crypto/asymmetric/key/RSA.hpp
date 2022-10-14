// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
class KeyPrivate;
}  // namespace asymmetric
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key
{
class OPENTXS_EXPORT RSA : public Key
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> RSA&;

    OPENTXS_NO_EXPORT RSA(KeyPrivate* imp) noexcept;
    RSA(allocator_type alloc = {}) noexcept;
    RSA(const RSA& rhs, allocator_type alloc = {}) noexcept;
    RSA(RSA&& rhs) noexcept;
    RSA(RSA&& rhs, allocator_type alloc) noexcept;
    auto operator=(const RSA& rhs) noexcept -> RSA&;
    auto operator=(RSA&& rhs) noexcept -> RSA&;

    ~RSA() override;
};
}  // namespace opentxs::crypto::asymmetric::key
