// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/crypto/Bip32.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Factory;
}  // namespace api

namespace crypto
{
namespace asymmetric
{
namespace key
{
class HDPrivate;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::internal
{
struct Bip32 {
    virtual auto DerivePrivateKey(
        const asymmetric::key::HDPrivate& parent,
        const crypto::Bip32::Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false)
        -> crypto::Bip32::Key = 0;
    virtual auto DerivePublicKey(
        const asymmetric::key::HDPrivate& parent,
        const crypto::Bip32::Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false)
        -> crypto::Bip32::Key = 0;

    virtual auto Init(
        const std::shared_ptr<const api::Factory>& factory) noexcept
        -> void = 0;

    virtual ~Bip32() = default;
};
}  // namespace opentxs::crypto::internal
