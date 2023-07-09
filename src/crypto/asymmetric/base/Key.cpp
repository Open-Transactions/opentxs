// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/crypto/asymmetric/Key.hpp"  // IWYU pragma: associated

#include <cstring>
#include <string_view>
#include <utility>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric
{
auto operator==(const Key& lhs, const Key& rhs) noexcept -> bool
{
    return lhs.PublicKey() == rhs.PublicKey();
}

auto operator<=>(const Key& lKey, const Key& rKey) noexcept
    -> std::strong_ordering
{
    const auto lhs = lKey.PublicKey();
    const auto rhs = rKey.PublicKey();
    // TODO someday when fucking Android uses a less broken version of fucking
    // libc++ that actually fucking implements modern c++ standards use the
    // std::string_view overload of operator<=>

    if (auto l = lhs.size(), r = rhs.size(); l < r) {

        return std::strong_ordering::less;
    } else if (r < l) {

        return std::strong_ordering::greater;
    } else {
        if (0_uz == l) {

            return std::strong_ordering::equal;
        } else if (auto c = std::memcmp(lhs.data(), rhs.data(), lhs.size());
                   0 == c) {

            return std::strong_ordering::equal;
        } else if (0 < c) {

            return std::strong_ordering::greater;
        } else {

            return std::strong_ordering::less;
        }
    }
}

Key::Key(KeyPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Key::Key(allocator_type alloc) noexcept
    : Key(KeyPrivate::Blank(alloc))
{
}

Key::Key(const Key& rhs, allocator_type alloc) noexcept
    : Key(rhs.imp_->clone(alloc))
{
}

Key::Key(Key&& rhs) noexcept
    : Key(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Key::Key(Key&& rhs, allocator_type alloc) noexcept
    : Key(alloc)
{
    operator=(std::move(rhs));
}

auto Key::asEllipticCurve() const noexcept -> const key::EllipticCurve&
{
    return imp_->asEllipticCurvePublic();
}

auto Key::asEllipticCurve() noexcept -> key::EllipticCurve&
{
    return imp_->asEllipticCurvePublic();
}

auto Key::asPublic(alloc::Strategy alloc) const noexcept -> Key
{
    return imp_->asPublic(alloc);
}

auto Key::asRSA() const noexcept -> const key::RSA&
{
    return imp_->asRSAPublic();
}

auto Key::asRSA() noexcept -> key::RSA& { return imp_->asRSAPublic(); }

auto Key::Blank() noexcept -> Key&
{
    static auto blank = Key{allocator_type{alloc::Strategy().result_}};

    return blank;
}

auto Key::DefaultVersion() noexcept -> VersionNumber { return 2; }

auto Key::ErasePrivateData() noexcept -> bool
{
    return imp_->ErasePrivateData();
}

auto Key::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Key::HasCapability(identity::NymCapability capability) const noexcept
    -> bool
{
    return imp_->HasCapability(capability);
}

auto Key::HasPrivate() const noexcept -> bool { return imp_->HasPrivate(); }

auto Key::HasPublic() const noexcept -> bool { return imp_->HasPublic(); }

auto Key::Internal() const noexcept -> const internal::Key& { return *imp_; }

auto Key::Internal() noexcept -> internal::Key& { return *imp_; }

auto Key::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Key::MaxVersion() noexcept -> VersionNumber { return 2; }

auto Key::operator=(const Key& rhs) noexcept -> Key&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Key::operator=(Key&& rhs) noexcept -> Key&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Key::PreferredHash() const noexcept -> crypto::HashType
{
    return imp_->PreferredHash();
}

auto Key::PrivateKey(const PasswordPrompt& reason) const noexcept -> ReadView
{
    return imp_->PrivateKey(reason);
}

auto Key::PublicKey() const noexcept -> ReadView { return imp_->PublicKey(); }

auto Key::Role() const noexcept -> asymmetric::Role { return imp_->Role(); }

auto Key::Sign(
    ReadView preimage,
    Writer&& output,
    crypto::HashType hash,
    const PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->Sign(preimage, std::move(output), hash, reason);
}

auto Key::swap(Key& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Key::Type() const noexcept -> asymmetric::Algorithm
{
    return imp_->Type();
}

auto Key::Verify(ReadView plaintext, ReadView sig) const noexcept -> bool
{
    return imp_->Verify(plaintext, sig);
}

auto Key::Version() const noexcept -> VersionNumber { return imp_->Version(); }

Key::~Key() { pmr_delete(imp_); }
}  // namespace opentxs::crypto::asymmetric
