// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/Reply.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/OTXPush.pb.h>
#include <opentxs/protobuf/ServerReply.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <span>
#include <utility>

#include "core/contract/Signable.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/protobuf/Types.internal.tpp"
#include "opentxs/protobuf/syntax/ServerReply.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::otx
{
const VersionNumber Reply::DefaultVersion{1};
const VersionNumber Reply::MaxVersion{1};

static auto construct_push(PushType pushtype, const UnallocatedCString& payload)
    -> std::shared_ptr<protobuf::OTXPush>
{
    auto pPush = std::make_shared<protobuf::OTXPush>();
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
    std::shared_ptr<const protobuf::OTXPush>&& push) -> Reply
{
    assert_false(nullptr == signer);

    auto output{std::make_unique<Reply::Imp>(
        api,
        signer,
        recipient,
        server,
        type,
        number,
        success,
        std::move(push))};

    assert_false(nullptr == output);

    output->update_signature(reason);

    assert_false(output->ID().empty());

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
    opentxs::otx::PushType pushtype,
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
    const protobuf::ServerReply serialized) -> Reply
{
    return Reply{new Reply::Imp(api, serialized)};
}

auto Reply::Factory(const api::Session& api, const ReadView& view) -> Reply
{
    return Reply{
        new Reply::Imp(api, protobuf::Factory<protobuf::ServerReply>(view))};
}

auto Reply::Number() const -> RequestNumber { return imp_->Number(); }

auto Reply::Push() const -> std::shared_ptr<const protobuf::OTXPush>
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

auto Reply::Serialize(protobuf::ServerReply& serialized) const -> bool
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
    assert_false(nullptr == imp);
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
    std::shared_ptr<const protobuf::OTXPush>&& push)
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
Reply::Imp::Imp(const api::Session& api, const protobuf::ServerReply serialized)
    : Signable(
          api,
          extract_nym(api, serialized),
          serialized.version(),
          "",
          "",
          "",
          api.Factory().Internal().Identifier(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<protobuf::Signature>(
                    serialized.signature())}
              : Signatures{})
    , recipient_(api_.Factory().Internal().NymID(serialized.nym()))
    , server_(api_.Factory().Internal().NotaryID(serialized.server()))
    , type_(translate(serialized.type()))
    , success_(serialized.success())
    , number_(serialized.request())
    , payload_(
          serialized.has_push()
              ? std::make_shared<protobuf::OTXPush>(serialized.push())
              : std::shared_ptr<protobuf::OTXPush>{})
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
    return api_.Factory().Internal().Session().IdentifierFromPreimage(
        id_version());
}

auto Reply::Imp::extract_nym(
    const api::Session& api,
    const protobuf::ServerReply serialized) -> Nym_p
{
    const auto serverID =
        api.Factory().Internal().NotaryID(serialized.server());

    try {
        return api.Wallet().Internal().Server(serverID)->Signer();
    } catch (...) {
        LogError()()("Invalid server id.").Flush();

        return nullptr;
    }
}

auto Reply::Imp::full_version() const -> protobuf::ServerReply
{
    auto contract = signature_version();

    if (const auto sigs = signatures(); false == sigs.empty()) {
        contract.mutable_signature()->CopyFrom(*sigs.front());
    }

    return contract;
}

auto Reply::Imp::id_version() const -> protobuf::ServerReply
{
    protobuf::ServerReply output{};
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
    auto serialized = protobuf::ServerReply{};

    if (false == serialize(serialized)) { return false; }

    return serialize(serialized, std::move(destination));
}

auto Reply::Imp::Serialize(protobuf::ServerReply& output) const -> bool
{
    return serialize(output);
}

auto Reply::Imp::serialize(protobuf::ServerReply& output) const -> bool
{
    output = full_version();

    return true;
}

auto Reply::Imp::signature_version() const -> protobuf::ServerReply
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
        sigs.emplace_back(new protobuf::Signature(signature));
        add_signatures(std::move(sigs));
    } else {
        LogError()()("Failed to create signature.").Flush();
    }

    return success;
}

auto Reply::Imp::validate() const -> bool
{
    auto validNym{false};

    if (Signer()) { validNym = Signer()->VerifyPseudonym(); }

    if (false == validNym) {
        LogError()()("Invalid nym.").Flush();

        return false;
    }

    const bool validSyntax =
        protobuf::syntax::check(LogError(), full_version());

    if (false == validSyntax) {
        LogError()()("Invalid syntax.").Flush();

        return false;
    }

    const auto sigs = signatures();

    if (1_uz != sigs.size()) {
        LogError()()("Wrong number signatures.").Flush();

        return false;
    }

    auto validSig{false};

    if (const auto& signature = sigs.front(); signature) {
        validSig = verify_signature(*signature);
    }

    if (false == validSig) {
        LogError()()("Invalid signature.").Flush();

        return false;
    }

    return true;
}

auto Reply::Imp::verify_signature(const protobuf::Signature& signature) const
    -> bool
{
    if (false == Signable::verify_signature(signature)) { return false; }

    auto serialized = signature_version();
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return Signer()->Internal().Verify(serialized, sigProto);
}
}  // namespace opentxs::otx
