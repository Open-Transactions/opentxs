// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/core/identifier/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class IdentifierPrivate;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Identifier;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using namespace std::literals;

static constexpr auto identifier_prefix_ = "ot"sv;
}  // namespace opentxs

namespace opentxs::identifier::internal
{
class Identifier
{
public:
    virtual auto Get() const noexcept -> const IdentifierPrivate& = 0;
    virtual auto Serialize(proto::Identifier& out) const noexcept -> bool = 0;
    virtual auto Serialize(network::zeromq::Message& out) const noexcept
        -> bool = 0;

    Identifier(const Identifier&) = delete;
    Identifier(Identifier&&) = delete;
    auto operator=(const Identifier&) noexcept -> Identifier& = delete;
    auto operator=(Identifier&&) noexcept -> Identifier& = delete;

    virtual ~Identifier() = default;

protected:
    Identifier() = default;
};
}  // namespace opentxs::identifier::internal
