// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/identity/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace internal
{
class Source;
}  // namespace internal
}  // namespace identity
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity
{
class OPENTXS_EXPORT Source
{
public:
    virtual auto Internal() const noexcept -> const internal::Source& = 0;
    virtual auto Type() const noexcept -> identity::SourceType = 0;
    virtual auto NymID() const noexcept -> identifier::Nym = 0;

    virtual auto Internal() noexcept -> internal::Source& = 0;

    Source(const Source&) = delete;
    Source(Source&&) = delete;
    auto operator=(const Source&) -> Source& = delete;
    auto operator=(Source&&) -> Source& = delete;

    virtual ~Source() = default;

protected:
    Source() = default;
};
}  // namespace opentxs::identity
