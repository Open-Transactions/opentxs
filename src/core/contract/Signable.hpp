// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <functional>
#include <shared_mutex>
#include <span>
#include <string_view>

#include "internal/core/contract/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Signature;
}  // namespace proto

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::implementation
{
template <typename IDType>
class Signable : virtual public opentxs::contract::Signable<IDType>
{
public:
    auto Alias() const noexcept -> UnallocatedCString override;
    auto Alias(alloc::Strategy alloc) const noexcept -> CString override;
    auto ID() const noexcept -> const IDType& override;
    auto Name() const noexcept -> std::string_view override;
    auto Signer() const noexcept -> Nym_p override;
    auto Terms() const noexcept -> std::string_view override;
    auto Validate() const noexcept -> bool override;
    auto Version() const noexcept -> VersionNumber override;

    auto SetAlias(std::string_view alias) noexcept -> bool override;

    Signable(Signable&&) = delete;
    auto operator=(const Signable&) -> Signable& = delete;
    auto operator=(Signable&&) -> Signable& = delete;

    ~Signable() override = default;

protected:
    using Signatures = Vector<Signature>;
    using GuardedAlias = libguarded::shared_guarded<CString, std::shared_mutex>;

    struct Calculated {
        IDType id_{};
        CString name_{};
    };

    struct SetNameFromID_t {
    };

    static constexpr auto set_name_from_id_ = SetNameFromID_t{};

    const api::Session& api_;

    virtual auto calculate_id() const -> IDType = 0;
    auto check_id() const noexcept -> bool;
    virtual auto signatures() const noexcept -> std::span<const Signature>;
    auto serialize(const ProtobufType& in, Writer&& out) const noexcept -> bool;
    virtual auto validate() const -> bool = 0;
    virtual auto verify_signature(const proto::Signature& signature) const
        -> bool;

    auto add_signatures(Signatures signatures) noexcept -> void;
    auto first_time_init() noexcept(false) -> void;
    auto first_time_init(SetNameFromID_t) noexcept(false) -> void;
    virtual auto init_serialized() noexcept(false) -> void;
    virtual auto update_signature(const PasswordPrompt& reason) -> bool;

    Signable(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        std::string_view conditions,
        std::string_view alias) noexcept;
    Signable(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        std::string_view conditions,
        std::string_view alias,
        std::string_view name,
        IDType id,
        Signatures&& signatures) noexcept;
    Signable(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        std::string_view conditions,
        std::string_view alias,
        Signatures&& signatures) noexcept;
    Signable(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        std::string_view conditions,
        std::string_view alias,
        IDType id,
        Signatures&& signatures) noexcept;
    Signable(const Signable&) noexcept;

private:
    using GetName = std::function<CString(const IDType&)>;

    const Nym_p signer_;
    const VersionNumber version_;
    const CString conditions_;
    GuardedAlias alias_;
    DeferredConstruction<Signatures> signatures_;
    DeferredConstruction<Calculated> set_once_;

    auto first_time_init(GetName name) noexcept(false) -> void;
};
}  // namespace opentxs::contract::implementation
