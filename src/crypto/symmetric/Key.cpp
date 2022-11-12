// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::symmetric::Algorithm

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "opentxs/crypto/symmetric/Key.hpp"  // IWYU pragma: associated

#include <utility>

#include "crypto/symmetric/KeyPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::symmetric
{
Key::Key(KeyPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Key::Key(allocator_type alloc) noexcept
    : Key([&] {
        auto pmr = alloc::PMR<KeyPrivate>{alloc};
        auto* out = pmr.allocate(1_uz);

        OT_ASSERT(nullptr != out);

        pmr.construct(out);

        return out;
    }())
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

auto Key::ChangePassword(
    const Secret& newPassword,
    const PasswordPrompt& reason) noexcept -> bool
{
    return imp_->ChangePassword(newPassword, reason);
}

auto Key::Decrypt(
    ReadView ciphertext,
    Writer&& plaintext,
    const PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->Decrypt(ciphertext, std::move(plaintext), reason);
}

auto Key::Encrypt(
    ReadView plaintext,
    Writer&& ciphertext,
    Algorithm mode,
    const PasswordPrompt& reason,
    bool attachKey,
    ReadView iv) const noexcept -> bool
{
    return imp_->Encrypt(
        plaintext, std::move(ciphertext), mode, attachKey, iv, reason);
}

auto Key::Encrypt(
    ReadView plaintext,
    Writer&& ciphertext,
    const PasswordPrompt& reason,
    bool attachKey,
    ReadView iv) const noexcept -> bool
{
    return imp_->Encrypt(
        plaintext, std::move(ciphertext), attachKey, iv, reason);
}

auto Key::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Key::ID(const PasswordPrompt& reason) const noexcept
    -> const identifier::Generic&
{
    return imp_->ID(reason);
}

auto Key::Internal() const noexcept -> const internal::Key& { return *imp_; }

auto Key::Internal() noexcept -> internal::Key& { return *imp_; }

auto Key::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Key::operator=(const Key& rhs) noexcept -> Key&
{
    if (imp_ != rhs.imp_) {
        auto* old{imp_};
        imp_ = rhs.imp_->clone(get_allocator());
        delete old;
    }

    return *this;
}

auto Key::operator=(Key&& rhs) noexcept -> Key&
{
    swap(rhs);

    return *this;
}

auto Key::swap(Key& rhs) noexcept -> void
{
    OT_ASSERT(get_allocator() == rhs.get_allocator());

    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Key::Unlock(const PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->Unlock(reason);
}

Key::~Key()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::crypto::symmetric
