// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/credential/Key.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Primary : virtual public identity::credential::Key
{
public:
    OPENTXS_NO_EXPORT virtual auto Path(proto::HDPath& output) const
        -> bool = 0;
    virtual auto Path() const -> UnallocatedCString = 0;

    Primary(const Primary&) = delete;
    Primary(Primary&&) = delete;
    auto operator=(const Primary&) -> Primary& = delete;
    auto operator=(Primary&&) -> Primary& = delete;

    ~Primary() override = default;

protected:
    Primary() noexcept = default;  // TODO Signable
};
}  // namespace opentxs::identity::credential
