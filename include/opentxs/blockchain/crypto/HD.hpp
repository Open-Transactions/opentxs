// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct HD;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT HD : virtual public Deterministic
{
public:
    OPENTXS_NO_EXPORT virtual auto InternalHD() const noexcept
        -> internal::HD& = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Standard() const noexcept -> HDProtocol = 0;

    OPENTXS_NO_EXPORT ~HD() override = default;

protected:
    HD() noexcept = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace opentxs::blockchain::crypto
