// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/base/RequestPrivate.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/request/bailment/BailmentPrivate.hpp"
#include "core/contract/peer/request/bailmentnotice/BailmentNoticePrivate.hpp"
#include "core/contract/peer/request/connection/ConnectionPrivate.hpp"
#include "core/contract/peer/request/faucet/FaucetPrivate.hpp"
#include "core/contract/peer/request/outbailment/OutbailmentPrivate.hpp"
#include "core/contract/peer/request/storesecret/StoreSecretPrivate.hpp"
#include "core/contract/peer/request/verification/VerificationPrivate.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/request/Bailment.hpp"
#include "opentxs/core/contract/peer/request/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/request/Connection.hpp"
#include "opentxs/core/contract/peer/request/Faucet.hpp"
#include "opentxs/core/contract/peer/request/Outbailment.hpp"
#include "opentxs/core/contract/peer/request/StoreSecret.hpp"
#include "opentxs/core/contract/peer/request/Verification.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"

namespace opentxs::contract::peer
{
RequestPrivate::RequestPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

RequestPrivate::RequestPrivate(
    const RequestPrivate&,
    allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

auto RequestPrivate::Alias() const noexcept -> UnallocatedCString { return {}; }

auto RequestPrivate::Alias(alloc::Strategy alloc) const noexcept -> CString
{
    return CString{alloc.result_};
}

auto RequestPrivate::asBailmentNoticePrivate() const& noexcept
    -> const request::BailmentNoticePrivate*
{
    static auto blank = request::BailmentNoticePrivate{alloc::System()};

    return std::addressof(blank);
}

auto RequestPrivate::asBailmentNoticePublic() const& noexcept
    -> const request::BailmentNotice&
{
    return request::BailmentNotice::Blank();
}

auto RequestPrivate::asBailmentPrivate() const& noexcept
    -> const request::BailmentPrivate*
{
    static auto blank = request::BailmentPrivate{alloc::System()};

    return std::addressof(blank);
}

auto RequestPrivate::asBailmentPublic() const& noexcept
    -> const request::Bailment&
{
    return request::Bailment::Blank();
}

auto RequestPrivate::asConnectionPrivate() const& noexcept
    -> const request::ConnectionPrivate*
{
    static auto blank = request::ConnectionPrivate{alloc::System()};

    return std::addressof(blank);
}

auto RequestPrivate::asConnectionPublic() const& noexcept
    -> const request::Connection&
{
    return request::Connection::Blank();
}

auto RequestPrivate::asFaucetPrivate() const& noexcept
    -> const request::FaucetPrivate*
{
    static auto blank = request::FaucetPrivate{alloc::System()};

    return std::addressof(blank);
}

auto RequestPrivate::asFaucetPublic() const& noexcept -> const request::Faucet&
{
    return request::Faucet::Blank();
}

auto RequestPrivate::asOutbailmentPrivate() const& noexcept
    -> const request::OutbailmentPrivate*
{
    static auto blank = request::OutbailmentPrivate{alloc::System()};

    return std::addressof(blank);
}

auto RequestPrivate::asOutbailmentPublic() const& noexcept
    -> const request::Outbailment&
{
    return request::Outbailment::Blank();
}

auto RequestPrivate::asStoreSecretPrivate() const& noexcept
    -> const request::StoreSecretPrivate*
{
    static auto blank = request::StoreSecretPrivate{alloc::System()};

    return std::addressof(blank);
}

auto RequestPrivate::asStoreSecretPublic() const& noexcept
    -> const request::StoreSecret&
{
    return request::StoreSecret::Blank();
}

auto RequestPrivate::asVerificationPrivate() const& noexcept
    -> const request::VerificationPrivate*
{
    static auto blank = request::VerificationPrivate{alloc::System()};

    return std::addressof(blank);
}

auto RequestPrivate::asVerificationPublic() const& noexcept
    -> const request::Verification&
{
    return request::Verification::Blank();
}

auto RequestPrivate::ID() const noexcept -> const identifier_type&
{
    static const auto blank = identifier_type{};

    return blank;
}

auto RequestPrivate::Initiator() const noexcept -> const identifier::Nym&
{
    static const auto blank = identifier::Nym{};

    return blank;
}

auto RequestPrivate::IsValid() const noexcept -> bool { return false; }

auto RequestPrivate::Name() const noexcept -> std::string_view { return {}; }

auto RequestPrivate::Received() const noexcept -> Time { return {}; }

auto RequestPrivate::Reset(peer::Request& request) noexcept -> void
{
    request.imp_ = nullptr;
}

auto RequestPrivate::Responder() const noexcept -> const identifier::Nym&
{
    return Initiator();
}

auto RequestPrivate::Serialize(Writer&&) const noexcept -> bool { return {}; }

auto RequestPrivate::SetAlias(std::string_view) noexcept -> bool { return {}; }

auto RequestPrivate::Signer() const noexcept -> Nym_p { return {}; }

auto RequestPrivate::Terms() const noexcept -> std::string_view { return {}; }

auto RequestPrivate::Type() const noexcept -> RequestType { return {}; }

auto RequestPrivate::Validate() const noexcept -> bool { return {}; }

auto RequestPrivate::Version() const noexcept -> VersionNumber { return {}; }

RequestPrivate::~RequestPrivate() = default;
}  // namespace opentxs::contract::peer
