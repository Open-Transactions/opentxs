// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/ContractType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/Types.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class OPENTXS_EXPORT PublishContract final : public Base
{
public:
    class Imp;

    auto ID() const noexcept -> const Identifier&;
    auto Payload() const noexcept -> ReadView;
    auto ContractType() const noexcept -> contract::Type;

    OPENTXS_NO_EXPORT PublishContract(Imp* imp) noexcept;

    ~PublishContract() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop

    PublishContract(const PublishContract&) = delete;
    PublishContract(PublishContract&&) = delete;
    auto operator=(const PublishContract&) -> PublishContract& = delete;
    auto operator=(PublishContract&&) -> PublishContract& = delete;
};
}  // namespace opentxs::network::p2p
