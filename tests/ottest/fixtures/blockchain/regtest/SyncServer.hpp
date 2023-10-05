// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "ottest/fixtures/blockchain/SyncListener.hpp"
// IWYU pragma: no_include "ottest/fixtures/blockchain/TXOState.hpp"

#pragma once

#include <opentxs/opentxs.hpp>
#include <memory>
#include <optional>

#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "ottest/fixtures/blockchain/regtest/Normal.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct SyncRequestor;
struct SyncSubscriber;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
class OPENTXS_EXPORT Regtest_fixture_sync_server : public Regtest_fixture_normal
{
protected:
    using MessageType = ot::network::otdht::MessageType;

    static const User alex_;
    static std::optional<ot::OTServerContract> notary_;
    static std::optional<ot::OTUnitDefinition> unit_;

    auto Requestor() noexcept -> SyncRequestor&;
    auto Shutdown() noexcept -> void final;
    auto Subscriber() noexcept -> SyncSubscriber&;

    Regtest_fixture_sync_server();

private:
    static bool init_sync_server_;
    static std::unique_ptr<SyncSubscriber> sync_subscriber_;
    static std::unique_ptr<SyncRequestor> sync_requestor_;
};
}  // namespace ottest
