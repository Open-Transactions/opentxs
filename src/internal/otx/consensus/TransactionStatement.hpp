// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/String.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::otx::context
{
class TransactionStatement
{
public:
    explicit operator OTString() const;

    auto Issued() const -> const UnallocatedSet<TransactionNumber>&;
    auto Notary() const -> const UnallocatedCString&;

    void Remove(const TransactionNumber& number);

    TransactionStatement(
        const UnallocatedCString& notary,
        const UnallocatedSet<TransactionNumber>& issued,
        const UnallocatedSet<TransactionNumber>& available);
    TransactionStatement(const String& serialized);
    TransactionStatement(TransactionStatement&& rhs) = default;
    TransactionStatement() = delete;
    TransactionStatement(const TransactionStatement& rhs) = delete;
    auto operator=(const TransactionStatement& rhs)
        -> TransactionStatement& = delete;
    auto operator=(TransactionStatement&& rhs)
        -> TransactionStatement& = delete;

    ~TransactionStatement() = default;

private:
    UnallocatedCString version_;
    UnallocatedCString nym_id_;
    UnallocatedCString notary_;
    UnallocatedSet<TransactionNumber> available_;
    UnallocatedSet<TransactionNumber> issued_;
};
}  // namespace opentxs::otx::context
