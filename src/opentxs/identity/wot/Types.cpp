// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/Types.hpp"  // IWYU pragma: associated

#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"

namespace opentxs::identity::wot
{
auto copy(const Claimant& in, alloc::Strategy alloc) noexcept -> Claimant
{
    struct Visitor {
        alloc::Default alloc_{};

        auto operator()(const identifier::Nym& val) const noexcept -> Claimant
        {
            return identifier::Nym{val, alloc_};
        }
        auto operator()(const ContactID& val) const noexcept -> Claimant
        {
            return ContactID{val, alloc_};
        }
    };

    return std::visit(Visitor{alloc.result_}, in);
}
auto get_identifier(const Claimant& in) noexcept -> const identifier::Generic&
{
    struct Visitor {
        auto operator()(const identifier::Nym& val) const noexcept
            -> const identifier::Generic&
        {
            return val;
        }
        auto operator()(const ContactID& val) const noexcept
            -> const identifier::Generic&
        {
            return val;
        }
    };

    return std::visit(Visitor{}, in);
}
}  // namespace opentxs::identity::wot
