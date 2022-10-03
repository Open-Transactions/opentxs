// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace otdht
{
class State;
}  // namespace otdht
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT Data final : public Base
{
public:
    class Imp;

    auto Blocks() const noexcept -> const SyncData&;
    auto FirstPosition(const api::Session& api) const noexcept
        -> opentxs::blockchain::block::Position;
    auto LastPosition(const api::Session& api) const noexcept
        -> opentxs::blockchain::block::Position;
    auto PreviousCfheader() const noexcept -> ReadView;
    auto State() const noexcept -> const otdht::State&;

    OPENTXS_NO_EXPORT auto Add(ReadView data) noexcept -> bool;

    OPENTXS_NO_EXPORT Data(Imp* imp) noexcept;
    Data(const Data&) = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;

    ~Data() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
