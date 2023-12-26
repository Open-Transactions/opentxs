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
#include "opentxs/Time.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace internal
{
class Request;
}  // namespace internal

namespace request
{
class Bailment;
class BailmentNotice;
class Connection;
class Faucet;
class Outbailment;
class StoreSecret;
class Verification;
}  // namespace request

class Request;
class RequestPrivate;
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
struct OPENTXS_EXPORT hash<opentxs::contract::peer::Request> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::contract::peer::Request&) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::contract::peer::Request> {
    auto operator()(
        const opentxs::contract::peer::Request& lhs,
        const opentxs::contract::peer::Request& rhs) const noexcept -> bool;
};
}  // namespace std

namespace opentxs::contract::peer
{
OPENTXS_EXPORT auto operator==(const Request& lhs, const Request& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Request& lhs, const Request& rhs) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Request& lhs, Request& rhs) noexcept -> void;
}  // namespace opentxs::contract::peer

namespace opentxs::contract::peer
{
class OPENTXS_EXPORT Request : virtual public Signable<identifier::Generic>,
                               virtual public Allocated
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Request&;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    [[nodiscard]] auto Alias() const noexcept -> UnallocatedCString final;
    [[nodiscard]] auto Alias(alloc::Strategy alloc) const noexcept
        -> CString final;
    [[nodiscard]] auto asBailment() const& noexcept -> const request::Bailment&;
    [[nodiscard]] auto asBailmentNotice() const& noexcept
        -> const request::BailmentNotice&;
    [[nodiscard]] auto asConnection() const& noexcept
        -> const request::Connection&;
    [[nodiscard]] auto asFaucet() const& noexcept -> const request::Faucet&;
    [[nodiscard]] auto asOutbailment() const& noexcept
        -> const request::Outbailment&;
    [[nodiscard]] auto asStoreSecret() const& noexcept
        -> const request::StoreSecret&;
    [[nodiscard]] auto asVerification() const& noexcept
        -> const request::Verification&;
    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type final;
    [[nodiscard]] auto ID() const noexcept -> const identifier_type& final;
    [[nodiscard]] auto Initiator() const noexcept -> const identifier::Nym&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Request&;
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

    [[nodiscard]] auto asBailment() && noexcept -> request::Bailment;
    [[nodiscard]] auto asBailmentNotice() && noexcept
        -> request::BailmentNotice;
    [[nodiscard]] auto asConnection() && noexcept -> request::Connection;
    [[nodiscard]] auto asFaucet() && noexcept -> request::Faucet;
    [[nodiscard]] auto asOutbailment() && noexcept -> request::Outbailment;
    [[nodiscard]] auto asStoreSecret() && noexcept -> request::StoreSecret;
    [[nodiscard]] auto asVerification() && noexcept -> request::Verification;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Request&;
    [[nodiscard]] auto SetAlias(std::string_view alias) noexcept -> bool final;
    auto swap(Request& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Request(RequestPrivate* imp) noexcept;
    Request(allocator_type alloc = {}) noexcept;
    Request(const Request& rhs, allocator_type alloc = {}) noexcept;
    Request(Request&& rhs) noexcept;
    Request(Request&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Request& rhs) noexcept -> Request&;
    auto operator=(Request&& rhs) noexcept -> Request&;

    ~Request() override;

protected:
    friend RequestPrivate;

    RequestPrivate* imp_;
};
}  // namespace opentxs::contract::peer
