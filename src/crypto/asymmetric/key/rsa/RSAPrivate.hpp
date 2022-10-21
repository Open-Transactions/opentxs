// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <new>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "internal/crypto/asymmetric/key/RSA.hpp"
#include "opentxs/util/Allocated.hpp"

namespace opentxs::crypto::asymmetric::key
{
class RSAPrivate : virtual public internal::key::RSA, virtual public KeyPrivate
{
public:
    static auto Blank(allocator_type alloc) noexcept -> RSAPrivate*;

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
        -> RSAPrivate* override;
    [[nodiscard]] auto get_deleter() const noexcept
        -> std::function<void(KeyPrivate*)> override;

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
