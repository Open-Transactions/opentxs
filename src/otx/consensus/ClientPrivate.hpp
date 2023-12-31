// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/consensus/ConsensusPrivate.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace protobuf
{
class Context;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context
{
class ClientPrivate final : public ConsensusPrivate
{
public:
    UnallocatedSet<TransactionNumber> open_cron_items_;

    ClientPrivate(const VersionNumber targetVersion) noexcept;
    ClientPrivate(
        const api::Session& api,
        const VersionNumber targetVersion,
        const protobuf::Context& serialized) noexcept;
    ClientPrivate() = delete;
    ClientPrivate(const ClientPrivate&) = delete;
    ClientPrivate(ClientPrivate&&) = delete;
    auto operator=(const ClientPrivate&) -> ClientPrivate& = delete;
    auto operator=(ClientPrivate&&) -> ClientPrivate& = delete;

    ~ClientPrivate() final = default;
};
}  // namespace opentxs::otx::context
