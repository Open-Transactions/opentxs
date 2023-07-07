// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/Reply.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/reply/Bailment.hpp"
#include "opentxs/core/contract/peer/reply/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/reply/Connection.hpp"
#include "opentxs/core/contract/peer/reply/Faucet.hpp"
#include "opentxs/core/contract/peer/reply/Outbailment.hpp"
#include "opentxs/core/contract/peer/reply/StoreSecret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::contract::peer
{
auto operator==(const Reply& lhs, const Reply& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const Reply& lhs, const Reply& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}

auto swap(Reply& lhs, Reply& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer
{
Reply::Reply(ReplyPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Reply::Reply(allocator_type alloc) noexcept
    : Reply(ReplyPrivate::Blank(alloc))
{
}

Reply::Reply(const Reply& rhs, allocator_type alloc) noexcept
    : Reply(rhs.imp_->clone(alloc))
{
}

Reply::Reply(Reply&& rhs) noexcept
    : Reply(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Reply::Reply(Reply&& rhs, allocator_type alloc) noexcept
    : Reply(alloc)
{
    operator=(std::move(rhs));
}

auto Reply::Blank() noexcept -> Reply&
{
    static auto blank = Reply{allocator_type{alloc::Default()}};

    return blank;
}

auto Reply::Alias() const noexcept -> UnallocatedCString
{
    return imp_->Alias();
}

auto Reply::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return imp_->Alias(alloc);
}

auto Reply::asBailment() const& noexcept -> const reply::Bailment&
{
    return imp_->asBailmentPublic();
}

auto Reply::asBailment() && noexcept -> reply::Bailment
{
    return std::exchange(imp_, nullptr);
}

auto Reply::asBailmentNotice() const& noexcept -> const reply::BailmentNotice&
{
    return imp_->asBailmentNoticePublic();
}

auto Reply::asBailmentNotice() && noexcept -> reply::BailmentNotice
{
    return std::exchange(imp_, nullptr);
}

auto Reply::asConnection() const& noexcept -> const reply::Connection&
{
    return imp_->asConnectionPublic();
}

auto Reply::asConnection() && noexcept -> reply::Connection
{
    return std::exchange(imp_, nullptr);
}

auto Reply::asFaucet() const& noexcept -> const reply::Faucet&
{
    return imp_->asFaucetPublic();
}

auto Reply::asFaucet() && noexcept -> reply::Faucet
{
    return std::exchange(imp_, nullptr);
}

auto Reply::asOutbailment() const& noexcept -> const reply::Outbailment&
{
    return imp_->asOutbailmentPublic();
}

auto Reply::asOutbailment() && noexcept -> reply::Outbailment
{
    return std::exchange(imp_, nullptr);
}

auto Reply::asStoreSecret() const& noexcept -> const reply::StoreSecret&
{
    return imp_->asStoreSecretPublic();
}

auto Reply::asStoreSecret() && noexcept -> reply::StoreSecret
{
    return std::exchange(imp_, nullptr);
}

auto Reply::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Reply::ID() const noexcept -> const identifier_type& { return imp_->ID(); }

auto Reply::Initiator() const noexcept -> const identifier::Nym&
{
    return imp_->Initiator();
}

auto Reply::InReferenceToRequest() const noexcept -> const identifier_type&
{
    return imp_->InReferenceToRequest();
}

auto Reply::Internal() const noexcept -> const internal::Reply&
{
    return *imp_;
}

auto Reply::Internal() noexcept -> internal::Reply& { return *imp_; }

auto Reply::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Reply::Name() const noexcept -> std::string_view { return imp_->Name(); }

auto Reply::operator=(const Reply& rhs) noexcept -> Reply&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Reply::operator=(Reply&& rhs) noexcept -> Reply&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Reply::Received() const noexcept -> Time { return imp_->Received(); }

auto Reply::Responder() const noexcept -> const identifier::Nym&
{
    return imp_->Responder();
}

auto Reply::Serialize(Writer&& out) const noexcept -> bool
{
    return imp_->Serialize(std::move(out));
}

auto Reply::SetAlias(std::string_view alias) noexcept -> bool
{
    return imp_->SetAlias(alias);
}

auto Reply::Signer() const noexcept -> Nym_p { return imp_->Signer(); }

auto Reply::swap(Reply& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Reply::Terms() const noexcept -> std::string_view { return imp_->Terms(); }

auto Reply::Type() const noexcept -> RequestType { return imp_->Type(); }

auto Reply::Validate() const noexcept -> bool { return imp_->Validate(); }

auto Reply::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

Reply::~Reply() { pmr_delete(imp_); }
}  // namespace opentxs::contract::peer
