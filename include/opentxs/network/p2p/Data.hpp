// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/p2p/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace p2p
{
class State;
}  // namespace p2p
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class OPENTXS_EXPORT Data final : public Base
{
public:
    class Imp;

    auto Blocks() const noexcept -> const SyncData&;
    auto LastPosition(const api::Session& api) const noexcept
        -> opentxs::blockchain::block::Position;
    auto PreviousCfheader() const noexcept -> ReadView;
    auto State() const noexcept -> const p2p::State&;

    OPENTXS_NO_EXPORT auto Add(ReadView data) noexcept -> bool;

    OPENTXS_NO_EXPORT Data(Imp* imp) noexcept;

    ~Data() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop

    Data(const Data&) = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;
};
}  // namespace opentxs::network::p2p
