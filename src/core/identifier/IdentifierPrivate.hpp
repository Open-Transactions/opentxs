// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <sstream>

#include "core/ByteArrayPrivate.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Identifier;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identifier
{
auto deserialize_account_subtype(std::uint16_t in) noexcept -> AccountSubtype;
auto deserialize_algorithm(std::uint8_t in) noexcept -> Algorithm;
auto deserialize_identifier_type(std::uint16_t in) noexcept -> Type;
auto get_hash_type(Algorithm) noexcept(false) -> crypto::HashType;

class IdentifierPrivate final : public internal::Identifier,
                                public ByteArrayPrivate
{
public:
    const identifier::Algorithm algorithm_;
    const identifier::Type type_;
    const identifier::AccountSubtype account_subtype_;

    auto AccountType() const noexcept -> opentxs::AccountType;
    auto Algorithm() const noexcept -> identifier::Algorithm
    {
        return algorithm_;
    }
    auto asBase58(const api::Crypto& api) const -> UnallocatedCString;
    auto asBase58(const api::Crypto& api, alloc::Default alloc) const
        -> CString;
    auto Get() const noexcept -> const IdentifierPrivate& final
    {
        return *this;
    }
    auto Serialize(proto::Identifier& out) const noexcept -> bool final;
    auto Serialize(network::zeromq::Message& out) const noexcept -> bool final;
    auto Type() const noexcept -> identifier::Type { return type_; }

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    IdentifierPrivate() = delete;
    IdentifierPrivate(
        const identifier::Algorithm algorithm,
        const identifier::Type type,
        const ReadView hash,
        const identifier::AccountSubtype subtype,
        allocator_type alloc = {}) noexcept;
    IdentifierPrivate(const IdentifierPrivate& rhs) = delete;
    IdentifierPrivate(IdentifierPrivate&& rhs) = delete;
    auto operator=(const IdentifierPrivate& rhs) -> IdentifierPrivate& = delete;
    auto operator=(IdentifierPrivate&& rhs) -> IdentifierPrivate& = delete;

    ~IdentifierPrivate() override = default;

private:
    static constexpr auto proto_version_ = VersionNumber{1};

    auto serialize_account_subtype() const noexcept -> bool;
    auto text(const api::Crypto& api, alloc::Default alloc = {}) const noexcept
        -> std::stringstream;
};
}  // namespace opentxs::identifier
