// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace verification
{
namespace internal
{
struct Nym;
}  // namespace internal
}  // namespace verification
}  // namespace wot

class Nym;
}  // namespace identity

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification
{
class OPENTXS_EXPORT Nym
{
public:
    using value_type = Item;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Nym, const value_type>;

    static const VersionNumber DefaultVersion;

    /// Throws std::out_of_range for invalid position
    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto ID() const noexcept -> const identifier::Nym& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Nym& = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto AddItem(
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const verification::Type value,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = Item::DefaultVersion) noexcept
        -> bool = 0;
    virtual auto DeleteItem(const identifier::Generic& item) noexcept
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Nym& = 0;

    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;

    virtual ~Nym() = default;

protected:
    Nym() = default;
};
}  // namespace opentxs::identity::wot::verification
