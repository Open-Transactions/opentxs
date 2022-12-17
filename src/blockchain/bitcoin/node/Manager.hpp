// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "blockchain/node/manager/Manager.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Header;
}  // namespace block

namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database

namespace node
{
namespace internal
{
struct Config;
}  // namespace internal

class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::base
{
class Bitcoin final : public node::implementation::Base
{
public:
    Bitcoin(
        const api::Session& api,
        const Type type,
        const internal::Config& config,
        std::string_view seednode);
    Bitcoin() = delete;
    Bitcoin(const Bitcoin&) = delete;
    Bitcoin(Bitcoin&&) = delete;
    auto operator=(const Bitcoin&) -> Bitcoin& = delete;
    auto operator=(Bitcoin&&) -> Bitcoin& = delete;

    ~Bitcoin() final;

private:
    using ot_super = node::implementation::Base;
};
}  // namespace opentxs::blockchain::node::base
