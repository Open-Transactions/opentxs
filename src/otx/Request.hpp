// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::otx::ServerRequestType

#pragma once

#include <OTXEnums.pb.h>
#include <ServerRequest.pb.h>

#include "core/contract/Signable.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/Request.hpp"
#include "opentxs/otx/ServerRequestType.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Container.hpp"
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
class Signature;
}  // namespace proto

class ByteArray;
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx
{
class Request::Imp final : public opentxs::contract::implementation::Signable
{
public:
    auto Initiator() const -> const identifier::Nym& { return initiator_; }
    auto Number() const -> RequestNumber;
    auto Serialize(Writer&& destination) const -> bool;
    auto Serialize(proto::ServerRequest& serialized) const -> bool;
    auto Server() const -> const identifier::Notary& { return server_; }
    auto Type() const -> otx::ServerRequestType { return type_; }

    auto SetIncludeNym(const bool include, const PasswordPrompt& reason)
        -> bool;

    Imp(const api::Session& api,
        const Nym_p signer,
        const identifier::Nym& initiator,
        const identifier::Notary& server,
        const otx::ServerRequestType type,
        const RequestNumber number);
    Imp(const api::Session& api, const proto::ServerRequest serialized);
    Imp() = delete;
    Imp(const Imp& rhs) noexcept;
    Imp(Imp&& rhs) = delete;
    auto operator=(const Imp& rhs) -> Imp& = delete;
    auto operator=(Imp&& rhs) -> Imp& = delete;

    ~Imp() final = default;

private:
    friend otx::Request;

    const identifier::Nym initiator_;
    const identifier::Notary server_;
    const otx::ServerRequestType type_;
    const RequestNumber number_;
    OTFlag include_nym_;

    static auto extract_nym(
        const api::Session& api,
        const proto::ServerRequest serialized) -> Nym_p;

    auto GetID(const Lock& lock) const -> identifier::Generic final;
    auto full_version(const Lock& lock) const -> proto::ServerRequest;
    auto id_version(const Lock& lock) const -> proto::ServerRequest;
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Serialize() const noexcept -> ByteArray final;
    auto serialize(const Lock& lock, proto::ServerRequest& serialized) const
        -> bool;
    auto signature_version(const Lock& lock) const -> proto::ServerRequest;
    auto update_signature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final;
    auto validate(const Lock& lock) const -> bool final;
    auto verify_signature(const Lock& lock, const proto::Signature& signature)
        const -> bool final;
};
}  // namespace opentxs::otx
