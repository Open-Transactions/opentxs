// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/common/User.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <cctype>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <utility>

#include "internal/util/Mutex.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

namespace ottest
{
User::User(
    std::string_view words,
    std::string_view name,
    std::string_view passphrase) noexcept
    : words_(words)
    , passphrase_(passphrase)
    , name_(name)
    , name_lower_([&] {
        auto out = std::stringstream{};

        for (const auto c : name_) {
            out << static_cast<char>(
                std::tolower(static_cast<unsigned char>(c)));
        }

        return out.str();
    }())
    , api_(nullptr)
    , init_(false)
    , seed_id_()
    , index_()
    , nym_()
    , nym_id_()
    , payment_code_()
    , lock_()
    , contacts_()
    , accounts_()
{
}

auto User::Account(std::string_view type) const noexcept
    -> const ot::identifier::Account&
{
    ot::Lock lock(lock_);

    if (auto i = accounts_.find(type); accounts_.end() != i) {

        return i->second;
    } else {
        abort();
    }
}

auto User::Contact(std::string_view contact) const noexcept
    -> const ot::identifier::Generic&
{
    static const auto blank = ot::identifier::Generic{};

    ot::Lock lock(lock_);

    if (auto i = contacts_.find(contact); contacts_.end() != i) {

        return i->second;
    } else {

        return blank;
    }
}

auto User::init_basic(
    const ot::api::session::Client& api,
    const ot::identity::Type type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> bool
{
    if (init_) { return false; }

    api_ = &api;
    seed_id_ = api.Crypto().Seed().ImportSeed(
        api.Factory().SecretFromText(words_),
        api.Factory().SecretFromText(passphrase_),
        seed,
        ot::crypto::Language::en,
        Reason());
    index_ = index;
    nym_ = api.Wallet().Nym(
        {api.Factory(), seed_id_, static_cast<int>(index_)},
        type,
        Reason(),
        name_);

    opentxs::assert_false(nullptr == nym_);

    nym_id_ = nym_->ID();
    payment_code_ =
        api.Factory()
            .PaymentCode(
                seed_id_, index_, ot::PaymentCode::DefaultVersion(), Reason())
            .asBase58();

    if (false == name_.empty()) {
        contacts_.emplace(name_, api.Contacts().NymToContact(nym_id_));
    }

    return true;
}

auto User::init(
    const ot::api::session::Client& api,
    const Server& server,
    const ot::identity::Type type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> bool
{
    if (init_basic(api, type, index, seed)) {
        set_introduction_server(api, server);
        init_ = true;

        return true;
    }

    return false;
}

auto User::init(
    const ot::api::session::Client& api,
    const ot::identity::Type type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> bool
{
    if (init_basic(api, type, index, seed)) {
        init_ = true;

        return true;
    }

    return false;
}

auto User::init_custom(
    const ot::api::session::Client& api,
    const Server& server,
    const std::function<void(User&)> custom,
    const ot::identity::Type type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> void
{
    if (init(api, server, type, index, seed) && custom) { custom(*this); }
}

auto User::init_custom(
    const ot::api::session::Client& api,
    const std::function<void(User&)> custom,
    const ot::identity::Type type,
    const std::uint32_t index,
    const ot::crypto::SeedStyle seed) noexcept -> void
{
    if (init_basic(api, type, index, seed)) {
        if (custom) { custom(*this); }

        init_ = true;
    }
}

auto User::PaymentCode() const -> ot::PaymentCode
{
    opentxs::assert_false(nullptr == api_);

    return api_->Factory().PaymentCodeFromBase58(payment_code_);
}

auto User::Reason() const noexcept -> ot::PasswordPrompt
{
    opentxs::assert_false(nullptr == api_);

    return api_->Factory().PasswordPrompt(__func__);
}

auto User::SetAccount(std::string_view type, std::string_view id) const noexcept
    -> bool
{
    opentxs::assert_false(nullptr == api_);

    return SetAccount(type, api_->Factory().AccountIDFromBase58(id));
}

auto User::SetAccount(std::string_view type, const ot::identifier::Account& id)
    const noexcept -> bool
{
    opentxs::assert_false(nullptr == api_);

    ot::Lock lock(lock_);
    const auto [it, added] = accounts_.emplace(type, id);

    return added;
}

auto User::SetContact(std::string_view contact, std::string_view id)
    const noexcept -> bool
{
    opentxs::assert_false(nullptr == api_);

    return SetContact(contact, api_->Factory().IdentifierFromBase58(id));
}

auto User::SetContact(
    std::string_view contact,
    const ot::identifier::Generic& id) const noexcept -> bool
{
    opentxs::assert_false(nullptr == api_);

    ot::Lock lock(lock_);
    const auto [it, added] = contacts_.emplace(contact, id);

    return added;
}
}  // namespace ottest
