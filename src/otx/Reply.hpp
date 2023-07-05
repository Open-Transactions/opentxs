// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ServerReply.pb.h>
#include <memory>
#include <string_view>

#include "core/contract/Signable.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx
{
class Reply::Imp final
    : public opentxs::contract::implementation::Signable<identifier::Generic>
{
public:
    auto Number() const -> RequestNumber { return number_; }
    auto Push() const -> std::shared_ptr<const proto::OTXPush>
    {
        return payload_;
    }
    auto Recipient() const -> const identifier::Nym& { return recipient_; }
    auto Serialize(proto::ServerReply& serialize) const -> bool;
    auto Serialize(Writer&& destination) const noexcept -> bool final;
    auto Server() const -> const identifier::Notary& { return server_; }
    auto Success() const -> bool { return success_; }
    auto Type() const -> otx::ServerReplyType { return type_; }

    Imp(const api::Session& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Notary& server,
        const otx::ServerReplyType type,
        const RequestNumber number,
        const bool success,
        std::shared_ptr<const proto::OTXPush>&& push);
    Imp(const api::Session& api, const proto::ServerReply serialized);
    Imp() = delete;
    Imp(const Imp& rhs) noexcept;
    Imp(Imp&& rhs) = delete;
    auto operator=(const Imp& rhs) -> Imp& = delete;
    auto operator=(Imp&& rhs) -> Imp& = delete;

    ~Imp() final = default;

private:
    friend otx::Reply;

    const identifier::Nym recipient_;
    const identifier::Notary server_;
    const otx::ServerReplyType type_;
    const bool success_;
    const RequestNumber number_;
    const std::shared_ptr<const proto::OTXPush> payload_;

    static auto extract_nym(
        const api::Session& api,
        const proto::ServerReply serialized) -> Nym_p;

    auto calculate_id() const -> identifier_type final;
    auto full_version() const -> proto::ServerReply;
    auto id_version() const -> proto::ServerReply;
    auto Name() const noexcept -> std::string_view final { return {}; }
    using Signable::serialize;
    auto serialize(proto::ServerReply& output) const -> bool;
    auto signature_version() const -> proto::ServerReply;
    auto update_signature(const PasswordPrompt& reason) -> bool final;
    auto validate() const -> bool final;
    auto verify_signature(const proto::Signature& signature) const
        -> bool final;
};
}  // namespace opentxs::otx
