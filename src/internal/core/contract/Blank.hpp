// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs::contract::blank
{
template <typename IDType>
struct Signable2 : virtual public contract::Signable<IDType> {
    auto Alias() const noexcept -> UnallocatedCString final { return {}; }
    auto Alias(alloc::Strategy alloc) const noexcept -> CString final
    {
        return CString{alloc.result_};
    }
    auto ID() const noexcept -> const IDType& final
    {
        static const auto blank = IDType{};

        return blank;
    }
    auto Name() const noexcept -> std::string_view final { return {}; }
    auto Signer() const noexcept -> Nym_p final { return {}; }
    auto Terms() const noexcept -> std::string_view final { return {}; }
    auto Serialize(Writer&&) const noexcept -> bool final { return {}; }
    auto Validate() const noexcept -> bool final { return {}; }
    auto Version() const noexcept -> VersionNumber final { return {}; }

    auto SetAlias(std::string_view) noexcept -> bool final { return {}; }
};
}  // namespace opentxs::contract::blank
