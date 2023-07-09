// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::KeyPrivate
// IWYU pragma: no_include "crypto/asymmetric/base/KeyPrivate.hpp"

#pragma once

#include <functional>

#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"
#include "internal/crypto/asymmetric/key/HD.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key
{
class HDPrivate : virtual public internal::key::HD,
                  virtual public EllipticCurvePrivate
{
public:
    static auto Blank(alloc::Strategy alloc) noexcept -> HDPrivate*
    {
        return default_construct<HDPrivate>({alloc.result_});
    }

    auto asHD() const noexcept -> const internal::key::HD& override
    {
        return *this;
    }
    [[nodiscard]] auto asHDPrivate() const noexcept
        -> const key::HDPrivate* override
    {
        return this;
    }
    virtual auto Chaincode(const PasswordPrompt& reason) const noexcept
        -> ReadView;
    virtual auto ChildKey(
        const Bip32Index index,
        const PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept -> asymmetric::key::HD;
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> asymmetric::KeyPrivate* override
    {
        return pmr::clone_as<asymmetric::KeyPrivate>(this, {alloc});
    }
    virtual auto Depth() const noexcept -> int;
    virtual auto Fingerprint() const noexcept -> Bip32Fingerprint;
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
    {
        return make_deleter(this);
    }
    virtual auto Parent() const noexcept -> Bip32Fingerprint;
    virtual auto Xprv(const PasswordPrompt& reason, Writer&& out) const noexcept
        -> bool;
    virtual auto Xpub(const PasswordPrompt& reason, Writer&& out) const noexcept
        -> bool;

    auto asHD() noexcept -> internal::key::HD& override { return *this; }
    [[nodiscard]] auto asHDPrivate() noexcept -> key::HDPrivate* override
    {
        return this;
    }

    HDPrivate(allocator_type alloc) noexcept;
    HDPrivate() = delete;
    HDPrivate(const HDPrivate& rhs, allocator_type alloc) noexcept;
    HDPrivate(const HDPrivate&) = delete;
    HDPrivate(HDPrivate&&) = delete;
    auto operator=(const HDPrivate&) -> HDPrivate& = delete;
    auto operator=(HDPrivate&&) -> HDPrivate& = delete;

    ~HDPrivate() override;
};
}  // namespace opentxs::crypto::asymmetric::key
