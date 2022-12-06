// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::Language
// IWYU pragma: no_forward_declare opentxs::crypto::SeedStyle

#include "opentxs/interface/qt/SeedValidator.hpp"  // IWYU pragma: associated

#include <QChar>

#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui
{
struct SeedValidator::Imp {
    using State = QValidator::State;

    auto checkPhrase(const Phrase& phrase) const -> bool
    {
        if (0 == phrase.size()) { return false; }

        for (const auto& word : phrase) {
            if (0 == word.size()) { return false; }

            const auto in = word.toStdString();
            const auto matches =
                api_.Crypto().Seed().ValidateWord(type_, lang_, in);

            if (1 != matches.size()) { return false; }
        }

        // TODO validate type-specific checksum

        return true;
    }
    auto fixup(QString& input) const -> void
    {
        auto clean = QString{};

        for (const auto& c : input) {
            if (c.isLetter()) { clean += c; }
        }

        auto notUsed{0};
        validate(clean, notUsed);
        input = clean;
    }
    auto validate(QString& input, int& pos) const -> State
    {
        if (0 == input.size()) { return State::Intermediate; }

        const auto string = input.toStdString();
        const auto matches =
            api_.Crypto().Seed().ValidateWord(type_, lang_, string);

        if (0 == matches.size()) {

            return State::Invalid;
        } else if (1 == matches.size()) {
            input = matches.begin()->c_str();
            pos = static_cast<int>(input.size());

            return State::Acceptable;
        } else {

            return State::Intermediate;
        }
    }

    Imp(const api::session::Client& api,
        const crypto::SeedStyle type,
        const crypto::Language lang) noexcept
        : api_(api)
        , type_(type)
        , lang_(lang)
    {
    }

private:
    const api::session::Client& api_;
    const crypto::SeedStyle type_;
    const crypto::Language lang_;
};

SeedValidator::SeedValidator(
    const api::session::Client& api,
    std::uint8_t type,
    std::uint8_t lang) noexcept
    : QValidator()
    , imp_(std::make_unique<Imp>(
          api,
          static_cast<crypto::SeedStyle>(type),
          static_cast<crypto::Language>(lang)))
{
}

auto SeedValidator::checkPhrase(const Phrase& phrase) const -> bool
{
    return imp_->checkPhrase(phrase);
}

auto SeedValidator::fixup(QString& input) const -> void
{
    return imp_->fixup(input);
}

auto SeedValidator::validate(QString& input, int& pos) const -> State
{
    return imp_->validate(input, pos);
}

SeedValidator::~SeedValidator() = default;
}  // namespace opentxs::ui
