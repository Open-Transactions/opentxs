// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/blockchain/block/Validator.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace node
{
class HeaderOracle;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class PacketCrypt final : public blockchain::block::Validator
{
public:
    auto Validate(const blockchain::bitcoin::block::Block& block) const noexcept
        -> bool final;

    PacketCrypt(const blockchain::node::HeaderOracle& oracle) noexcept;
    PacketCrypt() = delete;
    PacketCrypt(const PacketCrypt&) = delete;
    PacketCrypt(PacketCrypt&&) = delete;
    auto operator=(const PacketCrypt&) -> PacketCrypt& = delete;
    auto operator=(PacketCrypt&&) -> PacketCrypt& = delete;

    ~PacketCrypt() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::crypto::implementation
