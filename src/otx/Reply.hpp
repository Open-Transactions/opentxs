// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "Proto.hpp"
#include "core/contract/Signable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/ServerReplyType.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/ServerReply.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class OTXPush;
class Signature;
}  // namespace proto

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::implementation
{
class Reply final : public otx::Reply,
                    public opentxs::contract::implementation::Signable
{
public:
    auto Number() const -> RequestNumber final { return number_; }
    auto Push() const -> std::shared_ptr<const proto::OTXPush> final
    {
        return payload_;
    }
    auto Recipient() const -> const identifier::Nym& final
    {
        return recipient_;
    }
    auto Serialize(proto::ServerReply& serialize) const -> bool final;
    auto Serialize(AllocateOutput destination) const -> bool final;
    auto Server() const -> const identifier::Notary& final { return server_; }
    auto Success() const -> bool final { return success_; }
    auto Type() const -> otx::ServerReplyType final { return type_; }

    ~Reply() final = default;

private:
    friend otx::Reply;

    const OTNymID recipient_;
    const OTNotaryID server_;
    const otx::ServerReplyType type_;
    const bool success_;
    const RequestNumber number_;
    const std::shared_ptr<const proto::OTXPush> payload_;

    static auto extract_nym(
        const api::Session& api,
        const proto::ServerReply serialized) -> Nym_p;

    auto clone() const noexcept -> Reply* final { return new Reply(*this); }
    auto GetID(const Lock& lock) const -> OTIdentifier final;
    auto full_version(const Lock& lock) const -> proto::ServerReply;
    auto id_version(const Lock& lock) const -> proto::ServerReply;
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Serialize() const noexcept -> OTData final;
    auto serialize(const Lock& lock, proto::ServerReply& output) const -> bool;
    auto signature_version(const Lock& lock) const -> proto::ServerReply;
    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;
    auto validate(const Lock& lock) const -> bool final;
    auto verify_signature(const Lock& lock, const proto::Signature& signature)
        const -> bool final;

    Reply(
        const api::Session& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Notary& server,
        const otx::ServerReplyType type,
        const RequestNumber number,
        const bool success,
        std::shared_ptr<const proto::OTXPush>&& push);
    Reply(const api::Session& api, const proto::ServerReply serialized);
    Reply() = delete;
    Reply(const Reply& rhs);
    Reply(Reply&& rhs) = delete;
    auto operator=(const Reply& rhs) -> Reply& = delete;
    auto operator=(Reply&& rhs) -> Reply& = delete;
};
}  // namespace opentxs::otx::implementation
