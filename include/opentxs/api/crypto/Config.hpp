// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/ParameterType.hpp"
// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/key/symmetric/Algorithm.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
namespace internal
{
class Config;
}  // namespace internal
}  // namespace crypto
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
auto HaveHDKeys() noexcept -> bool;
auto HaveSupport(opentxs::crypto::ParameterType) noexcept -> bool;
auto HaveSupport(opentxs::crypto::key::asymmetric::Algorithm) noexcept -> bool;
auto HaveSupport(opentxs::crypto::key::symmetric::Algorithm) noexcept -> bool;

class OPENTXS_EXPORT Config
{
public:
    OPENTXS_NO_EXPORT virtual auto InternalConfig() const noexcept
        -> const internal::Config& = 0;
    virtual auto IterationCount() const -> std::uint32_t = 0;
    virtual auto SymmetricSaltSize() const -> std::uint32_t = 0;
    virtual auto SymmetricKeySize() const -> std::uint32_t = 0;
    virtual auto SymmetricKeySizeMax() const -> std::uint32_t = 0;
    virtual auto SymmetricIvSize() const -> std::uint32_t = 0;
    virtual auto SymmetricBufferSize() const -> std::uint32_t = 0;
    virtual auto PublicKeysize() const -> std::uint32_t = 0;
    virtual auto PublicKeysizeMax() const -> std::uint32_t = 0;

    OPENTXS_NO_EXPORT virtual auto InternalConfig() noexcept
        -> internal::Config& = 0;

    OPENTXS_NO_EXPORT virtual ~Config() = default;

protected:
    Config() = default;

private:
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    auto operator=(const Config&) -> Config& = delete;
    auto operator=(Config&&) -> Config& = delete;
};
}  // namespace opentxs::api::crypto
