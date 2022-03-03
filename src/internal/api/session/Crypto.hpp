// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/Crypto.hpp"

#include <memory>

#include "internal/api/Crypto.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Crypto : virtual public session::Crypto,
               virtual public api::internal::Crypto
{
public:
    auto InternalSession() const noexcept -> const Crypto& final
    {
        return *this;
    }

    virtual auto Cleanup() noexcept -> void = 0;
    using api::internal::Crypto::Init;
    virtual auto Init(
        const std::shared_ptr<const crypto::Blockchain>& blockchain) noexcept
        -> void = 0;
    auto InternalSession() noexcept -> Crypto& final { return *this; }
    virtual auto PrepareShutdown() noexcept -> void = 0;

    ~Crypto() override = default;
};
}  // namespace opentxs::api::session::internal
