// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/session/FactoryPrivate.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Notary;
}  // namespace internal
}  // namespace session

class Factory;
}  // namespace api

class OTCron;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::notary
{
class FactoryPrivate final : public session::FactoryPrivate
{
public:
    auto Cron() const -> std::unique_ptr<OTCron> final;

    FactoryPrivate(const internal::Notary& api, const api::Factory& parent);
    FactoryPrivate() = delete;
    FactoryPrivate(const FactoryPrivate&) = delete;
    FactoryPrivate(FactoryPrivate&&) = delete;
    auto operator=(const FactoryPrivate&) -> FactoryPrivate& = delete;
    auto operator=(FactoryPrivate&&) -> FactoryPrivate& = delete;

    ~FactoryPrivate() final = default;

private:
    const internal::Notary& server_;
};
}  // namespace opentxs::api::session::notary
