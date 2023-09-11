// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/core/contract/peer/Reply.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
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
}  // namespace reply

class Reply;
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
class ReplyPrivate : virtual public internal::Reply,
                     virtual public contract::Signable<identifier::Generic>,
                     public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> ReplyPrivate*
    {
        return pmr::default_construct<ReplyPrivate>(
            alloc::PMR<ReplyPrivate>{alloc});
    }
    static auto Reset(peer::Reply& reply) noexcept -> void;

    [[nodiscard]] auto Alias() const noexcept -> UnallocatedCString override;
    [[nodiscard]] auto Alias(alloc::Strategy alloc) const noexcept
        -> CString override;
    [[nodiscard]] virtual auto asBailmentNoticePrivate() const& noexcept
        -> const reply::BailmentNoticePrivate*;
    [[nodiscard]] virtual auto asBailmentNoticePublic() const& noexcept
        -> const reply::BailmentNotice&;
    [[nodiscard]] virtual auto asBailmentPrivate() const& noexcept
        -> const reply::BailmentPrivate*;
    [[nodiscard]] virtual auto asBailmentPublic() const& noexcept
        -> const reply::Bailment&;
    [[nodiscard]] virtual auto asConnectionPrivate() const& noexcept
        -> const reply::ConnectionPrivate*;
    [[nodiscard]] virtual auto asConnectionPublic() const& noexcept
        -> const reply::Connection&;
    [[nodiscard]] virtual auto asFaucetPrivate() const& noexcept
        -> const reply::FaucetPrivate*;
    [[nodiscard]] virtual auto asFaucetPublic() const& noexcept
        -> const reply::Faucet&;
    [[nodiscard]] virtual auto asOutbailmentPrivate() const& noexcept
        -> const reply::OutbailmentPrivate*;
    [[nodiscard]] virtual auto asOutbailmentPublic() const& noexcept
        -> const reply::Outbailment&;
    [[nodiscard]] virtual auto asStoreSecretPrivate() const& noexcept
        -> const reply::StoreSecretPrivate*;
    [[nodiscard]] virtual auto asStoreSecretPublic() const& noexcept
        -> const reply::StoreSecret&;
    [[nodiscard]] virtual auto asVerificationPrivate() const& noexcept
        -> const reply::VerificationPrivate*;
    [[nodiscard]] virtual auto asVerificationPublic() const& noexcept
        -> const reply::Verification&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> ReplyPrivate*
    {
        return pmr::clone(this, alloc::PMR<ReplyPrivate>{alloc});
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    [[nodiscard]] auto ID() const noexcept -> const identifier_type& override;
    [[nodiscard]] virtual auto Initiator() const noexcept
        -> const identifier::Nym&;
    [[nodiscard]] virtual auto InReferenceToRequest() const noexcept
        -> const identifier_type&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Name() const noexcept -> std::string_view override;
    [[nodiscard]] auto Signer() const noexcept -> Nym_p override;
    [[nodiscard]] virtual auto Received() const noexcept -> Time;
    [[nodiscard]] virtual auto Responder() const noexcept
        -> const identifier::Nym&;
    using internal::Reply::Serialize;
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool override;
    [[nodiscard]] auto Terms() const noexcept -> std::string_view override;
    [[nodiscard]] virtual auto Type() const noexcept -> RequestType;
    [[nodiscard]] auto Validate() const noexcept -> bool override;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber override;

    [[nodiscard]] auto SetAlias(std::string_view alias) noexcept
        -> bool override;

    ReplyPrivate(allocator_type alloc) noexcept;
    ReplyPrivate() = delete;
    ReplyPrivate(const ReplyPrivate& rhs, allocator_type alloc) noexcept;
    ReplyPrivate(const ReplyPrivate&) = delete;
    ReplyPrivate(ReplyPrivate&&) = delete;
    auto operator=(const ReplyPrivate&) -> ReplyPrivate& = delete;
    auto operator=(ReplyPrivate&&) -> ReplyPrivate& = delete;

    ~ReplyPrivate() override;
};
}  // namespace opentxs::contract::peer
