// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/Request.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/request/Bailment.hpp"
#include "opentxs/core/contract/peer/request/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/request/Connection.hpp"
#include "opentxs/core/contract/peer/request/Faucet.hpp"
#include "opentxs/core/contract/peer/request/Outbailment.hpp"
#include "opentxs/core/contract/peer/request/StoreSecret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::contract::peer
{
auto operator==(const Request& lhs, const Request& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const Request& lhs, const Request& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}

auto swap(Request& lhs, Request& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer
{
Request::Request(RequestPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Request::Request(allocator_type alloc) noexcept
    : Request(RequestPrivate::Blank(alloc))
{
}

Request::Request(const Request& rhs, allocator_type alloc) noexcept
    : Request(rhs.imp_->clone(alloc))
{
}

Request::Request(Request&& rhs) noexcept
    : Request(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Request::Request(Request&& rhs, allocator_type alloc) noexcept
    : Request(alloc)
{
    operator=(std::move(rhs));
}

auto Request::Blank() noexcept -> Request&
{
    static auto blank = Request{allocator_type{alloc::Default()}};

    return blank;
}

auto Request::Alias() const noexcept -> UnallocatedCString
{
    return imp_->Alias();
}

auto Request::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return imp_->Alias(alloc);
}

auto Request::asBailment() const& noexcept -> const request::Bailment&
{
    return imp_->asBailmentPublic();
}

auto Request::asBailment() && noexcept -> request::Bailment
{
    return std::exchange(imp_, nullptr);
}

auto Request::asBailmentNotice() const& noexcept
    -> const request::BailmentNotice&
{
    return imp_->asBailmentNoticePublic();
}

auto Request::asBailmentNotice() && noexcept -> request::BailmentNotice
{
    return std::exchange(imp_, nullptr);
}

auto Request::asConnection() const& noexcept -> const request::Connection&
{
    return imp_->asConnectionPublic();
}

auto Request::asConnection() && noexcept -> request::Connection
{
    return std::exchange(imp_, nullptr);
}

auto Request::asFaucet() const& noexcept -> const request::Faucet&
{
    return imp_->asFaucetPublic();
}

auto Request::asFaucet() && noexcept -> request::Faucet
{
    return std::exchange(imp_, nullptr);
}

auto Request::asOutbailment() const& noexcept -> const request::Outbailment&
{
    return imp_->asOutbailmentPublic();
}

auto Request::asOutbailment() && noexcept -> request::Outbailment
{
    return std::exchange(imp_, nullptr);
}

auto Request::asStoreSecret() const& noexcept -> const request::StoreSecret&
{
    return imp_->asStoreSecretPublic();
}

auto Request::asStoreSecret() && noexcept -> request::StoreSecret
{
    return std::exchange(imp_, nullptr);
}

auto Request::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Request::ID() const noexcept -> const identifier_type&
{
    return imp_->ID();
}

auto Request::Initiator() const noexcept -> const identifier::Nym&
{
    return imp_->Initiator();
}

auto Request::Internal() const noexcept -> const internal::Request&
{
    return *imp_;
}

auto Request::Internal() noexcept -> internal::Request& { return *imp_; }

auto Request::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Request::Name() const noexcept -> std::string_view { return imp_->Name(); }

auto Request::operator=(const Request& rhs) noexcept -> Request&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Request::operator=(Request&& rhs) noexcept -> Request&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Request::Received() const noexcept -> Time { return imp_->Received(); }

auto Request::Responder() const noexcept -> const identifier::Nym&
{
    return imp_->Responder();
}

auto Request::Serialize(Writer&& out) const noexcept -> bool
{
    return imp_->Serialize(std::move(out));
}

auto Request::SetAlias(std::string_view alias) noexcept -> bool
{
    return imp_->SetAlias(alias);
}

auto Request::Signer() const noexcept -> Nym_p { return imp_->Signer(); }

auto Request::swap(Request& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Request::Terms() const noexcept -> std::string_view
{
    return imp_->Terms();
}

auto Request::Type() const noexcept -> RequestType { return imp_->Type(); }

auto Request::Validate() const noexcept -> bool { return imp_->Validate(); }

auto Request::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

Request::~Request() { pmr_delete(imp_); }
}  // namespace opentxs::contract::peer
