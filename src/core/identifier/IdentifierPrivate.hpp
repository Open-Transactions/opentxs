// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/ByteArrayPrivate.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/core/identifier/Types.hpp"
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

namespace proto
{
class Identifier;
}  // namespace proto

}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identifier
{
class IdentifierPrivate final : public internal::Identifier,
                                public ByteArrayPrivate
{
public:
    const identifier::Algorithm algorithm_;
    const identifier::Type type_;

    auto Algorithm() const noexcept -> identifier::Algorithm
    {
        return algorithm_;
    }
    auto asBase58(const api::Crypto& api) const -> UnallocatedCString;
    auto asBase58(const api::Crypto& api, alloc::Default alloc) const
        -> CString;
    auto Serialize(proto::Identifier& out) const noexcept -> bool final;
    auto Type() const noexcept -> identifier::Type { return type_; }

    IdentifierPrivate() = delete;
    IdentifierPrivate(
        const identifier::Algorithm algorithm,
        const identifier::Type type,
        const ReadView hash,
        allocator_type alloc = {}) noexcept;
    IdentifierPrivate(const IdentifierPrivate& rhs) = delete;
    IdentifierPrivate(IdentifierPrivate&& rhs) = delete;
    auto operator=(const IdentifierPrivate& rhs) -> IdentifierPrivate& = delete;
    auto operator=(IdentifierPrivate&& rhs) -> IdentifierPrivate& = delete;

    ~IdentifierPrivate() override = default;

private:
    static constexpr auto proto_version_ = VersionNumber{1};
};
}  // namespace opentxs::identifier
