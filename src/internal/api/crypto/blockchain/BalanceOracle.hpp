// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <string_view>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::blockchain
{
class BalanceOracle
{
public:
    auto Start() noexcept -> void;

    BalanceOracle(
        std::shared_ptr<const api::internal::Session> api,
        std::string_view endpoint) noexcept;
    BalanceOracle() = delete;
    BalanceOracle(const BalanceOracle&) = delete;
    BalanceOracle(BalanceOracle&&) = delete;

    ~BalanceOracle();

private:
    class Imp;

    std::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::api::crypto::blockchain
