// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::KeyPrivate
// IWYU pragma: no_include "crypto/asymmetric/base/KeyPrivate.hpp"

#pragma once

#include "crypto/asymmetric/key/hd/HDPrivate.hpp"
#include "internal/crypto/asymmetric/key/Ed25519.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::crypto::asymmetric::key
{
class Ed25519Private : virtual public internal::key::Ed25519,
                       virtual public HDPrivate
{
public:
    static auto Blank(allocator_type alloc) noexcept -> Ed25519Private*
    {
        return pmr::default_construct<Ed25519Private>({alloc});
    }

    auto asEd25519() const noexcept -> const internal::key::Ed25519& override
    {
        return *this;
    }
    [[nodiscard]] auto asEd25519Private() const noexcept
        -> const key::Ed25519Private* override
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> asymmetric::KeyPrivate* override
    {
        return pmr::clone_as<asymmetric::KeyPrivate>(this, {alloc});
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    auto asEd25519() noexcept -> internal::key::Ed25519& override
    {
        return *this;
    }
    [[nodiscard]] auto asEd25519Private() noexcept
        -> key::Ed25519Private* override
    {
        return this;
    }

    Ed25519Private(allocator_type alloc = {}) noexcept;
    Ed25519Private(const Ed25519Private& rhs, allocator_type alloc) noexcept;
    Ed25519Private(const Ed25519Private&) = delete;
    Ed25519Private(Ed25519Private&&) = delete;
    auto operator=(const Ed25519Private&) -> Ed25519Private& = delete;
    auto operator=(Ed25519Private&&) -> Ed25519Private& = delete;

    ~Ed25519Private() override;
};
}  // namespace opentxs::crypto::asymmetric::key
