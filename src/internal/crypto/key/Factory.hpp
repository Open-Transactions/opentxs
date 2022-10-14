// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"

#pragma once

#include "opentxs/crypto/asymmetric/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Keypair;
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Keypair() noexcept -> std::unique_ptr<crypto::key::Keypair>;
auto Keypair(
    const api::Session& api,
    const opentxs::crypto::asymmetric::Role role,
    crypto::asymmetric::Key publicKey,
    crypto::asymmetric::Key privateKey) noexcept(false)
    -> std::unique_ptr<crypto::key::Keypair>;
}  // namespace opentxs::factory
