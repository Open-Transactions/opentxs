// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::otx::blind::CashType
// IWYU pragma: no_forward_declare opentxs::otx::blind::PurseType

#include "otx/blind/purse/Purse.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::otx::blind
{
auto Purse::Imp::at(const std::size_t) const -> const Token&
{
    static const auto blank = Token{};

    return blank;
}

auto Purse::Imp::at(const std::size_t) -> Token&
{
    static auto blank = Token{};

    return blank;
}

auto Purse::Imp::Notary() const -> const identifier::Notary&
{
    static const auto id = identifier::Notary{};

    return id;
}

auto Purse::Imp::PrimaryKey(PasswordPrompt&) -> crypto::symmetric::Key&
{
    static auto blank = crypto::symmetric::Key{};

    return blank;
}

auto Purse::Imp::SecondaryKey(const identity::Nym&, PasswordPrompt&)
    -> const crypto::symmetric::Key&
{
    static auto blank = crypto::symmetric::Key{};

    return blank;
}

auto Purse::Imp::Unit() const -> const identifier::UnitDefinition&
{
    static const auto id = identifier::UnitDefinition{};

    return id;
}

auto Purse::Imp::Value() const -> const Amount&
{
    static auto blank = Amount{};

    return blank;
}

Purse::Imp::~Imp() = default;
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
Purse::Purse(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
}

Purse::Purse() noexcept
    : Purse(std::make_unique<Imp>().release())
{
    OT_ASSERT(this == imp_->parent_);
}

Purse::Purse(const Purse& rhs) noexcept
    : Purse(rhs.imp_->clone())
{
    OT_ASSERT(this == imp_->parent_);
}

Purse::Purse(Purse&& rhs) noexcept
    : Purse()
{
    swap(rhs);

    OT_ASSERT(this == imp_->parent_);
    OT_ASSERT(nullptr != rhs.imp_->parent_);
}

Purse::operator bool() const noexcept { return imp_->IsValid(); }

auto Purse::AddNym(const identity::Nym& nym, const PasswordPrompt& reason)
    -> bool
{
    return imp_->AddNym(nym, reason);
}

auto Purse::at(const std::size_t position) -> Token&
{
    return imp_->at(position);
}

auto Purse::at(const std::size_t position) const -> const Token&
{
    return imp_->at(position);
}

auto Purse::begin() const noexcept -> const_iterator { return imp_->cbegin(); }

auto Purse::begin() noexcept -> iterator { return imp_->begin(); }

auto Purse::cbegin() const noexcept -> const_iterator { return imp_->cbegin(); }

auto Purse::cend() const noexcept -> const_iterator { return imp_->cend(); }

auto Purse::EarliestValidTo() const -> Time { return imp_->EarliestValidTo(); }

auto Purse::end() const noexcept -> const_iterator { return imp_->cend(); }

auto Purse::end() noexcept -> iterator { return imp_->end(); }

auto Purse::Internal() const noexcept -> const internal::Purse&
{
    return *imp_;
}

auto Purse::Internal() noexcept -> internal::Purse& { return *imp_; }

auto Purse::IsUnlocked() const -> bool { return imp_->IsUnlocked(); }

auto Purse::LatestValidFrom() const -> Time { return imp_->LatestValidFrom(); }

auto Purse::Notary() const -> const identifier::Notary&
{
    return imp_->Notary();
}

auto Purse::operator=(const Purse& rhs) noexcept -> Purse&
{
    auto old = std::unique_ptr<Imp>(imp_);
    imp_ = rhs.imp_->clone();
    imp_->parent_ = this;

    return *this;
}

auto Purse::operator=(Purse&& rhs) noexcept -> Purse&
{
    swap(rhs);

    return *this;
}

auto Purse::Pop() -> Token { return imp_->Pop(); }

auto Purse::Push(Token&& token, const PasswordPrompt& reason) -> bool
{
    return imp_->Push(std::move(token), reason);
}

auto Purse::Serialize(Writer&& destination) const noexcept -> bool
{
    return imp_->Serialize(std::move(destination));
}

auto Purse::size() const noexcept -> std::size_t { return imp_->size(); }

auto Purse::State() const -> blind::PurseType { return imp_->State(); }

auto Purse::swap(Purse& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
    std::swap(imp_->parent_, rhs.imp_->parent_);

    OT_ASSERT(this == imp_->parent_);
    OT_ASSERT(nullptr != rhs.imp_->parent_);
}

auto Purse::Type() const -> blind::CashType { return imp_->Type(); }

auto Purse::Unit() const -> const identifier::UnitDefinition&
{
    return imp_->Unit();
}

auto Purse::Unlock(const identity::Nym& nym, const PasswordPrompt& reason) const
    -> bool
{
    return imp_->Unlock(nym, reason);
}

auto Purse::Value() const -> const Amount& { return imp_->Value(); }

auto Purse::Verify(const api::session::Notary& server) const -> bool
{
    return imp_->Verify(server);
}

Purse::~Purse()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::otx::blind
