// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contract/Types.hpp"
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
class OPENTXS_EXPORT QueryContractReply final : public Base
{
public:
    class Imp;

    auto ContractType() const noexcept -> contract::Type;
    auto ID() const noexcept -> const identifier::Generic&;
    auto Payload() const noexcept -> ReadView;

    OPENTXS_NO_EXPORT QueryContractReply(Imp* imp) noexcept;
    QueryContractReply(const QueryContractReply&) = delete;
    QueryContractReply(QueryContractReply&&) = delete;
    auto operator=(const QueryContractReply&) -> QueryContractReply& = delete;
    auto operator=(QueryContractReply&&) -> QueryContractReply& = delete;

    ~QueryContractReply() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop
};
}  // namespace opentxs::network::otdht
