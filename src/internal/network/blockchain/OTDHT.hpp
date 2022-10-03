// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain
{
class OTDHT
{
public:
    class Actor;

    auto Init() noexcept -> void;

    OTDHT(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> node)
    noexcept;
    OTDHT() = delete;
    OTDHT(const OTDHT&) = delete;
    OTDHT(OTDHT&&) = delete;
    auto operator=(const OTDHT&) -> OTDHT& = delete;
    auto operator=(OTDHT&&) -> OTDHT& = delete;

    ~OTDHT();
};
}  // namespace opentxs::network::blockchain
