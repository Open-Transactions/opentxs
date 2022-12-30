// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::KeyPrivate
// IWYU pragma: no_include "crypto/asymmetric/base/KeyPrivate.hpp"

#pragma once

#include <functional>

#include "crypto/asymmetric/key/hd/HDPrivate.hpp"
#include "internal/crypto/asymmetric/key/Secp256k1.hpp"
#include "internal/util/PMR.hpp"

namespace opentxs::crypto::asymmetric::key
{
class Secp256k1Private : virtual public internal::key::Secp256k1,
                         virtual public HDPrivate
{
public:
    static auto Blank(allocator_type alloc) noexcept -> Secp256k1Private*
    {
        return default_construct<Secp256k1Private>({alloc});
    }

    auto asSecp256k1() const noexcept
        -> const internal::key::Secp256k1& override
    {
        return *this;
    }
    [[nodiscard]] auto asSecp256k1Private() const noexcept
        -> const key::Secp256k1Private* override
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> asymmetric::KeyPrivate* override
    {
        return pmr::clone_as<asymmetric::KeyPrivate>(this, {alloc});
    }
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
    {
        return make_deleter(this);
    }

    auto asSecp256k1() noexcept -> internal::key::Secp256k1& override
    {
        return *this;
    }
    [[nodiscard]] auto asSecp256k1Private() noexcept
        -> key::Secp256k1Private* override
    {
        return this;
    }

    Secp256k1Private(allocator_type alloc) noexcept;
    Secp256k1Private() = delete;
    Secp256k1Private(
        const Secp256k1Private& rhs,
        allocator_type alloc) noexcept;
    Secp256k1Private(const Secp256k1Private&) = delete;
    Secp256k1Private(Secp256k1Private&&) = delete;
    auto operator=(const Secp256k1Private&) -> Secp256k1Private& = delete;
    auto operator=(Secp256k1Private&&) -> Secp256k1Private& = delete;

    ~Secp256k1Private() override;
};
}  // namespace opentxs::crypto::asymmetric::key
