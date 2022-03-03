// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/identifier/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Type.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <tuple>

#include "Proto.hpp"
#include "core/Data.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identity
{
class Nym;
}  // namespace identity

namespace proto
{
class HDPath;
class Identifier;
}  // namespace proto

class Contract;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::implementation
{
class Identifier final : virtual public opentxs::Identifier,
                         virtual public opentxs::identifier::Nym,
                         virtual public opentxs::identifier::Notary,
                         virtual public opentxs::identifier::UnitDefinition,
                         public Data
{
public:
    using ot_super = Data;
    using Decoded = std::tuple<Vector, identifier::Algorithm, identifier::Type>;

    static auto decode(ReadView encoded) noexcept -> Decoded;

    using ot_super::operator==;
    auto operator==(const opentxs::Identifier& rhs) const noexcept -> bool final
    {
        return Data::operator==(rhs);
    }
    using ot_super::operator!=;
    auto operator!=(const opentxs::Identifier& rhs) const noexcept -> bool final
    {
        return Data::operator!=(rhs);
    }
    using ot_super::operator>;
    auto operator>(const opentxs::Identifier& rhs) const noexcept -> bool final
    {
        return Data::operator>(rhs);
    }
    using ot_super::operator<;
    auto operator<(const opentxs::Identifier& rhs) const noexcept -> bool final
    {
        return Data::operator<(rhs);
    }
    using ot_super::operator<=;
    auto operator<=(const opentxs::Identifier& rhs) const noexcept -> bool final
    {
        return Data::operator<=(rhs);
    }
    using ot_super::operator>=;
    auto operator>=(const opentxs::Identifier& rhs) const noexcept -> bool final
    {
        return Data::operator>=(rhs);
    }

    auto Algorithm() const noexcept -> identifier::Algorithm final
    {
        return algorithm_;
    }
    auto GetString(String& theStr) const -> void final;
    auto str() const -> UnallocatedCString final;
    auto Type() const noexcept -> identifier::Type final { return type_; }

    using Data::Assign;
    auto Assign(const void* data, const std::size_t size) noexcept
        -> bool final;
    auto CalculateDigest(const ReadView bytes, const identifier::Algorithm type)
        -> bool final;
    using Data::Concatenate;
    auto Concatenate(const void* data, const std::size_t size) noexcept
        -> bool final;
    using ot_super::Randomize;
    auto Randomize(const std::size_t size) -> bool final;
    auto Randomize() -> bool final;
    auto Serialize(proto::Identifier& out) const noexcept -> bool final;
    using opentxs::Identifier::SetString;
    auto SetString(const UnallocatedCString& encoded) -> void final;
    auto SetString(const String& encoded) -> void final;
    auto SetString(const ReadView encoded) -> void;
    using ot_super::swap;
    auto swap(opentxs::Identifier& rhs) -> void final;

    Identifier(const identity::Nym& nym) noexcept;
    Identifier(const Contract& contract) noexcept;
    Identifier(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path) noexcept;
    Identifier(
        Vector&& data,
        identifier::Algorithm algorithm,
        identifier::Type type) noexcept;
    Identifier(Decoded&& data) noexcept;
    Identifier(
        const Vector& data,
        identifier::Algorithm algorithm,
        identifier::Type type) noexcept;
    Identifier(identifier::Type type) noexcept;
    Identifier(const Identifier& rhs) noexcept;
    Identifier() noexcept;

    ~Identifier() final = default;

private:
    friend opentxs::Identifier;
    friend opentxs::identifier::Nym;
    friend opentxs::identifier::Notary;
    friend opentxs::identifier::UnitDefinition;

    identifier::Algorithm algorithm_;
    identifier::Type type_;

    static constexpr auto prefix_ = "ot";
    static constexpr auto minimum_encoded_bytes_ = std::size_t{6};
    static constexpr auto header_bytes_ = sizeof(algorithm_) + sizeof(type_);
    static constexpr auto proto_version_ = VersionNumber{1};

    static auto contract_contents_to_identifier(const Contract& in)
        -> Identifier*;
    static auto hash_bytes(const identifier::Algorithm type) noexcept
        -> std::size_t;
    static auto IDToHashType(const identifier::Algorithm type)
        -> crypto::HashType;
    static auto is_compatible(
        identifier::Type lhs,
        identifier::Type rhs) noexcept -> bool;
    static auto is_supported(const identifier::Algorithm type) noexcept -> bool;
    static auto is_supported(const identifier::Type type) noexcept -> bool;
    static auto path_to_data(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path) -> OTData;
    static auto required_payload(const identifier::Algorithm type) noexcept
        -> std::size_t
    {
        return header_bytes_ + hash_bytes(type);
    }

    auto clone() const -> Identifier* final;
    auto to_string() const noexcept -> UnallocatedCString;

    Identifier(Identifier&& rhs) = delete;
    auto operator=(const Identifier& rhs) -> Identifier& = delete;
    auto operator=(Identifier&& rhs) -> Identifier& = delete;
};
}  // namespace opentxs::implementation
