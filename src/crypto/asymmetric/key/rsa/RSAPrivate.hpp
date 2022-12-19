// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::KeyPrivate

#pragma once

#include <functional>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "internal/crypto/asymmetric/key/RSA.hpp"
#include "internal/util/PMR.hpp"

namespace opentxs::crypto::asymmetric::key
{
class RSAPrivate : virtual public internal::key::RSA, virtual public KeyPrivate
{
public:
    static auto Blank(allocator_type alloc) noexcept -> RSAPrivate*
    {
        return default_construct<RSAPrivate>({alloc});
    }

    auto asRSA() const noexcept -> const internal::key::RSA& override
    {
        return *this;
    }
    [[nodiscard]] auto asRSAPrivate() const noexcept
        -> const key::RSAPrivate* override
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

    auto asRSA() noexcept -> internal::key::RSA& override { return *this; }
    [[nodiscard]] auto asRSAPrivate() noexcept -> key::RSAPrivate* override
    {
        return this;
    }

    RSAPrivate(allocator_type alloc) noexcept;
    RSAPrivate() = delete;
    RSAPrivate(const RSAPrivate& rhs, allocator_type alloc) noexcept;
    RSAPrivate(const RSAPrivate&) = delete;
    RSAPrivate(RSAPrivate&&) = delete;
    auto operator=(const RSAPrivate&) -> RSAPrivate& = delete;
    auto operator=(RSAPrivate&&) -> RSAPrivate& = delete;

    ~RSAPrivate() override;
};
}  // namespace opentxs::crypto::asymmetric::key
