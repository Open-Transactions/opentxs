// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Config;
}  // namespace crypto

class Settings;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::imp
{
class Config final : public internal::Config
{
public:
    auto IterationCount() const -> std::uint32_t override;
    auto SymmetricSaltSize() const -> std::uint32_t override;
    auto SymmetricKeySize() const -> std::uint32_t override;
    auto SymmetricKeySizeMax() const -> std::uint32_t override;
    auto SymmetricIvSize() const -> std::uint32_t override;
    auto SymmetricBufferSize() const -> std::uint32_t override;
    auto PublicKeysize() const -> std::uint32_t override;
    auto PublicKeysizeMax() const -> std::uint32_t override;

    Config(const api::Settings& settings) noexcept;
    Config() = delete;
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    auto operator=(const Config&) -> Config& = delete;
    auto operator=(Config&&) -> Config& = delete;

private:
    const api::Settings& config_;
    mutable std::int32_t sp_n_iteration_count_{0};
    mutable std::int32_t sp_n_symmetric_salt_size_{0};
    mutable std::int32_t sp_n_symmetric_key_size_{0};
    mutable std::int32_t sp_n_symmetric_key_size_max_{0};
    mutable std::int32_t sp_n_symmetric_iv_size_{0};
    mutable std::int32_t sp_n_symmetric_buffer_size_{0};
    mutable std::int32_t sp_n_public_keysize_{0};
    mutable std::int32_t sp_n_public_keysize_max_{0};

    auto GetSetAll() const -> bool;
    auto GetSetValue(
        const UnallocatedCString& strKeyName,
        const std::int32_t nDefaultValue,
        std::int32_t& out_nValue) const -> bool;
};
}  // namespace opentxs::api::crypto::imp
