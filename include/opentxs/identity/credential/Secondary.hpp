// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/credential/Key.hpp"

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Secondary : virtual public Key
{
public:
    Secondary(const Secondary&) = delete;
    Secondary(Secondary&&) = delete;
    auto operator=(const Secondary&) -> Secondary& = delete;
    auto operator=(Secondary&&) -> Secondary& = delete;

    ~Secondary() override = default;

protected:
    Secondary() noexcept = default;  // TODO Signable
};
}  // namespace opentxs::identity::credential
