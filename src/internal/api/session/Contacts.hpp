// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/util/Editor.hpp"
#include "opentxs/api/session/Contacts.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api

class Contact;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Contacts : virtual public api::session::Contacts
{
public:
    auto Internal() const noexcept -> const internal::Contacts& final
    {
        return *this;
    }
    virtual auto mutable_Contact(const identifier::Generic& id) const
        -> std::unique_ptr<Editor<opentxs::Contact>> = 0;

    auto Internal() noexcept -> internal::Contacts& final { return *this; }
    virtual auto init(
        const std::shared_ptr<const crypto::Blockchain>& blockchain)
        -> void = 0;
    virtual auto prepare_shutdown() -> void = 0;
    virtual auto start() -> void = 0;

    ~Contacts() override = default;
};
}  // namespace opentxs::api::session::internal
