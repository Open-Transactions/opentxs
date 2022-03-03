// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <string_view>

#include "internal/api/FactoryAPI.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Crypto;
class Factory;
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
class Factory final : public internal::Factory
{
public:
    auto Secret(const std::size_t bytes) const noexcept -> OTSecret final;
    auto SecretFromBytes(const ReadView bytes) const noexcept -> OTSecret final;
    auto SecretFromText(const std::string_view text) const noexcept
        -> OTSecret final;

    Factory(const api::Crypto& crypto) noexcept;

    ~Factory() final = default;

private:
    const api::Crypto& crypto_;

    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;
};
}  // namespace opentxs::api::imp
