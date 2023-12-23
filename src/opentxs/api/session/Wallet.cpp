// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/Wallet.hpp"  // IWYU pragma: associated

#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/NymEditor.hpp"

namespace opentxs::api::session
{
using namespace std::literals;

Wallet::Wallet(internal::Wallet* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp);
}

auto Wallet::AccountPartialMatch(const UnallocatedCString& hint) const
    -> identifier::Generic
{
    return imp_->AccountPartialMatch(hint);
}

auto Wallet::DefaultNym() const noexcept
    -> std::pair<identifier::Nym, std::size_t>
{
    return imp_->DefaultNym();
}

auto Wallet::DeleteAccount(const identifier::Account& accountID) const -> bool
{
    return imp_->DeleteAccount(accountID);
}

auto Wallet::Internal() const noexcept -> const internal::Wallet&
{
    return *imp_;
}

auto Wallet::Internal() noexcept -> internal::Wallet& { return *imp_; }

auto Wallet::IsLocalNym(const identifier::Nym& id) const -> bool
{
    return imp_->IsLocalNym(id);
}

auto Wallet::IsLocalNym(const std::string_view id) const -> bool
{
    return imp_->IsLocalNym(id);
}

auto Wallet::IssuerList(const identifier::Nym& nymID) const
    -> UnallocatedSet<identifier::Nym>
{
    return imp_->IssuerList(nymID);
}

auto Wallet::LocalNymCount() const -> std::size_t
{
    return imp_->LocalNymCount();
}

auto Wallet::LocalNyms() const -> Set<identifier::Nym>
{
    return imp_->LocalNyms();
}

auto Wallet::Nym(const PasswordPrompt& reason, const UnallocatedCString& name)
    const -> Nym_p
{
    return imp_->Nym(reason, name);
}

auto Wallet::Nym(const ReadView& bytes) const -> Nym_p
{
    return imp_->Nym(bytes);
}

auto Wallet::Nym(const identifier::Nym& id) const -> Nym_p
{
    return Nym(id, 0ms);
}

auto Wallet::Nym(const identifier::Nym& id, std::chrono::milliseconds timeout)
    const -> Nym_p
{
    return imp_->Nym(id, timeout);
}

auto Wallet::Nym(
    const identity::Type type,
    const PasswordPrompt& reason,
    const UnallocatedCString& name) const -> Nym_p
{
    return imp_->Nym(type, reason, name);
}

auto Wallet::Nym(
    const opentxs::crypto::Parameters& parameters,
    const PasswordPrompt& reason,
    const UnallocatedCString& name) const -> Nym_p
{
    return imp_->Nym(parameters, reason, name);
}

auto Wallet::Nym(
    const opentxs::crypto::Parameters& parameters,
    const identity::Type type,
    const PasswordPrompt& reason,
    const UnallocatedCString& name) const -> Nym_p
{
    return imp_->Nym(parameters, type, reason, name);
}

auto Wallet::NymByIDPartialMatch(const UnallocatedCString& partialId) const
    -> Nym_p
{
    return imp_->NymByIDPartialMatch(partialId);
}

auto Wallet::NymList() const -> ObjectList { return imp_->NymList(); }

auto Wallet::NymNameByIndex(const std::size_t index, String& name) const -> bool
{
    return imp_->NymNameByIndex(index, name);
}

auto Wallet::PeerReply(
    const identifier::Nym& nym,
    const identifier::Generic& reply,
    otx::client::StorageBox box,
    alloc::Strategy alloc) const noexcept -> contract::peer::Reply
{
    return imp_->PeerReply(nym, reply, box, alloc);
}

auto Wallet::PeerReplyComplete(
    const identifier::Nym& nym,
    const identifier::Generic& replyOrRequest) const -> bool
{
    return imp_->PeerReplyComplete(nym, replyOrRequest);
}

auto Wallet::PeerReplyCreateRollback(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const identifier::Generic& reply) const -> bool
{
    return imp_->PeerReplyCreateRollback(nym, request, reply);
}

auto Wallet::PeerReplyFinished(const identifier::Nym& nym) const -> ObjectList
{
    return imp_->PeerReplyFinished(nym);
}

auto Wallet::PeerReplyIncoming(const identifier::Nym& nym) const -> ObjectList
{
    return imp_->PeerReplyIncoming(nym);
}

auto Wallet::PeerReplyProcessed(const identifier::Nym& nym) const -> ObjectList
{
    return imp_->PeerReplyProcessed(nym);
}

auto Wallet::PeerReplyReceive(
    const identifier::Nym& nym,
    const PeerObject& reply) const -> bool
{
    return imp_->PeerReplyReceive(nym, reply);
}

auto Wallet::PeerReplySent(const identifier::Nym& nym) const -> ObjectList
{
    return imp_->PeerReplySent(nym);
}

auto Wallet::PeerRequest(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const otx::client::StorageBox& box,
    alloc::Strategy alloc) const noexcept -> contract::peer::Request
{
    return imp_->PeerRequest(nym, request, box, alloc);
}

auto Wallet::PeerRequestComplete(
    const identifier::Nym& nym,
    const identifier::Generic& reply) const -> bool
{
    return imp_->PeerRequestComplete(nym, reply);
}

auto Wallet::PeerRequestCreateRollback(
    const identifier::Nym& nym,
    const identifier::Generic& request) const -> bool
{
    return imp_->PeerRequestCreateRollback(nym, request);
}

auto Wallet::PeerRequestDelete(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const otx::client::StorageBox& box) const -> bool
{
    return imp_->PeerRequestDelete(nym, request, box);
}

auto Wallet::PeerRequestFinished(const identifier::Nym& nym) const -> ObjectList
{
    return imp_->PeerRequestFinished(nym);
}

auto Wallet::PeerRequestIncoming(const identifier::Nym& nym) const -> ObjectList
{
    return imp_->PeerRequestIncoming(nym);
}

auto Wallet::PeerRequestProcessed(const identifier::Nym& nym) const
    -> ObjectList
{
    return imp_->PeerRequestProcessed(nym);
}

auto Wallet::PeerRequestReceive(
    const identifier::Nym& nym,
    const PeerObject& request) const -> bool
{
    return imp_->PeerRequestReceive(nym, request);
}

auto Wallet::PeerRequestSent(const identifier::Nym& nym) const -> ObjectList
{
    return imp_->PeerRequestSent(nym);
}

auto Wallet::PeerRequestUpdate(
    const identifier::Nym& nym,
    const identifier::Generic& request,
    const otx::client::StorageBox& box) const -> bool
{
    return imp_->PeerRequestUpdate(nym, request, box);
}

auto Wallet::Purse(
    const identifier::Nym& nym,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const bool checking) const -> const otx::blind::Purse&
{
    return imp_->Purse(nym, server, unit, checking);
}

auto Wallet::RemoveServer(const identifier::Notary& id) const -> bool
{
    return imp_->RemoveServer(id);
}

auto Wallet::RemoveUnitDefinition(const identifier::UnitDefinition& id) const
    -> bool
{
    return imp_->RemoveUnitDefinition(id);
}

auto Wallet::ServerList() const -> ObjectList { return imp_->ServerList(); }

auto Wallet::SetDefaultNym(const identifier::Nym& id) const noexcept -> bool
{
    return imp_->SetDefaultNym(id);
}

auto Wallet::SetNymAlias(const identifier::Nym& id, std::string_view alias)
    const -> bool
{
    return imp_->SetNymAlias(id, alias);
}

auto Wallet::SetServerAlias(
    const identifier::Notary& id,
    std::string_view alias) const -> bool
{
    return imp_->SetServerAlias(id, alias);
}

auto Wallet::SetUnitDefinitionAlias(
    const identifier::UnitDefinition& id,
    std::string_view alias) const -> bool
{
    return imp_->SetUnitDefinitionAlias(id, alias);
}

auto Wallet::UnitDefinitionList() const -> ObjectList
{
    return imp_->UnitDefinitionList();
}

auto Wallet::mutable_Nym(
    const identifier::Nym& id,
    const PasswordPrompt& reason) const -> NymData
{
    return imp_->mutable_Nym(id, reason);
}

Wallet::~Wallet()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::session
