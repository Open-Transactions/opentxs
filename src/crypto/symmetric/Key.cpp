// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/crypto/symmetric/Key.hpp"  // IWYU pragma: associated

#include <utility>

#include "crypto/symmetric/KeyPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::symmetric
{
Key::Key(KeyPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Key::Key(allocator_type alloc) noexcept
    : Key(pmr::default_construct<KeyPrivate>(alloc))
{
}

Key::Key(const Key& rhs, allocator_type alloc) noexcept
    : Key(rhs.imp_->clone(alloc))
{
}

Key::Key(Key&& rhs) noexcept
    : Key(std::exchange(rhs.imp_, nullptr))
{
}

Key::Key(Key&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
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

auto Key::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
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
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Key::operator=(Key&& rhs) noexcept -> Key&
{
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Key::swap(Key& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

auto Key::Unlock(const PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->Unlock(reason);
}

Key::~Key() { pmr::destroy(imp_); }
}  // namespace opentxs::crypto::symmetric
