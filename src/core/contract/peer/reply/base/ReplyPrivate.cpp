// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/reply/bailment/BailmentPrivate.hpp"
#include "core/contract/peer/reply/bailmentnotice/BailmentNoticePrivate.hpp"
#include "core/contract/peer/reply/connection/ConnectionPrivate.hpp"
#include "core/contract/peer/reply/faucet/FaucetPrivate.hpp"
#include "core/contract/peer/reply/outbailment/OutbailmentPrivate.hpp"
#include "core/contract/peer/reply/storesecret/StoreSecretPrivate.hpp"
#include "core/contract/peer/reply/verification/VerificationPrivate.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/reply/Bailment.hpp"
#include "opentxs/core/contract/peer/reply/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/reply/Connection.hpp"
#include "opentxs/core/contract/peer/reply/Faucet.hpp"
#include "opentxs/core/contract/peer/reply/Outbailment.hpp"
#include "opentxs/core/contract/peer/reply/StoreSecret.hpp"
#include "opentxs/core/contract/peer/reply/Verification.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs::contract::peer
{
ReplyPrivate::ReplyPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

ReplyPrivate::ReplyPrivate(const ReplyPrivate&, allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

auto ReplyPrivate::Alias() const noexcept -> UnallocatedCString { return {}; }

auto ReplyPrivate::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return CString{alloc.result_};
}

auto ReplyPrivate::asBailmentNoticePrivate() const& noexcept
    -> const reply::BailmentNoticePrivate*
{
    static auto blank = reply::BailmentNoticePrivate{alloc::System()};

    return std::addressof(blank);
}

auto ReplyPrivate::asBailmentNoticePublic() const& noexcept
    -> const reply::BailmentNotice&
{
    return reply::BailmentNotice::Blank();
}

auto ReplyPrivate::asBailmentPrivate() const& noexcept
    -> const reply::BailmentPrivate*
{
    static auto blank = reply::BailmentPrivate{alloc::System()};

    return std::addressof(blank);
}

auto ReplyPrivate::asBailmentPublic() const& noexcept -> const reply::Bailment&
{
    return reply::Bailment::Blank();
}

auto ReplyPrivate::asConnectionPrivate() const& noexcept
    -> const reply::ConnectionPrivate*
{
    static auto blank = reply::ConnectionPrivate{alloc::System()};

    return std::addressof(blank);
}

auto ReplyPrivate::asConnectionPublic() const& noexcept
    -> const reply::Connection&
{
    return reply::Connection::Blank();
}

auto ReplyPrivate::asFaucetPrivate() const& noexcept
    -> const reply::FaucetPrivate*
{
    static auto blank = reply::FaucetPrivate{alloc::System()};

    return std::addressof(blank);
}

auto ReplyPrivate::asFaucetPublic() const& noexcept -> const reply::Faucet&
{
    return reply::Faucet::Blank();
}

auto ReplyPrivate::asOutbailmentPrivate() const& noexcept
    -> const reply::OutbailmentPrivate*
{
    static auto blank = reply::OutbailmentPrivate{alloc::System()};

    return std::addressof(blank);
}

auto ReplyPrivate::asOutbailmentPublic() const& noexcept
    -> const reply::Outbailment&
{
    return reply::Outbailment::Blank();
}

auto ReplyPrivate::asStoreSecretPrivate() const& noexcept
    -> const reply::StoreSecretPrivate*
{
    static auto blank = reply::StoreSecretPrivate{alloc::System()};

    return std::addressof(blank);
}

auto ReplyPrivate::asStoreSecretPublic() const& noexcept
    -> const reply::StoreSecret&
{
    return reply::StoreSecret::Blank();
}

auto ReplyPrivate::asVerificationPrivate() const& noexcept
    -> const reply::VerificationPrivate*
{
    static auto blank = reply::VerificationPrivate{alloc::System()};

    return std::addressof(blank);
}

auto ReplyPrivate::asVerificationPublic() const& noexcept
    -> const reply::Verification&
{
    return reply::Verification::Blank();
}

auto ReplyPrivate::ID() const noexcept -> const identifier_type&
{
    static const auto blank = identifier_type{};

    return blank;
}

auto ReplyPrivate::Initiator() const noexcept -> const identifier::Nym&
{
    static const auto blank = identifier::Nym{};

    return blank;
}

auto ReplyPrivate::InReferenceToRequest() const noexcept
    -> const identifier_type&
{
    return ID();
}

auto ReplyPrivate::IsValid() const noexcept -> bool { return false; }

auto ReplyPrivate::Name() const noexcept -> std::string_view { return {}; }

auto ReplyPrivate::Received() const noexcept -> Time { return {}; }

auto ReplyPrivate::Reset(peer::Reply& reply) noexcept -> void
{
    reply.imp_ = nullptr;
}

auto ReplyPrivate::Responder() const noexcept -> const identifier::Nym&
{
    return Initiator();
}

auto ReplyPrivate::Serialize(Writer&&) const noexcept -> bool { return {}; }

auto ReplyPrivate::SetAlias(std::string_view) noexcept -> bool { return {}; }

auto ReplyPrivate::Signer() const noexcept -> Nym_p { return {}; }

auto ReplyPrivate::Terms() const noexcept -> std::string_view { return {}; }

auto ReplyPrivate::Type() const noexcept -> RequestType { return {}; }

auto ReplyPrivate::Validate() const noexcept -> bool { return {}; }

auto ReplyPrivate::Version() const noexcept -> VersionNumber { return {}; }

ReplyPrivate::~ReplyPrivate() = default;
}  // namespace opentxs::contract::peer
