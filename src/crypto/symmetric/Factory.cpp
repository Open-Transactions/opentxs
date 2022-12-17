// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/symmetric/Factory.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "crypto/symmetric/KeyPrivate.hpp"
#include "internal/crypto/library/SymmetricProvider.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/SymmetricKey.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const crypto::symmetric::Algorithm mode,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) noexcept -> crypto::symmetric::KeyPrivate*
{
    try {
        using ReturnType = crypto::symmetric::implementation::Key;
        // TODO c++20
        auto pmr = alloc::PMR<ReturnType>{alloc};
        auto* output = pmr.allocate(1_uz);

        if (nullptr == output) {

            throw std::runtime_error{"failed to allocate key"};
        }

        pmr.construct(output, api, engine);
        auto& key = *output;
        const auto realMode{
            mode == opentxs::crypto::symmetric::Algorithm::Error
                ? engine.DefaultMode()
                : mode};

        if (false == key.Derive(realMode, reason)) {

            throw std::runtime_error{"failed to derive key"};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();
        // TODO c++20
        auto pmr = alloc::PMR<crypto::symmetric::KeyPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);

        OT_ASSERT(nullptr != out);

        pmr.construct(out);

        return out;
    }
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey& serialized,
    alloc::Default alloc) noexcept -> crypto::symmetric::KeyPrivate*
{
    try {
        using ReturnType = crypto::symmetric::implementation::Key;

        if (false == proto::Validate(serialized, VERBOSE)) {

            throw std::runtime_error{"invalid serialized key"};
        }

        // TODO c++20
        auto pmr = alloc::PMR<ReturnType>{alloc};
        auto* output = pmr.allocate(1_uz);

        if (nullptr == output) {

            throw std::runtime_error{"failed to allocate key"};
        }

        pmr.construct(output, api, engine, serialized);

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();
        // TODO c++20
        auto pmr = alloc::PMR<crypto::symmetric::KeyPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);

        OT_ASSERT(nullptr != out);

        pmr.construct(out);

        return out;
    }
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const crypto::symmetric::Source type,
    alloc::Default alloc) noexcept -> crypto::symmetric::KeyPrivate*
{
    try {
        using ReturnType = crypto::symmetric::implementation::Key;

        auto salt = ByteArray{};

        if (salt.resize(engine.SaltSize(type))) {

            throw std::runtime_error{"failed to create salt"};
        }

        // TODO c++20
        auto pmr = alloc::PMR<ReturnType>{alloc};
        auto* output = pmr.allocate(1_uz);

        if (nullptr == output) {

            throw std::runtime_error{"failed to allocate key"};
        }

        pmr.construct(
            output,
            api,
            engine,
            seed,
            salt.Bytes(),
            size,
            operations,
            difficulty,
            0u,
            type);

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();
        // TODO c++20
        auto pmr = alloc::PMR<crypto::symmetric::KeyPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);

        OT_ASSERT(nullptr != out);

        pmr.construct(out);

        return out;
    }
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t size,
    const crypto::symmetric::Source type,
    alloc::Default alloc) noexcept -> crypto::symmetric::KeyPrivate*
{
    try {
        using ReturnType = crypto::symmetric::implementation::Key;
        // TODO c++20
        auto pmr = alloc::PMR<ReturnType>{alloc};
        auto* output = pmr.allocate(1_uz);

        if (nullptr == output) {

            throw std::runtime_error{"failed to allocate key"};
        }

        pmr.construct(
            output,
            api,
            engine,
            seed,
            salt,
            size,
            (0u == operations) ? ReturnType::default_operations_ : operations,
            (0u == difficulty) ? ReturnType::default_difficulty_ : difficulty,
            (0u == parallel) ? ReturnType::default_threads_ : parallel,
            type);

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();
        // TODO c++20
        auto pmr = alloc::PMR<crypto::symmetric::KeyPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);

        OT_ASSERT(nullptr != out);

        pmr.construct(out);

        return out;
    }
}

auto SymmetricKey(
    const api::Session& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& raw,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) noexcept -> crypto::symmetric::KeyPrivate*
{
    try {
        using ReturnType = crypto::symmetric::implementation::Key;
        // TODO c++20
        auto pmr = alloc::PMR<ReturnType>{alloc};
        auto* output = pmr.allocate(1_uz);

        if (nullptr == output) {

            throw std::runtime_error{"failed to allocate key"};
        }

        pmr.construct(output, api, engine);
        auto& key = *output;

        if (false == key.SetRawKey(raw, reason)) {

            throw std::runtime_error{"failed to encrypt key"};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();
        // TODO c++20
        auto pmr = alloc::PMR<crypto::symmetric::KeyPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);

        OT_ASSERT(nullptr != out);

        pmr.construct(out);

        return out;
    }
}
}  // namespace opentxs::factory
