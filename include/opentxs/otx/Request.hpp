// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

namespace otx
{
class Request;
}  // namespace otx

namespace proto
{
class ServerRequest;
}  // namespace proto

class ByteArray;
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx
{
class OPENTXS_EXPORT Request
{
public:
    class Imp;

    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    static auto Factory(
        const api::Session& api,
        const Nym_p signer,
        const identifier::Notary& server,
        const otx::ServerRequestType type,
        const RequestNumber number,
        const opentxs::PasswordPrompt& reason) -> Request;
    OPENTXS_NO_EXPORT static auto Factory(
        const api::Session& api,
        const proto::ServerRequest serialized) -> Request;
    static auto Factory(const api::Session& api, const ReadView& view)
        -> Request;

    auto Initiator() const -> const identifier::Nym&;
    auto Number() const -> RequestNumber;
    auto Serialize() const noexcept -> ByteArray;
    auto Serialize(Writer&& destination) const noexcept -> bool;
    OPENTXS_NO_EXPORT auto Serialize(proto::ServerRequest& serialized) const
        -> bool;
    auto Server() const -> const identifier::Notary&;
    auto Type() const -> otx::ServerRequestType;
    auto SetIncludeNym(const bool include, const PasswordPrompt& reason)
        -> bool;

    auto Alias() const noexcept -> UnallocatedCString;
    auto Alias(alloc::Strategy alloc) const noexcept -> CString;
    auto ID() const noexcept -> identifier::Generic;
    auto Nym() const noexcept -> Nym_p;
    auto Terms() const noexcept -> std::string_view;
    auto Validate() const noexcept -> bool;
    auto Version() const noexcept -> VersionNumber;
    auto SetAlias(std::string_view alias) noexcept -> bool;

    auto swap(Request& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Request(Imp* imp) noexcept;
    Request(const Request&) noexcept;
    Request(Request&&) noexcept;
    auto operator=(const Request&) noexcept -> Request&;
    auto operator=(Request&&) noexcept -> Request&;
    virtual ~Request();

private:
    Imp* imp_;
};
}  // namespace opentxs::otx
