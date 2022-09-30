// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/network/otdht/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT PublishContractReply final : public Base
{
public:
    class Imp;

    auto ID() const noexcept -> const identifier::Generic&;
    auto Success() const noexcept -> bool;

    OPENTXS_NO_EXPORT PublishContractReply(Imp* imp) noexcept;
    PublishContractReply(const PublishContractReply&) = delete;
    PublishContractReply(PublishContractReply&&) = delete;
    auto operator=(const PublishContractReply&)
        -> PublishContractReply& = delete;
    auto operator=(PublishContractReply&&) -> PublishContractReply& = delete;

    ~PublishContractReply() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
