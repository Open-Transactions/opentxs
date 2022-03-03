// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/p2p/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class OPENTXS_EXPORT PublishContractReply final : public Base
{
public:
    class Imp;

    auto ID() const noexcept -> const Identifier&;
    auto Success() const noexcept -> bool;

    OPENTXS_NO_EXPORT PublishContractReply(Imp* imp) noexcept;

    ~PublishContractReply() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop

    PublishContractReply(const PublishContractReply&) = delete;
    PublishContractReply(PublishContractReply&&) = delete;
    auto operator=(const PublishContractReply&)
        -> PublishContractReply& = delete;
    auto operator=(PublishContractReply&&) -> PublishContractReply& = delete;
};
}  // namespace opentxs::network::p2p
