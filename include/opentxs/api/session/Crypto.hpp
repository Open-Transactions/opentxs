// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/api/crypto/Crypto.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Blockchain;
class Seed;
class Symmetric;
}  // namespace crypto

namespace session
{
namespace internal
{
class Crypto;
}  // namespace internal
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class OPENTXS_EXPORT Crypto : virtual public api::Crypto
{
public:
    virtual auto Asymmetric() const noexcept -> const crypto::Asymmetric& = 0;
    virtual auto Blockchain() const noexcept -> const crypto::Blockchain& = 0;
    OPENTXS_NO_EXPORT virtual auto InternalSession() const noexcept
        -> const internal::Crypto& = 0;
    virtual auto Seed() const noexcept -> const crypto::Seed& = 0;
    virtual auto Symmetric() const noexcept -> const crypto::Symmetric& = 0;

    OPENTXS_NO_EXPORT virtual auto InternalSession() noexcept
        -> internal::Crypto& = 0;

    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    auto operator=(const Crypto&) -> Crypto& = delete;
    auto operator=(Crypto&&) -> Crypto& = delete;

    OPENTXS_NO_EXPORT ~Crypto() override = default;

protected:
    Crypto() = default;
};
}  // namespace opentxs::api::session
