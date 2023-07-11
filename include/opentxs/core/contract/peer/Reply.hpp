// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <string_view>
#include <variant>

#include "opentxs/Export.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocated.hpp"
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
namespace internal
{
class Reply;
}  // namespace internal

namespace reply
{
class Bailment;
class BailmentNotice;
class Connection;
class Faucet;
class Outbailment;
class StoreSecret;
}  // namespace reply

class Reply;
class ReplyPrivate;
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

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::contract::peer::Reply> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::contract::peer::Reply&) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::contract::peer::Reply> {
    auto operator()(
        const opentxs::contract::peer::Reply& lhs,
        const opentxs::contract::peer::Reply& rhs) const noexcept -> bool;
};
}  // namespace std

namespace opentxs::contract::peer
{
OPENTXS_EXPORT auto operator==(const Reply& lhs, const Reply& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Reply& lhs, const Reply& rhs) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Reply& lhs, Reply& rhs) noexcept -> void;
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer
{
class OPENTXS_EXPORT Reply : virtual public Signable<identifier::Generic>,
                             virtual public Allocated
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Reply&;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    [[nodiscard]] auto Alias() const noexcept -> UnallocatedCString final;
    [[nodiscard]] auto Alias(alloc::Strategy alloc) const noexcept
        -> CString final;
    [[nodiscard]] auto asBailment() const& noexcept -> const reply::Bailment&;
    [[nodiscard]] auto asBailmentNotice() const& noexcept
        -> const reply::BailmentNotice&;
    [[nodiscard]] auto asConnection() const& noexcept
        -> const reply::Connection&;
    [[nodiscard]] auto asFaucet() const& noexcept -> const reply::Faucet&;
    [[nodiscard]] auto asOutbailment() const& noexcept
        -> const reply::Outbailment&;
    [[nodiscard]] auto asStoreSecret() const& noexcept
        -> const reply::StoreSecret&;
    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type final;
    [[nodiscard]] auto ID() const noexcept -> const identifier_type& final;
    [[nodiscard]] auto Initiator() const noexcept -> const identifier::Nym&;
    [[nodiscard]] auto InReferenceToRequest() const noexcept
        -> const identifier_type&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Reply&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Name() const noexcept -> std::string_view final;
    [[nodiscard]] auto Received() const noexcept -> Time;
    [[nodiscard]] auto Responder() const noexcept -> const identifier::Nym&;
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool final;
    [[nodiscard]] auto Signer() const noexcept -> Nym_p final;
    [[nodiscard]] auto Terms() const noexcept -> std::string_view final;
    [[nodiscard]] auto Type() const noexcept -> RequestType;
    [[nodiscard]] auto Validate() const noexcept -> bool final;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber final;

    [[nodiscard]] auto asBailment() && noexcept -> reply::Bailment;
    [[nodiscard]] auto asBailmentNotice() && noexcept -> reply::BailmentNotice;
    [[nodiscard]] auto asConnection() && noexcept -> reply::Connection;
    [[nodiscard]] auto asFaucet() && noexcept -> reply::Faucet;
    [[nodiscard]] auto asOutbailment() && noexcept -> reply::Outbailment;
    [[nodiscard]] auto asStoreSecret() && noexcept -> reply::StoreSecret;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Reply&;
    [[nodiscard]] auto SetAlias(std::string_view alias) noexcept -> bool final;
    auto swap(Reply& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Reply(ReplyPrivate* imp) noexcept;
    Reply(allocator_type alloc = {}) noexcept;
    Reply(const Reply& rhs, allocator_type alloc = {}) noexcept;
    Reply(Reply&& rhs) noexcept;
    Reply(Reply&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Reply& rhs) noexcept -> Reply&;
    auto operator=(Reply&& rhs) noexcept -> Reply&;

    ~Reply() override;

protected:
    friend ReplyPrivate;

    ReplyPrivate* imp_;
};
}  // namespace opentxs::contract::peer
