// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class HDPath;
class Identifier;
}  // namespace proto

class Cheque;
class Contract;
class Identifier;
class Item;

using OTIdentifier = Pimpl<Identifier>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::OTIdentifier> {
    auto operator()(const opentxs::Identifier& data) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::OTIdentifier> {
    auto operator()(
        const opentxs::OTIdentifier& lhs,
        const opentxs::OTIdentifier& rhs) const -> bool;
};
}  // namespace std

namespace opentxs
{
OPENTXS_EXPORT auto default_identifier_algorithm() noexcept
    -> identifier::Algorithm;
OPENTXS_EXPORT auto operator==(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const opentxs::Pimpl<opentxs::Identifier>& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs
{
/** An Identifier is basically a 256 bit hash value. This class makes it easy to
 * convert IDs back and forth to strings. */
class OPENTXS_EXPORT Identifier : virtual public Data
{
public:
    using ot_super = opentxs::Data;

    static auto Random() -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory() -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Identifier& rhs)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const UnallocatedCString& rhs)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const String& rhs)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const identity::Nym& nym)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Cheque& cheque)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Item& item)
        -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Factory(const Contract& contract)
        -> opentxs::Pimpl<opentxs::Identifier>;
    OPENTXS_NO_EXPORT static auto Factory(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path) -> opentxs::Pimpl<opentxs::Identifier>;
    static auto Validate(const UnallocatedCString& id) -> bool;

    using ot_super::operator==;
    virtual auto operator==(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator!=;
    virtual auto operator!=(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator>;
    virtual auto operator>(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator<;
    virtual auto operator<(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator<=;
    virtual auto operator<=(const Identifier& rhs) const noexcept -> bool = 0;
    using ot_super::operator>=;
    virtual auto operator>=(const Identifier& rhs) const noexcept -> bool = 0;

    virtual auto Algorithm() const noexcept -> identifier::Algorithm = 0;
    virtual void GetString(String& theStr) const = 0;
    virtual auto Type() const noexcept -> identifier::Type = 0;

    virtual auto CalculateDigest(
        const ReadView bytes,
        const identifier::Algorithm type = default_identifier_algorithm())
        -> bool = 0;
    using opentxs::Data::Randomize;
    virtual auto Randomize() -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        proto::Identifier& out) const noexcept -> bool = 0;
    virtual void SetString(const UnallocatedCString& encoded) = 0;
    virtual void SetString(const String& encoded) = 0;
    using ot_super::swap;
    virtual void swap(Identifier& rhs) = 0;

    ~Identifier() override = default;

protected:
    Identifier() = default;

private:
    friend opentxs::Pimpl<opentxs::Identifier>;

#ifndef _WIN32
    auto clone() const -> Identifier* override = 0;
#endif
    Identifier(const Identifier&) = delete;
    Identifier(Identifier&&) = delete;
    auto operator=(const Identifier&) -> Identifier& = delete;
    auto operator=(Identifier&&) -> Identifier& = delete;
};
}  // namespace opentxs
