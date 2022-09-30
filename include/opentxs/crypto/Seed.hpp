// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"

#pragma once

#include <functional>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace internal
{
class Seed;
}  // namespace internal

class Seed;
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::crypto::Seed> {
    auto operator()(const opentxs::crypto::Seed& data) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::crypto::Seed> {
    auto operator()(
        const opentxs::crypto::Seed& lhs,
        const opentxs::crypto::Seed& rhs) const noexcept -> bool;
};
}  // namespace std

namespace opentxs::crypto
{
OPENTXS_EXPORT auto operator<(const Seed& lhs, const Seed& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator==(const Seed& lhs, const Seed& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto swap(Seed& lhs, Seed& rhs) noexcept -> void;

class OPENTXS_EXPORT Seed
{
public:
    class Imp;

    auto Entropy() const noexcept -> const Secret&;
    auto ID() const noexcept -> const identifier::Generic&;
    auto Index() const noexcept -> Bip32Index;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Seed&;
    auto Phrase() const noexcept -> const Secret&;
    auto Type() const noexcept -> SeedStyle;
    auto Words() const noexcept -> const Secret&;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Seed&;
    auto swap(Seed& rhs) noexcept -> void;

    Seed() noexcept;
    OPENTXS_NO_EXPORT Seed(Imp* imp) noexcept;
    Seed(const Seed&) noexcept;
    Seed(Seed&&) noexcept;
    auto operator=(const Seed&) noexcept -> Seed&;
    auto operator=(Seed&&) noexcept -> Seed&;

    ~Seed();

private:
    Imp* imp_;
};
}  // namespace opentxs::crypto
