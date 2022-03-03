// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network
{
class OPENTXS_EXPORT OpenDHT
{
public:
    virtual void Insert(
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        DhtDoneCallback cb = {}) const noexcept = 0;
    virtual void Retrieve(
        const UnallocatedCString& key,
        DhtResultsCallback vcb,
        DhtDoneCallback dcb = {}) const noexcept = 0;

    virtual ~OpenDHT() = default;

protected:
    OpenDHT() = default;

private:
    OpenDHT(const OpenDHT&) = delete;
    OpenDHT(OpenDHT&&) = delete;
    auto operator=(const OpenDHT&) -> OpenDHT& = delete;
    auto operator=(OpenDHT&&) -> OpenDHT& = delete;
};
}  // namespace opentxs::network
