// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class Contact;
class ContactSection;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT Contact : virtual public List
{
public:
    virtual auto ContactID() const noexcept -> UnallocatedCString = 0;
    virtual auto DisplayName() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactSection> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactSection> = 0;
    virtual auto PaymentCode() const noexcept -> UnallocatedCString = 0;

    ~Contact() override = default;

protected:
    Contact() noexcept = default;

private:
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;
};
}  // namespace opentxs::ui