// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
class HasherPrivate;
}  // namespace crypto

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class OPENTXS_EXPORT Hasher
{
public:
    [[nodiscard]] auto operator()(ReadView data) noexcept -> bool;
    [[nodiscard]] auto operator()(Writer&& destination) noexcept -> bool;

    Hasher() noexcept;
    OPENTXS_NO_EXPORT Hasher(HasherPrivate* imp) noexcept;
    Hasher(const Hasher&) = delete;
    Hasher(Hasher&& rhs) noexcept;
    auto operator=(const Hasher&) -> Hasher& = delete;
    auto operator=(Hasher&& rhs) noexcept -> Hasher&;

    ~Hasher();

private:
    HasherPrivate* imp_;
};
}  // namespace opentxs::crypto
