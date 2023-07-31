// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/Reply.hpp"  // IWYU pragma: associated

#include <OTXPush.pb.h>
#include <ServerReply.pb.h>
#include <Signature.pb.h>
#include <span>
#include <utility>

#include "core/contract/Signable.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/ServerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::otx
{
const VersionNumber Reply::DefaultVersion{1};
const VersionNumber Reply::MaxVersion{1};

static auto construct_push(
    OTXPushType pushtype,
    const UnallocatedCString& payload) -> std::shared_ptr<proto::OTXPush>
{
    auto pPush = std::make_shared<proto::OTXPush>();
    auto& push = *pPush;
    push.set_version(1);
    push.set_type(translate(pushtype));
    push.set_item(payload);

    return pPush;
}

auto Reply::Factory(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Notary& server,
    const otx::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    const PasswordPrompt& reason,
    std::shared_ptr<const proto::OTXPush>&& push) -> Reply
{
    OT_ASSERT(signer);

    auto output{std::make_unique<Reply::Imp>(
        api,
        signer,
        recipient,
        server,
        type,
        number,
        success,
        std::move(push))};

    OT_ASSERT(output);

    output->update_signature(reason);

    OT_ASSERT(false == output->ID().empty());

    return output.release();
}

auto Reply::Factory(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Notary& server,
    const otx::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    const PasswordPrompt& reason,
    opentxs::otx::OTXPushType pushtype,
    const UnallocatedCString& payload) -> Reply
{
    return Factory(
        api,
        signer,
        recipient,
        server,
        type,
        number,
        success,
        reason,
        construct_push(pushtype, payload));
}

auto Reply::Factory(
    const api::Session& api,
    const proto::ServerReply serialized) -> Reply
{
    return Reply{new Reply::Imp(api, serialized)};
}

auto Reply::Factory(const api::Session& api, const ReadView& view) -> Reply
{
    return Reply{new Reply::Imp(api, proto::Factory<proto::ServerReply>(view))};
}

auto Reply::Number() const -> RequestNumber { return imp_->Number(); }

auto Reply::Push() const -> std::shared_ptr<const proto::OTXPush>
{
    return imp_->Push();
}

auto Reply::Recipient() const -> const identifier::Nym&
{
    return imp_->Recipient();
}

auto Reply::Serialize(Writer&& destination) const noexcept -> bool
{
    return imp_->Serialize(std::move(destination));
}

auto Reply::Serialize(proto::ServerReply& serialized) const -> bool
{
    return imp_->Serialize(serialized);
}

auto Reply::Server() const -> const identifier::Notary&
{
    return imp_->Server();
}

auto Reply::Success() const -> bool { return imp_->Success(); }

auto Reply::Type() const -> otx::ServerReplyType { return imp_->Type(); }

auto Reply::Alias() const noexcept -> UnallocatedCString
{
    return imp_->Alias();
}

auto Reply::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return imp_->Alias(alloc);
}

auto Reply::ID() const noexcept -> identifier::Generic { return imp_->ID(); }

auto Reply::Nym() const noexcept -> Nym_p { return imp_->Signer(); }

auto Reply::Terms() const noexcept -> std::string_view { return imp_->Terms(); }

auto Reply::Validate() const noexcept -> bool { return imp_->Validate(); }

auto Reply::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

auto Reply::SetAlias(std::string_view alias) noexcept -> bool
{
    return imp_->SetAlias(alias);
}

auto Reply::swap(Reply& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

Reply::Reply(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Reply::Reply(const Reply& rhs) noexcept
    : Reply(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Reply::Reply(Reply&& rhs) noexcept
    : imp_{nullptr}
{
    swap(rhs);
}

auto Reply::operator=(const Reply& rhs) noexcept -> Reply&
{
    auto old = std::unique_ptr<Imp>{imp_};
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();

    return *this;
}

auto Reply::operator=(Reply&& rhs) noexcept -> Reply&
{
    swap(rhs);

    return *this;
}

Reply::~Reply()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}

Reply::Imp::Imp(
    const api::Session& api,
    const Nym_p signer,
    const identifier::Nym& recipient,
    const identifier::Notary& server,
    const otx::ServerReplyType type,
    const RequestNumber number,
    const bool success,
    std::shared_ptr<const proto::OTXPush>&& push)
    : Signable(api, signer, DefaultVersion, "", "")
    , recipient_(recipient)
    , server_(server)
    , type_(type)
    , success_(success)
    , number_(number)
    , payload_(std::move(push))
{
    first_time_init(set_name_from_id_);
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
Reply::Imp::Imp(const api::Session& api, const proto::ServerReply serialized)
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
    , recipient_(api_.Factory().Internal().NymID(serialized.nym()))
    , server_(api_.Factory().Internal().NotaryID(serialized.server()))
    , type_(translate(serialized.type()))
    , success_(serialized.success())
    , number_(serialized.request())
    , payload_(
          serialized.has_push()
              ? std::make_shared<proto::OTXPush>(serialized.push())
              : std::shared_ptr<proto::OTXPush>{})
{
    init_serialized();
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

Reply::Imp::Imp(const Imp& rhs) noexcept
    : Signable(rhs)
    , recipient_(rhs.recipient_)
    , server_(rhs.server_)
    , type_(rhs.type_)
    , success_(rhs.success_)
    , number_(rhs.number_)
    , payload_(rhs.payload_)
{
}

auto Reply::Imp::calculate_id() const -> identifier_type
{
    return api_.Factory().InternalSession().IdentifierFromPreimage(
        id_version());
}

auto Reply::Imp::extract_nym(
    const api::Session& api,
    const proto::ServerReply serialized) -> Nym_p
{
    const auto serverID =
        api.Factory().Internal().NotaryID(serialized.server());

    try {
        return api.Wallet().Internal().Server(serverID)->Signer();
    } catch (...) {
        LogError()(OT_PRETTY_STATIC(Reply))("Invalid server id.").Flush();

        return nullptr;
    }
}

auto Reply::Imp::full_version() const -> proto::ServerReply
{
    auto contract = signature_version();

    if (const auto sigs = signatures(); false == sigs.empty()) {
        contract.mutable_signature()->CopyFrom(*sigs.front());
    }

    return contract;
}

auto Reply::Imp::id_version() const -> proto::ServerReply
{
    proto::ServerReply output{};
    output.set_version(Version());
    output.clear_id();  // Must be blank
    output.set_type(translate(type_));
    recipient_.Internal().Serialize(*output.mutable_nym());
    server_.Internal().Serialize(*output.mutable_server());
    output.set_request(number_);
    output.set_success(success_);

    if (payload_) { *output.mutable_push() = *payload_; }

    output.clear_signature();  // Must be blank

    return output;
}

auto Reply::Imp::Serialize(Writer&& destination) const noexcept -> bool
{
    auto serialized = proto::ServerReply{};

    if (false == serialize(serialized)) { return false; }

    return serialize(serialized, std::move(destination));
}

auto Reply::Imp::Serialize(proto::ServerReply& output) const -> bool
{
    return serialize(output);
}

auto Reply::Imp::serialize(proto::ServerReply& output) const -> bool
{
    output = full_version();

    return true;
}

auto Reply::Imp::signature_version() const -> proto::ServerReply
{
    auto contract = id_version();
    ID().Internal().Serialize(*contract.mutable_id());

    return contract;
}

auto Reply::Imp::update_signature(const PasswordPrompt& reason) -> bool
{
    if (false == Signable::update_signature(reason)) { return false; }

    auto success = false;
    auto sigs = Signatures{};
    auto serialized = signature_version();
    auto& signature = *serialized.mutable_signature();
    success = Signer()->Internal().Sign(
        serialized, crypto::SignatureRole::ServerReply, signature, reason);

    if (success) {
        sigs.emplace_back(new proto::Signature(signature));
        add_signatures(std::move(sigs));
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to create signature.").Flush();
    }

    return success;
}

auto Reply::Imp::validate() const -> bool
{
    auto validNym{false};

    if (Signer()) { validNym = Signer()->VerifyPseudonym(); }

    if (false == validNym) {
        LogError()(OT_PRETTY_CLASS())("Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax = proto::Validate(full_version(), VERBOSE);

    if (false == validSyntax) {
        LogError()(OT_PRETTY_CLASS())("Invalid syntax.").Flush();

        return false;
    }

    const auto sigs = signatures();

    if (1_uz != sigs.size()) {
        LogError()(OT_PRETTY_CLASS())("Wrong number signatures.").Flush();

        return false;
    }

    auto validSig{false};

    if (const auto& signature = sigs.front(); signature) {
        validSig = verify_signature(*signature);
    }

    if (false == validSig) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature.").Flush();

        return false;
    }

    return true;
}

auto Reply::Imp::verify_signature(const proto::Signature& signature) const
    -> bool
{
    if (false == Signable::verify_signature(signature)) { return false; }

    auto serialized = signature_version();
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return Signer()->Internal().Verify(serialized, sigProto);
}
}  // namespace opentxs::otx
