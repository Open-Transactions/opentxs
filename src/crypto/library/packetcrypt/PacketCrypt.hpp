// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin
}  // namespace block

namespace client
{
class HeaderOracle;
}  // namespace client
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::implementation
{
class PacketCrypt final : public blockchain::node::internal::BlockValidator
{
public:
    using HeaderOracle = blockchain::node::HeaderOracle;

    auto Validate(const BitcoinBlock& block) const noexcept -> bool final;

    PacketCrypt(const HeaderOracle& oracle) noexcept;

    ~PacketCrypt() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    PacketCrypt() = delete;
    PacketCrypt(const PacketCrypt&) = delete;
    PacketCrypt(PacketCrypt&&) = delete;
    auto operator=(const PacketCrypt&) -> PacketCrypt& = delete;
    auto operator=(PacketCrypt&&) -> PacketCrypt& = delete;
};
}  // namespace opentxs::crypto::implementation
