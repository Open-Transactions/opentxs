// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <set>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace otx
{
namespace context
{
class TransactionStatement
{
private:
    std::string version_;
    std::string nym_id_;
    std::string notary_;
    std::pmr::set<TransactionNumber> available_;
    std::pmr::set<TransactionNumber> issued_;

    TransactionStatement() = delete;
    TransactionStatement(const TransactionStatement& rhs) = delete;
    auto operator=(const TransactionStatement& rhs)
        -> TransactionStatement& = delete;
    auto operator=(TransactionStatement&& rhs)
        -> TransactionStatement& = delete;

public:
    TransactionStatement(
        const std::string& notary,
        const std::pmr::set<TransactionNumber>& issued,
        const std::pmr::set<TransactionNumber>& available);
    TransactionStatement(const String& serialized);
    TransactionStatement(TransactionStatement&& rhs) = default;

    explicit operator OTString() const;

    auto Issued() const -> const std::pmr::set<TransactionNumber>&;
    auto Notary() const -> const std::string&;

    void Remove(const TransactionNumber& number);

    ~TransactionStatement() = default;
};
}  // namespace context
}  // namespace otx
}  // namespace opentxs
