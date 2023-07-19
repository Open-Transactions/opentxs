// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/crypto/Hasher.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "crypto/hasher/HasherPrivate.hpp"
#include "internal/util/LogMacros.hpp"

namespace opentxs::crypto
{
Hasher::Hasher(HasherPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Hasher::Hasher() noexcept
    : Hasher(std::make_unique<HasherPrivate>().release())
{
}

Hasher::Hasher(Hasher&& rhs) noexcept
    : Hasher(std::exchange(rhs.imp_, nullptr))
{
}

auto Hasher::operator=(Hasher&& rhs) noexcept -> Hasher&
{
    using std::swap;
    swap(imp_, rhs.imp_);

    return *this;
}

auto Hasher::operator()(ReadView data) noexcept -> bool
{
    return imp_->operator()(data);
}

auto Hasher::operator()(Writer&& destination) noexcept -> bool
{
    return imp_->operator()(std::move(destination));
}

Hasher::~Hasher()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::crypto
