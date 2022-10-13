// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
namespace internal
{
class Symmetric;
}  // namespace internal
}  // namespace crypto
}  // namespace api

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
/**
 The api::crypto::Symmetric API contains functions specific to symmetric keys.
 (AES etc).
 */
class OPENTXS_EXPORT Symmetric
{
public:
    OPENTXS_NO_EXPORT virtual auto InternalSymmetric() const noexcept
        -> const internal::Symmetric& = 0;
    virtual auto IvSize(const opentxs::crypto::symmetric::Algorithm mode)
        const noexcept -> std::size_t = 0;
    virtual auto Key(
        opentxs::crypto::symmetric::Algorithm mode,
        const PasswordPrompt& password,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Key(const PasswordPrompt& password, alloc::Default alloc = {})
        const noexcept -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Key(
        ReadView serializedCiphertext,
        opentxs::crypto::symmetric::Algorithm mode,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Key(
        const Secret& seed,
        const opentxs::crypto::symmetric::Algorithm mode,
        const opentxs::crypto::symmetric::Source type,
        const std::uint64_t operations = 0u,
        const std::uint64_t difficulty = 0u,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Key(
        const Secret& seed,
        const opentxs::crypto::symmetric::Source type,
        const std::uint64_t operations = 0u,
        const std::uint64_t difficulty = 0u,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Key(
        const Secret& seed,
        const opentxs::crypto::symmetric::Algorithm mode,
        const std::uint64_t operations = 0u,
        const std::uint64_t difficulty = 0u,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Key(
        const Secret& seed,
        const std::uint64_t operations = 0u,
        const std::uint64_t difficulty = 0u,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Key(
        const Secret& seed,
        ReadView salt,
        std::uint64_t operations,
        std::uint64_t difficulty,
        std::uint64_t parallel,
        std::size_t bytes,
        opentxs::crypto::symmetric::Source type,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;

    OPENTXS_NO_EXPORT virtual auto InternalSymmetric() noexcept
        -> internal::Symmetric& = 0;

    Symmetric(const Symmetric&) = delete;
    Symmetric(Symmetric&&) = delete;
    auto operator=(const Symmetric&) -> Symmetric& = delete;
    auto operator=(Symmetric&&) -> Symmetric& = delete;

    OPENTXS_NO_EXPORT virtual ~Symmetric() = default;

protected:
    Symmetric() = default;
};
}  // namespace opentxs::api::crypto
