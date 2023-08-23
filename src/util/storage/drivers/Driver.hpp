// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/storage/Driver.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::implementation
{
class Driver : virtual public storage::Driver
{
public:
    Driver() = delete;
    Driver(const Driver&) = delete;
    Driver(Driver&&) = delete;
    auto operator=(const Driver&) -> Driver& = delete;
    auto operator=(Driver&&) -> Driver& = delete;

    ~Driver() override = default;

protected:
    const api::Crypto& crypto_;
    const storage::Config& config_;

    Driver(const api::Crypto& crypto, const storage::Config& config);
};
}  // namespace opentxs::storage::implementation
