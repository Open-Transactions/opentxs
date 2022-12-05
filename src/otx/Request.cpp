// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::SignatureRole
// IWYU pragma: no_forward_declare opentxs::otx::ServerRequestType

#include "otx/Request.hpp"  // IWYU pragma: associated

#include <Nym.pb.h>
#include <ServerRequest.pb.h>
#include <Signature.pb.h>
#include <memory>
#include <utility>

#include "core/contract/Signable.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/ServerRequest.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Request.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::otx
{
const VersionNumber Request::DefaultVersion{2};
const VersionNumber Request::MaxVersion{2};

auto Request::Factory(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Notary& server,
    const otx::ServerRequestType type,
    const RequestNumber number,
    const PasswordPrompt& reason) -> Request
{
    OT_ASSERT(signer);

    auto output{std::make_unique<Request::Imp>(
        api, signer, signer->ID(), server, type, number)};

    OT_ASSERT(output);

    Lock lock(output->lock_);
    output->update_signature(lock, reason);

    OT_ASSERT(false == output->id(lock).empty());

    return Request{output.release()};
}

auto Request::Factory(
    const api::Session& api,
    const proto::ServerRequest serialized) -> Request
{
    return Request{new Request::Imp(api, serialized)};
}

auto Request::Factory(const api::Session& api, const ReadView& view) -> Request
{
    return Request{
        new Request::Imp(api, proto::Factory<proto::ServerRequest>(view))};
}

auto Request::Number() const -> RequestNumber { return imp_->Number(); }

auto Request::Initiator() const -> const identifier::Nym&
{
    return imp_->Initiator();
}

auto Request::Serialize() const noexcept -> ByteArray
{
    return imp_->Serialize();
}

auto Request::Serialize(Writer&& destination) const -> bool
{
    return imp_->Serialize(std::move(destination));
}

auto Request::Serialize(proto::ServerRequest& serialized) const -> bool
{
    return imp_->Serialize(serialized);
}

auto Request::Server() const -> const identifier::Notary&
{
    return imp_->Server();
}

auto Request::Type() const -> otx::ServerRequestType { return imp_->Type(); }

auto Request::SetIncludeNym(const bool include, const PasswordPrompt& reason)
    -> bool
{
    return imp_->SetIncludeNym(include, reason);
}

auto Request::Alias() const noexcept -> UnallocatedCString
{
    return imp_->Alias();
}

auto Request::ID() const noexcept -> identifier::Generic { return imp_->ID(); }

auto Request::Nym() const noexcept -> Nym_p { return imp_->Nym(); }

auto Request::Terms() const noexcept -> const UnallocatedCString&
{
    return imp_->Terms();
}

auto Request::Validate() const noexcept -> bool { return imp_->Validate(); }

auto Request::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

auto Request::SetAlias(const UnallocatedCString& alias) noexcept -> bool
{
    return imp_->SetAlias(alias);
}

auto Request::swap(Request& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

Request::Request(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Request::Request(const Request& rhs) noexcept
    : Request(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Request::Request(Request&& rhs) noexcept
    : imp_{nullptr}
{
    swap(rhs);
}

auto Request::operator=(const Request& rhs) noexcept -> Request&
{
    auto old = std::unique_ptr<Imp>{imp_};
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();

    return *this;
}

auto Request::operator=(Request&& rhs) noexcept -> Request&
{
    swap(rhs);

    return *this;
}

Request::~Request()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}

Request::Imp::Imp(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Nym& initiator,
    const identifier::Notary& server,
    const otx::ServerRequestType type,
    const RequestNumber number)
    : Signable(api, signer, DefaultVersion, "", "")
    , initiator_(initiator)
    , server_(server)
    , type_(type)
    , number_(number)
    , include_nym_(Flag::Factory(false))
{
    Lock lock(lock_);
    first_time_init(lock);
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
Request::Imp::Imp(
    const api::Session& api,
    const proto::ServerRequest serialized)
    : Signable(
          api,
          extract_nym(api, serialized),
          serialized.version(),
          "",
          "",
          api.Factory().IdentifierFromBase58(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , initiator_((nym_) ? nym_->ID() : identifier::Nym())
    , server_(api.Factory().NotaryIDFromBase58(serialized.server()))
    , type_(translate(serialized.type()))
    , number_(serialized.request())
    , include_nym_(Flag::Factory(false))
{
    Lock lock(lock_);
    init_serialized(lock);
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

Request::Imp::Imp(const Imp& rhs) noexcept
    : Signable(rhs)
    , initiator_(rhs.initiator_)
    , server_(rhs.server_)
    , type_(rhs.type_)
    , number_(rhs.number_)
    , include_nym_(Flag::Factory(rhs.include_nym_.get()))
{
}

auto Request::Imp::extract_nym(
    const api::Session& api,
    const proto::ServerRequest serialized) -> Nym_p
{
    if (serialized.has_credentials()) {

        return api.Wallet().Internal().Nym(serialized.credentials());
    } else if (false == serialized.nym().empty()) {

        return api.Wallet().Nym(
            api.Factory().NymIDFromBase58(serialized.nym()));
    }

    return nullptr;
}

auto Request::Imp::full_version(const Lock& lock) const -> proto::ServerRequest
{
    auto contract = signature_version(lock);

    if (0 < signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    if (include_nym_.get() && bool(nym_)) {
        auto publicNym = proto::Nym{};
        if (false == nym_->Internal().Serialize(publicNym)) {}
        *contract.mutable_credentials() = publicNym;
    }

    return contract;
}

auto Request::Imp::GetID(const Lock& lock) const -> identifier::Generic
{
    return api_.Factory().InternalSession().IdentifierFromPreimage(
        id_version(lock));
}

auto Request::Imp::id_version(const Lock& lock) const -> proto::ServerRequest
{
    proto::ServerRequest output{};
    output.set_version(version_);
    output.clear_id();  // Must be blank
    output.set_type(translate(type_));
    output.set_nym(initiator_.asBase58(api_.Crypto()));
    output.set_server(server_.asBase58(api_.Crypto()));
    output.set_request(number_);
    output.clear_signature();  // Must be blank

    return output;
}

auto Request::Imp::Number() const -> RequestNumber
{
    Lock lock(lock_);

    return number_;
}

auto Request::Imp::Serialize() const noexcept -> ByteArray
{
    Lock lock(lock_);

    return api_.Factory().InternalSession().Data(full_version(lock));
}

auto Request::Imp::Serialize(Writer&& destination) const -> bool
{
    Lock lock(lock_);

    auto serialized = proto::ServerRequest{};
    if (false == serialize(lock, serialized)) { return false; }

    return write(serialized, std::move(destination));
}

auto Request::Imp::serialize(const Lock& lock, proto::ServerRequest& output)
    const -> bool
{
    output = full_version(lock);

    return true;
}

auto Request::Imp::Serialize(proto::ServerRequest& output) const -> bool
{
    Lock lock(lock_);

    return serialize(lock, output);
}

auto Request::Imp::SetIncludeNym(
    const bool include,
    const PasswordPrompt& reason) -> bool
{
    Lock lock(lock_);

    if (include) {
        include_nym_->On();
    } else {
        include_nym_->Off();
    }

    return update_signature(lock, reason);
}

auto Request::Imp::signature_version(const Lock& lock) const
    -> proto::ServerRequest
{
    auto contract = id_version(lock);
    contract.set_id(id_.asBase58(api_.Crypto()));

    return contract;
}

auto Request::Imp::update_signature(
    const Lock& lock,
    const PasswordPrompt& reason) -> bool
{
    if (false == Signable::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = signature_version(lock);
    auto& signature = *serialized.mutable_signature();
    success = nym_->Internal().Sign(
        serialized, crypto::SignatureRole::ServerRequest, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to create signature.").Flush();
    }

    return success;
}

auto Request::Imp::validate(const Lock& lock) const -> bool
{
    bool validNym{false};

    if (nym_) { validNym = nym_->VerifyPseudonym(); }

    if (false == validNym) {
        LogError()(OT_PRETTY_CLASS())("Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(full_version(lock), VERBOSE);

    if (false == validSyntax) {
        LogError()(OT_PRETTY_CLASS())("Invalid syntax.").Flush();

        return false;
    }

    if (1 != signatures_.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number signatures.").Flush();

        return false;
    }

    bool validSig{false};
    const auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    if (false == validSig) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();

        return false;
    }

    return true;
}

auto Request::Imp::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const -> bool
{
    if (false == Signable::verify_signature(lock, signature)) { return false; }

    auto serialized = signature_version(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Internal().Verify(serialized, sigProto);
}
}  // namespace opentxs::otx
