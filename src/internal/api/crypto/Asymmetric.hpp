// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Asymmetric.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::internal
{
class Asymmetric : virtual public api::crypto::Asymmetric
{
public:
    virtual auto API() const noexcept -> const api::Session& = 0;
    virtual auto InstantiateECKey(
        const proto::AsymmetricKey& serialized) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::EllipticCurve> = 0;
    virtual auto InstantiateHDKey(const proto::AsymmetricKey& serialized)
        const noexcept -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    using crypto::Asymmetric::InstantiateKey;
    virtual auto InstantiateKey(const proto::AsymmetricKey& serialized)
        const noexcept -> std::unique_ptr<opentxs::crypto::key::Asymmetric> = 0;
    auto Internal() const noexcept -> const Asymmetric& final { return *this; }

    auto Internal() noexcept -> Asymmetric& final { return *this; }

    ~Asymmetric() override = default;
};
}  // namespace opentxs::api::crypto::internal
