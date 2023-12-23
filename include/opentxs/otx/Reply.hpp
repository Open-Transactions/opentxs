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
class Reply;
}  // namespace otx

namespace proto
{
class OTXPush;
class ServerReply;
}  // namespace proto

class ByteArray;
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx
{
class OPENTXS_EXPORT Reply
{
public:
    class Imp;

    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    OPENTXS_NO_EXPORT static auto Factory(
        const api::Session& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Notary& server,
        const otx::ServerReplyType type,
        const RequestNumber number,
        const bool success,
        const PasswordPrompt& reason,
        std::shared_ptr<const proto::OTXPush>&& push = {}) -> Reply;
    static auto Factory(
        const api::Session& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Notary& server,
        const otx::ServerReplyType type,
        const RequestNumber number,
        const bool success,
        const PasswordPrompt& reason,
        opentxs::otx::PushType pushtype,
        const UnallocatedCString& payload) -> Reply;
    OPENTXS_NO_EXPORT static auto Factory(
        const api::Session& api,
        const proto::ServerReply serialized) -> Reply;
    static auto Factory(const api::Session& api, const ReadView& view) -> Reply;

    auto Number() const -> RequestNumber;
    auto Push() const -> std::shared_ptr<const proto::OTXPush>;
    auto Recipient() const -> const identifier::Nym&;
    auto Serialize(Writer&& destination) const noexcept -> bool;
    auto Serialize(proto::ServerReply& serialized) const -> bool;
    auto Server() const -> const identifier::Notary&;
    auto Success() const -> bool;
    auto Type() const -> otx::ServerReplyType;

    auto Alias() const noexcept -> UnallocatedCString;
    auto Alias(alloc::Strategy alloc) const noexcept -> CString;
    auto ID() const noexcept -> identifier::Generic;
    auto Nym() const noexcept -> Nym_p;
    auto Terms() const noexcept -> std::string_view;
    auto Validate() const noexcept -> bool;
    auto Version() const noexcept -> VersionNumber;
    auto SetAlias(std::string_view alias) noexcept -> bool;

    auto swap(Reply& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Reply(Imp* imp) noexcept;
    Reply(const Reply&) noexcept;
    Reply(Reply&&) noexcept;
    auto operator=(const Reply&) noexcept -> Reply&;
    auto operator=(Reply&&) noexcept -> Reply&;

    virtual ~Reply();

private:
    Imp* imp_;
};
}  // namespace opentxs::otx
