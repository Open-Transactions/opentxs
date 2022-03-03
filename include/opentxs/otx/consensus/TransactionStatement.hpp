// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs::otx::context
{
class TransactionStatement
{
private:
    UnallocatedCString version_;
    UnallocatedCString nym_id_;
    UnallocatedCString notary_;
    UnallocatedSet<TransactionNumber> available_;
    UnallocatedSet<TransactionNumber> issued_;

    TransactionStatement() = delete;
    TransactionStatement(const TransactionStatement& rhs) = delete;
    auto operator=(const TransactionStatement& rhs)
        -> TransactionStatement& = delete;
    auto operator=(TransactionStatement&& rhs)
        -> TransactionStatement& = delete;

public:
    TransactionStatement(
        const UnallocatedCString& notary,
        const UnallocatedSet<TransactionNumber>& issued,
        const UnallocatedSet<TransactionNumber>& available);
    TransactionStatement(const String& serialized);
    TransactionStatement(TransactionStatement&& rhs) = default;

    explicit operator OTString() const;

    auto Issued() const -> const UnallocatedSet<TransactionNumber>&;
    auto Notary() const -> const UnallocatedCString&;

    void Remove(const TransactionNumber& number);

    ~TransactionStatement() = default;
};
}  // namespace opentxs::otx::context
