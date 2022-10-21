// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "opentxs/crypto/asymmetric/Key.hpp"  // IWYU pragma: associated

#include <functional>
#include <utility>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric
{
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
    : imp_(nullptr)
{
    if (alloc == rhs.get_allocator()) {
        swap(rhs);
    } else {
        operator=(rhs);
    }
}

auto Key::asEllipticCurve() const noexcept -> const key::EllipticCurve&
{
    return imp_->asEllipticCurvePublic();
}

auto Key::asEllipticCurve() noexcept -> key::EllipticCurve&
{
    return imp_->asEllipticCurvePublic();
}

auto Key::asPublic(allocator_type alloc) const noexcept -> Key
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
    static auto blank = Key{allocator_type{alloc::Default()}};

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
    if (imp_ != rhs.imp_) {
        auto* old{imp_};
        imp_ = rhs.imp_->clone(get_allocator());
        // TODO switch to destroying delete after resolution of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
        auto deleter = old->get_deleter();
        std::invoke(deleter, old);
    }

    return *this;
}

auto Key::operator=(Key&& rhs) noexcept -> Key&
{
    swap(rhs);

    return *this;
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
    OT_ASSERT(get_allocator() == rhs.get_allocator());

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

Key::~Key()
{
    if (nullptr != imp_) {
        // TODO switch to destroying delete after resolution of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
        auto deleter = imp_->get_deleter();
        std::invoke(deleter, imp_);
        imp_ = nullptr;
    }
}
}  // namespace opentxs::crypto::asymmetric
