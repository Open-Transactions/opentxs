// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/Request.hpp"  // IWYU pragma: associated

#include <Nym.pb.h>
#include <ServerRequest.pb.h>
#include <Signature.pb.h>
#include <memory>
#include <span>
#include <utility>

#include "core/contract/Signable.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/ServerRequest.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Request.hpp"
#include "opentxs/util/Allocator.hpp"
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
    assert_false(nullptr == signer);

    auto output{std::make_unique<Request::Imp>(
        api, signer, signer->ID(), server, type, number)};

    assert_false(nullptr == output);

    output->update_signature(reason);

    assert_false(output->ID().empty());

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

auto Request::Serialize(Writer&& destination) const noexcept -> bool
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

auto Request::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return imp_->Alias(alloc);
}

auto Request::ID() const noexcept -> identifier::Generic { return imp_->ID(); }

auto Request::Nym() const noexcept -> Nym_p { return imp_->Signer(); }

auto Request::Terms() const noexcept -> std::string_view
{
    return imp_->Terms();
}

auto Request::Validate() const noexcept -> bool { return imp_->Validate(); }

auto Request::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

auto Request::SetAlias(std::string_view alias) noexcept -> bool
{
    return imp_->SetAlias(alias);
}

auto Request::swap(Request& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

Request::Request(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp);
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
    first_time_init(set_name_from_id_);
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
          "",
          api.Factory().Internal().Identifier(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
                    serialized.signature())}
              : Signatures{})
    , initiator_((Signer()) ? Signer()->ID() : identifier::Nym())
    , server_(api_.Factory().Internal().NotaryID(serialized.server()))
    , type_(translate(serialized.type()))
    , number_(serialized.request())
    , include_nym_(Flag::Factory(false))
{
    init_serialized();
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

auto Request::Imp::calculate_id() const -> identifier_type
{
    return api_.Factory().Internal().Session().IdentifierFromPreimage(
        id_version());
}

auto Request::Imp::extract_nym(
    const api::Session& api,
    const proto::ServerRequest serialized) -> Nym_p
{
    if (serialized.has_credentials()) {

        return api.Wallet().Internal().Nym(serialized.credentials());
    } else if (serialized.has_nym()) {

        return api.Wallet().Nym(
            api.Factory().Internal().NymID(serialized.nym()));
    }

    return nullptr;
}

auto Request::Imp::full_version() const -> proto::ServerRequest
{
    auto contract = signature_version();
    const auto sigs = signatures();

    if (false == sigs.empty()) {
        contract.mutable_signature()->CopyFrom(*sigs.front());
    }

    if (include_nym_.get() && bool(Signer())) {
        auto nym = proto::Nym{};

        if (Signer()->Internal().Serialize(nym)) {
            contract.mutable_credentials()->CopyFrom(nym);
        }
    }

    return contract;
}

auto Request::Imp::id_version() const -> proto::ServerRequest
{
    proto::ServerRequest output{};
    output.set_version(Version());
    output.clear_id();  // Must be blank
    output.set_type(translate(type_));
    initiator_.Internal().Serialize(*output.mutable_nym());
    server_.Internal().Serialize(*output.mutable_server());
    output.set_request(number_);
    output.clear_signature();  // Must be blank

    return output;
}

auto Request::Imp::Number() const -> RequestNumber { return number_; }

auto Request::Imp::Serialize(Writer&& destination) const noexcept -> bool
{
    auto serialized = proto::ServerRequest{};

    if (false == serialize(serialized)) { return false; }

    return serialize(serialized, std::move(destination));
}

auto Request::Imp::serialize(proto::ServerRequest& output) const -> bool
{
    output = full_version();

    return true;
}

auto Request::Imp::Serialize(proto::ServerRequest& output) const -> bool
{
    return serialize(output);
}

auto Request::Imp::SetIncludeNym(
    const bool include,
    const PasswordPrompt& reason) -> bool
{
    if (include) {
        include_nym_->On();
    } else {
        include_nym_->Off();
    }

    return true;
}

auto Request::Imp::signature_version() const -> proto::ServerRequest
{
    auto contract = id_version();
    ID().Internal().Serialize(*contract.mutable_id());

    return contract;
}

auto Request::Imp::update_signature(const PasswordPrompt& reason) -> bool
{
    if (false == Signable::update_signature(reason)) { return false; }

    auto success = false;
    auto sigs = Signatures{};
    auto serialized = signature_version();
    auto& signature = *serialized.mutable_signature();
    success = Signer()->Internal().Sign(
        serialized, crypto::SignatureRole::ServerRequest, signature, reason);

    if (success) {
        sigs.emplace_back(new proto::Signature(signature));
        add_signatures(std::move(sigs));
    } else {
        LogError()()("Failed to create signature.").Flush();
    }

    return success;
}

auto Request::Imp::validate() const -> bool
{
    bool validNym{false};

    if (Signer()) { validNym = Signer()->VerifyPseudonym(); }

    if (false == validNym) {
        LogError()()("Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(full_version(), VERBOSE);

    if (false == validSyntax) {
        LogError()()("Invalid syntax.").Flush();

        return false;
    }

    const auto sigs = signatures();

    if (1_uz != sigs.size()) {
        LogError()()("Wrong number signatures.").Flush();

        return false;
    }

    bool validSig{false};
    const auto& signature = sigs.front();

    if (signature) { validSig = verify_signature(*signature); }

    if (false == validSig) {
        LogError()()("Invalid signature.").Flush();

        return false;
    }

    return true;
}

auto Request::Imp::verify_signature(const proto::Signature& signature) const
    -> bool
{
    if (false == Signable::verify_signature(signature)) { return false; }

    auto serialized = signature_version();
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return Signer()->Internal().Verify(serialized, sigProto);
}
}  // namespace opentxs::otx
