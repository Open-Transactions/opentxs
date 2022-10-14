// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
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
class OPENTXS_EXPORT Ed25519 : public HD
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Ed25519&;

    OPENTXS_NO_EXPORT Ed25519(KeyPrivate* imp) noexcept;
    Ed25519(allocator_type alloc = {}) noexcept;
    Ed25519(const Ed25519& rhs, allocator_type alloc = {}) noexcept;
    Ed25519(Ed25519&& rhs) noexcept;
    Ed25519(Ed25519&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Ed25519& rhs) noexcept -> Ed25519&;
    auto operator=(Ed25519&& rhs) noexcept -> Ed25519&;

    ~Ed25519() override;
};
}  // namespace opentxs::crypto::asymmetric::key
