// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/base/Implementation.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PeerRequest.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/contract/peer/Types.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/PeerRequest.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::contract::peer::request::base
{
using namespace std::literals;

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    VersionNumber version,
    identifier::Nym initiator,
    identifier::Nym responder,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , api_(api)
    , signer_(std::move(signer))
    , version_(std::move(version))
    , initiator_(std::move(initiator), alloc)
    , responder_(std::move(responder), alloc)
    , cookie_(api_.Factory().IdentifierFromRandom())
    , id_()
    , sig_()
    , time_()
{
    check_nym();
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , api_(api)
    , signer_(std::move(signer))
    , version_(proto.version())
    , initiator_(api_.Factory().Internal().NymID(proto.initiator(), alloc))
    , responder_(api_.Factory().Internal().NymID(proto.recipient(), alloc))
    , cookie_(api_.Factory().Internal().Identifier(proto.cookie(), alloc))
    , id_()
    , sig_()
    , time_()
{
    id_.set_value(api_.Factory().Internal().Identifier(proto.id(), alloc));
    sig_.set_value(proto.signature());
    check_nym();
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , api_(rhs.api_)
    , signer_(rhs.signer_)
    , version_(rhs.version_)
    , initiator_(rhs.initiator_, alloc)
    , responder_(rhs.responder_, alloc)
    , cookie_(rhs.cookie_, alloc)
    , id_()
    , sig_()
    , time_(*rhs.time_.lock_shared())
{
    if (rhs.id_.ready()) { id_.set_value(rhs.id_.get()); }

    if (rhs.sig_.ready()) { sig_.set_value(rhs.sig_.get()); }
}

auto Implementation::add_signature(const PasswordPrompt& reason) -> bool
{
    if (sig_.ready()) {
        LogError()()("already signed").Flush();

        return false;
    }

    if (false == signer_.operator bool()) {
        LogError()()("missing signer nym").Flush();

        return false;
    }

    const auto& signer = *signer_;
    auto proto = signing_form();
    auto& sig = *proto.mutable_signature();
    using enum crypto::SignatureRole;

    if (signer.Internal().Sign(proto, PeerRequest, sig, reason)) {
        sig_.set_value(sig);

        return true;
    } else {
        LogError()()("failed to create signature").Flush();

        return false;
    }
}

auto Implementation::calculate_id(
    const api::Session& api,
    const serialized_type& contract) noexcept -> identifier_type
{
    return api.Factory().Internal().Session().IdentifierFromPreimage(contract);
}

auto Implementation::calculate_id() const noexcept -> identifier_type
{
    return calculate_id(api_, id_form());
}

auto Implementation::check_nym() const noexcept(false) -> void
{
    if (false == signer_.operator bool()) {

        throw std::runtime_error{"missing signer"};
    }

    if (false == signer_->VerifyPseudonym()) {

        throw std::runtime_error{"invalid signer"};
    }

    if (signer_->ID() != initiator_) {

        throw std::runtime_error{
            "expected signer nym "s.append(initiator_.asBase58(api_.Crypto()))
                .append(" but found ")
                .append(signer_->ID().asBase58(api_.Crypto()))};
    }
}

auto Implementation::final_form() const noexcept -> serialized_type
{
    auto out = signing_form();

    assert_true(sig_.ready());

    out.mutable_signature()->CopyFrom(sig_.get());

    return out;
}

auto Implementation::Finish(const PasswordPrompt& reason) noexcept -> bool
{
    if (id_.ready()) {

        return false;
    } else {
        id_.set_value(calculate_id());
    }

    if (add_signature(reason)) {

        return validate();
    } else {

        return false;
    }
}

auto Implementation::ID() const noexcept -> const identifier_type&
{
    assert_true(id_.ready());

    return id_.get();
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = serialized_type{};

    if (version_ < minimum_version_) {
        out.set_version(minimum_version_);
    } else {
        out.set_version(version_);
    }

    initiator_.Internal().Serialize(*out.mutable_initiator());
    responder_.Internal().Serialize(*out.mutable_recipient());
    out.set_type(translate(Type()));
    cookie_.Internal().Serialize(*out.mutable_cookie());

    return out;
}

auto Implementation::Received() const noexcept -> Time
{
    return *time_.lock_shared();
}

auto Implementation::Serialize(Writer&& out) const noexcept -> bool
{
    if (auto proto = serialized_type{}; Serialize(proto)) {

        return protobuf::write(proto, std::move(out));
    } else {

        return false;
    }
}

auto Implementation::Serialize(serialized_type& out) const noexcept -> bool
{
    out = final_form();

    return true;
}

auto Implementation::signing_form() const noexcept -> serialized_type
{
    auto out = id_form();
    ID().Internal().Serialize(*out.mutable_id());

    return out;
}

auto Implementation::Validate() const noexcept -> bool { return validate(); }

auto Implementation::validate() const noexcept -> bool
{
    if (calculate_id() != ID()) {
        LogError()()("id mismatch").Flush();

        return false;
    }

    if (false == protobuf::syntax::check(LogError(), final_form())) {
        LogError()()("invalid syntax").Flush();

        return false;
    }

    if (false == sig_.ready()) {
        LogError()()("missing signature").Flush();

        return false;
    }

    if (false == verify_signature(sig_.get())) {
        LogError()()("invalid signature").Flush();

        return false;
    }

    return true;
}

auto Implementation::verify_signature(
    const protobuf::Signature& signature) const noexcept -> bool
{
    if (false == signer_.operator bool()) {
        LogError()()("missing signer nym").Flush();

        return false;
    }

    const auto& signer = *signer_;
    auto proto = signing_form();
    auto& sig = *proto.mutable_signature();
    sig.CopyFrom(signature);

    return signer.Internal().Verify(proto, sig);
}

Implementation::~Implementation() = default;
}  // namespace opentxs::contract::peer::request::base
