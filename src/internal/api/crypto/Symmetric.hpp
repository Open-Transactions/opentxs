// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Symmetric.hpp"

#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace proto
{
class SymmetricKey;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::internal
{
class Symmetric : virtual public api::crypto::Symmetric
{
public:
    auto InternalSymmetric() const noexcept -> const Symmetric& final
    {
        return *this;
    }

    using crypto::Symmetric::Key;
    virtual auto Key(
        const proto::SymmetricKey& serialized,
        const opentxs::crypto::symmetric::Algorithm mode,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;

    auto InternalSymmetric() noexcept -> Symmetric& final { return *this; }

    ~Symmetric() override = default;
};
}  // namespace opentxs::api::crypto::internal
