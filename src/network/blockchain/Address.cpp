// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/blockchain/Address.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "network/blockchain/AddressPrivate.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::network::blockchain
{
auto operator==(const Address& lhs, const Address& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const Address& lhs, const Address& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}
}  // namespace opentxs::network::blockchain

namespace opentxs::network::blockchain
{
Address::Address(AddressPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Address::Address() noexcept
    : Address(std::make_unique<AddressPrivate>().release())
{
}

Address::Address(const Address& rhs) noexcept
    : Address(rhs.imp_->clone().release())
{
}

Address::Address(Address&& rhs) noexcept
    : Address(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

auto Address::Bytes() const noexcept -> ByteArray { return imp_->Bytes(); }

auto Address::Chain() const noexcept -> opentxs::blockchain::Type
{
    return imp_->Chain();
}

auto Address::Display() const noexcept -> UnallocatedCString
{
    return imp_->Display();
}

auto Address::ID() const noexcept -> const identifier::Generic&
{
    return imp_->ID();
}

auto Address::Internal() const noexcept -> const internal::Address&
{
    return *imp_;
}

auto Address::Internal() noexcept -> internal::Address& { return *imp_; }

auto Address::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Address::Key() const noexcept -> ReadView { return imp_->Key(); }

auto Address::LastConnected() const noexcept -> Time
{
    return imp_->LastConnected();
}

auto Address::operator=(const Address& rhs) noexcept -> Address&
{
    auto old = std::unique_ptr<AddressPrivate>(imp_);
    imp_ = rhs.imp_->clone().release();

    OT_ASSERT(nullptr != imp_);

    return *this;
}

auto Address::operator=(Address&& rhs) noexcept -> Address&
{
    using std::swap;
    swap(imp_, rhs.imp_);

    return *this;
}

auto Address::Port() const noexcept -> std::uint16_t { return imp_->Port(); }

auto Address::Services() const noexcept -> Set<bitcoin::Service>
{
    return imp_->Services();
}

auto Address::Subtype() const noexcept -> Transport { return imp_->Subtype(); }

auto Address::Style() const noexcept -> Protocol { return imp_->Style(); }

auto Address::Type() const noexcept -> Transport { return imp_->Type(); }

Address::~Address()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::blockchain