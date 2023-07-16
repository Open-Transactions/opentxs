// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/core/contract/peer/Request.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
class Bailment;
class BailmentNotice;
class BailmentNoticePrivate;
class BailmentPrivate;
class Connection;
class ConnectionPrivate;
class Faucet;
class FaucetPrivate;
class Outbailment;
class OutbailmentPrivate;
class StoreSecret;
class StoreSecretPrivate;
class Verification;
class VerificationPrivate;
}  // namespace request

class Request;
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer
{
class RequestPrivate : virtual public internal::Request,
                       virtual public contract::Signable<identifier::Generic>,
                       public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> RequestPrivate*
    {
        return pmr::default_construct<RequestPrivate>(
            alloc::PMR<RequestPrivate>{alloc});
    }
    static auto Reset(peer::Request& request) noexcept -> void;

    [[nodiscard]] auto Alias() const noexcept -> UnallocatedCString override;
    [[nodiscard]] auto Alias(alloc::Strategy alloc) const noexcept
        -> CString override;
    [[nodiscard]] virtual auto asBailmentNoticePrivate() const& noexcept
        -> const request::BailmentNoticePrivate*;
    [[nodiscard]] virtual auto asBailmentNoticePublic() const& noexcept
        -> const request::BailmentNotice&;
    [[nodiscard]] virtual auto asBailmentPrivate() const& noexcept
        -> const request::BailmentPrivate*;
    [[nodiscard]] virtual auto asBailmentPublic() const& noexcept
        -> const request::Bailment&;
    [[nodiscard]] virtual auto asConnectionPrivate() const& noexcept
        -> const request::ConnectionPrivate*;
    [[nodiscard]] virtual auto asConnectionPublic() const& noexcept
        -> const request::Connection&;
    [[nodiscard]] virtual auto asFaucetPrivate() const& noexcept
        -> const request::FaucetPrivate*;
    [[nodiscard]] virtual auto asFaucetPublic() const& noexcept
        -> const request::Faucet&;
    [[nodiscard]] virtual auto asOutbailmentPrivate() const& noexcept
        -> const request::OutbailmentPrivate*;
    [[nodiscard]] virtual auto asOutbailmentPublic() const& noexcept
        -> const request::Outbailment&;
    [[nodiscard]] virtual auto asStoreSecretPrivate() const& noexcept
        -> const request::StoreSecretPrivate*;
    [[nodiscard]] virtual auto asStoreSecretPublic() const& noexcept
        -> const request::StoreSecret&;
    [[nodiscard]] virtual auto asVerificationPrivate() const& noexcept
        -> const request::VerificationPrivate*;
    [[nodiscard]] virtual auto asVerificationPublic() const& noexcept
        -> const request::Verification&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> RequestPrivate*
    {
        return pmr::clone(this, alloc::PMR<RequestPrivate>{alloc});
    }
    [[nodiscard]] auto ID() const noexcept -> const identifier_type& override;
    [[nodiscard]] virtual auto Initiator() const noexcept
        -> const identifier::Nym&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Name() const noexcept -> std::string_view final;
    [[nodiscard]] virtual auto Received() const noexcept -> Time;
    [[nodiscard]] virtual auto Responder() const noexcept
        -> const identifier::Nym&;
    using internal::Request::Serialize;
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool override;
    [[nodiscard]] auto Signer() const noexcept -> Nym_p override;
    [[nodiscard]] auto Terms() const noexcept -> std::string_view final;
    [[nodiscard]] virtual auto Type() const noexcept -> RequestType;
    [[nodiscard]] auto Validate() const noexcept -> bool override;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber override;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    [[nodiscard]] auto SetAlias(std::string_view alias) noexcept
        -> bool override;

    RequestPrivate(allocator_type alloc) noexcept;
    RequestPrivate() = delete;
    RequestPrivate(const RequestPrivate& rhs, allocator_type alloc) noexcept;
    RequestPrivate(const RequestPrivate&) = delete;
    RequestPrivate(RequestPrivate&&) = delete;
    auto operator=(const RequestPrivate&) -> RequestPrivate& = delete;
    auto operator=(RequestPrivate&&) -> RequestPrivate& = delete;

    ~RequestPrivate() override;
};
}  // namespace opentxs::contract::peer
