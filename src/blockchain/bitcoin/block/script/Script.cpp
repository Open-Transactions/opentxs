// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::script::Pattern
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::script::Position

#include "opentxs/blockchain/bitcoin/block/Script.hpp"  // IWYU pragma: associated

#include <functional>
#include <utility>

#include "blockchain/bitcoin/block/script/ScriptPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block
{
Script::Script(ScriptPrivate* imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(nullptr != imp_);
}

Script::Script(allocator_type alloc) noexcept
    : Script(ScriptPrivate::Blank(alloc))
{
}

Script::Script(const Script& rhs, allocator_type alloc) noexcept
    : Script(rhs.imp_->clone(alloc))
{
}

Script::Script(Script&& rhs) noexcept
    : Script(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Script::Script(Script&& rhs, allocator_type alloc) noexcept
    : Script(alloc)
{
    operator=(std::move(rhs));
}

auto Script::Blank() noexcept -> Script&
{
    static auto blank = Script{};

    return blank;
}

auto Script::CalculateHash160(const api::Crypto& crypto, Writer&& output)
    const noexcept -> bool
{
    return imp_->CalculateHash160(crypto, std::move(output));
}

auto Script::CalculateSize() const noexcept -> std::size_t
{
    return imp_->CalculateSize();
}

auto Script::get() const noexcept -> std::span<const value_type>
{
    return imp_->get();
}

auto Script::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Script::Internal() const noexcept -> const internal::Script&
{
    return *imp_;
}

auto Script::Internal() noexcept -> internal::Script& { return *imp_; }

auto Script::IsNotification(
    const std::uint8_t version,
    const PaymentCode& recipient) const noexcept -> bool
{
    return imp_->IsNotification(version, recipient);
}

auto Script::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Script::M() const noexcept -> std::optional<std::uint8_t>
{
    return imp_->M();
}

auto Script::MultisigPubkey(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    return imp_->MultisigPubkey(position);
}

auto Script::N() const noexcept -> std::optional<std::uint8_t>
{
    return imp_->N();
}

auto Script::operator=(const Script& rhs) noexcept -> Script&
{
    if (imp_ != rhs.imp_) {
        if (nullptr == imp_) {
            // NOTE moved-from state
            imp_ = rhs.imp_->clone(rhs.imp_->get_allocator());
        } else {
            auto* old{imp_};
            imp_ = rhs.imp_->clone(get_allocator());
            // TODO switch to destroying delete after resolution of
            // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
            auto deleter = old->get_deleter();
            std::invoke(deleter);
        }
    }

    return *this;
}

auto Script::operator=(Script&& rhs) noexcept -> Script&
{
    if (nullptr == imp_) {
        // NOTE moved-from state
        swap(rhs);

        return *this;
    } else if (get_allocator() == rhs.get_allocator()) {
        swap(rhs);

        return *this;
    } else {

        return operator=(rhs);
    }
}

auto Script::Print() const noexcept -> UnallocatedCString
{
    return imp_->Print();
}

auto Script::Print(allocator_type alloc) const noexcept -> CString
{
    return imp_->Print(alloc);
}

auto Script::Pubkey() const noexcept -> std::optional<ReadView>
{
    return imp_->Pubkey();
}

auto Script::PubkeyHash() const noexcept -> std::optional<ReadView>
{
    return imp_->PubkeyHash();
}

auto Script::RedeemScript(alloc::Default alloc) const noexcept -> Script
{
    return imp_->RedeemScript(alloc);
}

auto Script::Role() const noexcept -> script::Position { return imp_->Role(); }

auto Script::ScriptHash() const noexcept -> std::optional<ReadView>
{
    return imp_->ScriptHash();
}

auto Script::Serialize(Writer&& destination) const noexcept -> bool
{
    return imp_->Serialize(std::move(destination));
}

auto Script::swap(Script& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Script::Type() const noexcept -> script::Pattern { return imp_->Type(); }

auto Script::Value(const std::size_t position) const noexcept
    -> std::optional<ReadView>
{
    return imp_->Value(position);
}

Script::~Script()
{
    if (nullptr != imp_) {
        // TODO switch to destroying delete after resolution of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
        auto deleter = imp_->get_deleter();
        std::invoke(deleter);
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::bitcoin::block
