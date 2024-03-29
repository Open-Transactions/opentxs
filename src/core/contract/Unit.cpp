// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/Unit.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/CurrencyParams.pb.h>
#include <opentxs/protobuf/DisplayScale.pb.h>
#include <opentxs/protobuf/Nym.pb.h>
#include <opentxs/protobuf/ScaleRatio.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <opentxs/protobuf/UnitDefinition.pb.h>
#include <cmath>  // IWYU pragma: keep
#include <cstddef>
#include <memory>
#include <span>
#include <sstream>  // IWYU pragma: keep
#include <string_view>
#include <utility>

#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/Contract.hpp"
#include "internal/core/contract/CurrencyContract.hpp"
#include "internal/core/contract/SecurityContract.hpp"
#include "internal/core/display/Factory.hpp"
#include "internal/core/display/Types.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/AccountVisitor.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/contract/Types.hpp"
#include "opentxs/contract/UnitDefinitionType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/display/Scale.hpp"
#include "opentxs/display/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/internal.factory.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/protobuf/syntax/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/common/OTStorage.hpp"

namespace opentxs
{
auto Factory::UnitDefinition(const api::Session& api) noexcept
    -> std::shared_ptr<contract::Unit>
{
    return std::make_shared<contract::blank::Unit>(api);
}

auto Factory::UnitDefinition(
    const api::Session& api,
    const Nym_p& nym,
    const protobuf::UnitDefinition serialized) noexcept
    -> std::shared_ptr<contract::Unit>
{
    switch (translate(serialized.type())) {
        case contract::UnitDefinitionType::Currency: {

            return CurrencyContract(api, nym, serialized);
        }
        case contract::UnitDefinitionType::Security: {

            return SecurityContract(api, nym, serialized);
        }
        case contract::UnitDefinitionType::Basket: {

            return BasketContract(api, nym, serialized);
        }
        case contract::UnitDefinitionType::Error:
        default: {
            return {};
        }
    }
}
}  // namespace opentxs

namespace opentxs::contract
{
const VersionNumber Unit::DefaultVersion{2};
const VersionNumber Unit::MaxVersion{2};
}  // namespace opentxs::contract

namespace opentxs::contract::implementation
{
using namespace std::literals;

const UnallocatedMap<VersionNumber, VersionNumber>
    Unit::unit_of_account_version_map_{{2, 6}};
const Unit::Locale Unit::locale_{};

Unit::Unit(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const opentxs::UnitType unitOfAccount,
    const VersionNumber version,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement)
    : Signable(api, nym, version, terms, shortname, Signatures{})
    , unit_of_account_(unitOfAccount)
    , display_definition_(displayDefinition)
    , redemption_increment_(redemptionIncrement)
    , short_name_(shortname)
{
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
Unit::Unit(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType serialized)
    : Signable(
          api,
          nym,
          serialized.version(),
          serialized.terms(),
          serialized.name(),
          api.Factory()
              .Internal()
              .UnitID(serialized.id())
              .asBase58(api.Crypto()),
          api.Factory().Internal().UnitID(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<protobuf::Signature>(
                    serialized.signature())}
              : Signatures{})
    , unit_of_account_(get_unitofaccount(serialized))
    , display_definition_(get_displayscales(serialized))
    , redemption_increment_(factory::Amount(serialized.redemption_increment()))
    , short_name_(serialized.name())
{
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

Unit::Unit(const Unit& rhs)
    : Signable(rhs)
    , unit_of_account_(rhs.unit_of_account_)
    , display_definition_(rhs.display_definition_)
    , redemption_increment_(rhs.redemption_increment_)
    , short_name_(rhs.short_name_)
{
}

auto Unit::AddAccountRecord(
    const UnallocatedCString& dataFolder,
    const Account& theAccount) const -> bool
{
    if (theAccount.GetInstrumentDefinitionID() != ID()) {
        LogError()()("Error: theAccount doesn't have the same "
                     "asset type ID as *this does.")
            .Flush();
        return false;
    }

    const auto theAcctID = api_.Factory().Internal().Identifier(theAccount);
    const auto strAcctID = String::Factory(theAcctID, api_.Crypto());

    const auto strInstrumentDefinitionID = ID().asBase58(api_.Crypto());
    auto record_file =
        api::internal::Paths::GetFilenameA(strInstrumentDefinitionID.c_str());

    OTDB::Storable* pStorable = nullptr;
    std::unique_ptr<OTDB::Storable> theAngel;
    OTDB::StringMap* pMap = nullptr;

    if (OTDB::Exists(
            api_,
            dataFolder,
            api_.Internal().Paths().Contract(),
            record_file,
            "",
            "")) {  // the file already exists; let's
                    // try to load it up.
        pStorable = OTDB::QueryObject(
            api_,
            OTDB::STORED_OBJ_STRING_MAP,
            dataFolder,
            api_.Internal().Paths().Contract(),
            record_file,
            "",
            "");
    } else {  // the account records file (for this instrument definition)
              // doesn't
              // exist.
        pStorable = OTDB::CreateObject(
            OTDB::STORED_OBJ_STRING_MAP);  // this asserts already, on failure.
    }

    theAngel.reset(pStorable);
    pMap = (nullptr == pStorable) ? nullptr
                                  : dynamic_cast<OTDB::StringMap*>(pStorable);

    // It exists.
    //
    if (nullptr == pMap) {
        LogError()()(
            "Error: Failed trying to load or create the account records "
            "file for instrument definition: ")(strInstrumentDefinitionID)(".")
            .Flush();
        return false;
    }

    auto& theMap = pMap->the_map_;
    auto map_it = theMap.find(strAcctID->Get());

    if (theMap.end() != map_it)  // we found it.
    {                            // We were ADDING IT, but it was ALREADY THERE.
        // (Thus, we're ALREADY DONE.)
        // Let's just make sure the right instrument definition ID is associated
        // with this
        // account
        // (it better be, since we loaded the account records file based on the
        // instrument definition ID as its filename...)
        //
        const auto& str2 = map_it->second;  // Containing the instrument
                                            // definition ID. (Just in
                                            // case
        // someone copied the wrong file here,
        // --------------------------------          // every account should map
        // to the SAME instrument definition id.)

        if (strInstrumentDefinitionID != str2) {
            LogError()()(
                "Error: wrong instrument definition found in account records "
                "file. For instrument definition: ")(
                strInstrumentDefinitionID)(". For account: ")(strAcctID.get())(
                ". Found wrong instrument definition: ")(str2)(".")
                .Flush();
            return false;
        }

        return true;  // already there (no need to add.) + the instrument
                      // definition ID
                      // matches.
    }

    // it wasn't already on the list...

    // ...so add it.
    //
    theMap[strAcctID->Get()] = strInstrumentDefinitionID;

    // Then save it back to local storage:
    //
    if (!OTDB::StoreObject(
            api_,
            *pMap,
            dataFolder,
            api_.Internal().Paths().Contract(),
            record_file,
            "",
            "")) {
        LogError()()(
            "Failed trying to StoreObject, while saving updated account "
            "records file for instrument definition: ")(
            strInstrumentDefinitionID)(" to contain account ID: ")(
            strAcctID.get())(".")
            .Flush();
        return false;
    }

    // Okay, we saved the updated file, with the account added. (done, success.)
    //
    return true;
}

auto Unit::calculate_id() const -> identifier_type
{
    return GetID(api_, IDVersion());
}

auto Unit::contract() const -> SerializedType
{
    auto contract = SigVersion();

    if (const auto sigs = signatures(); false == sigs.empty()) {
        contract.mutable_signature()->CopyFrom(*sigs.front());
    }

    return contract;
}

auto Unit::DisplayStatistics(String& strContents) const -> bool
{
    const auto type = [&] {
        switch (Type()) {
            case contract::UnitDefinitionType::Currency: {

                return "currency"sv;
            }
            case contract::UnitDefinitionType::Security: {

                return "security"sv;
            }
            case contract::UnitDefinitionType::Basket: {

                return "basket currency"sv;
            }
            case contract::UnitDefinitionType::Error:
            default: {

                return "error"sv;
            }
        }
    }();
    strContents.Concatenate(" Asset Type: "sv)
        .Concatenate(type)
        .Concatenate(" InstrumentDefinitionID: "sv)
        .Concatenate(ID().asBase58(api_.Crypto()))
        .Concatenate("\n\n"sv);

    return true;
}

auto Unit::EraseAccountRecord(
    const UnallocatedCString& dataFolder,
    const identifier::Account& theAcctID) const -> bool
{
    const auto strAcctID = String::Factory(theAcctID, api_.Crypto());
    const auto strInstrumentDefinitionID = ID().asBase58(api_.Crypto());
    auto strAcctRecordFile =
        api::internal::Paths::GetFilenameA(strInstrumentDefinitionID.c_str());

    OTDB::Storable* pStorable = nullptr;
    std::unique_ptr<OTDB::Storable> theAngel;
    OTDB::StringMap* pMap = nullptr;

    if (OTDB::Exists(
            api_,
            dataFolder,
            api_.Internal().Paths().Contract(),
            strAcctRecordFile,
            "",
            "")) {  // the file already exists; let's
                    // try to load it up.
        pStorable = OTDB::QueryObject(
            api_,
            OTDB::STORED_OBJ_STRING_MAP,
            dataFolder,
            api_.Internal().Paths().Contract(),
            strAcctRecordFile,
            "",
            "");
    } else {  // the account records file (for this instrument definition)
              // doesn't
              // exist.
        pStorable = OTDB::CreateObject(
            OTDB::STORED_OBJ_STRING_MAP);  // this asserts already, on failure.
    }

    theAngel.reset(pStorable);
    pMap = (nullptr == pStorable) ? nullptr
                                  : dynamic_cast<OTDB::StringMap*>(pStorable);

    // It exists.
    //
    if (nullptr == pMap) {
        LogError()()(
            "Error: Failed trying to load or create the account records file "
            "for instrument definition: ")(strInstrumentDefinitionID)(".")
            .Flush();
        return false;
    }

    // Before we can erase it, let's see if it's even there....
    //
    auto& theMap = pMap->the_map_;
    auto map_it = theMap.find(strAcctID->Get());

    // we found it!
    if (theMap.end() != map_it)  //  Acct ID was already there...
    {
        theMap.erase(map_it);  // remove it
    }

    // it wasn't already on the list...
    // (So it's like success, since the end result is, acct ID will not appear
    // on this list--whether
    // it was there or not beforehand, it's definitely not there now.)

    // Then save it back to local storage:
    //
    if (!OTDB::StoreObject(
            api_,
            *pMap,
            dataFolder,
            api_.Internal().Paths().Contract(),
            strAcctRecordFile,
            "",
            "")) {
        LogError()()(
            "Failed trying to StoreObject, while saving updated account "
            "records file for instrument definition: ")(
            strInstrumentDefinitionID)(" to erase account ID: ")(
            strAcctID.get())(".")
            .Flush();
        return false;
    }

    // Okay, we saved the updated file, with the account removed. (done,
    // success.)
    //
    return true;
}

auto Unit::get_displayscales(const SerializedType& serialized) const
    -> std::optional<display::Definition>
{
    if (serialized.has_params()) {
        const auto& params = serialized.params();
        auto scales = display::Scales{};

        for (const auto& scale : params.scales()) {
            scales.emplace_back(std::pair(
                scale.name(),
                factory::DisplayScale(
                    scale.prefix(),
                    scale.suffix(),
                    [&] {
                        auto out = Vector<display::Ratio>{};

                        for (const auto& ratio : scale.ratios()) {
                            out.emplace_back(
                                std::pair{ratio.base(), ratio.power()});
                        }

                        return out;
                    }(),
                    scale.default_minimum_decimals(),
                    scale.default_maximum_decimals())));
        }

        if (const auto count = scales.size(); 0 < count) {

            return display::Definition{factory::DisplayDefinition(
                params.short_name(),
                0,
                convert_to_size<std::size_t, display::ScaleIndex>(count - 1_uz),
                std::move(scales))};
        }
    }
    return {};
}

auto Unit::GetID(const api::Session& api, const SerializedType& contract)
    -> identifier_type
{
    return api.Factory().Internal().Session().UnitIDFromPreimage(contract);
}

auto Unit::get_unitofaccount(const SerializedType& serialized) const
    -> opentxs::UnitType
{
    if (serialized.has_params()) {
        return ClaimToUnit(translate(serialized.params().unit_of_account()));
    } else {
        return opentxs::UnitType::Custom;
    }
}

auto Unit::IDVersion() const -> SerializedType
{
    SerializedType contract;
    contract.set_version(Version());
    contract.clear_id();          // reinforcing that this field must be blank.
    contract.clear_signature();   // reinforcing that this field must be blank.
    contract.clear_issuer_nym();  // reinforcing that this field must be blank.

    if (Signer()) {
        Signer()->ID().Internal().Serialize(*contract.mutable_issuer());
    }

    redemption_increment_.Serialize(
        writer(contract.mutable_redemption_increment()));
    contract.set_name(short_name_);
    contract.set_terms(UnallocatedCString{Terms()});
    contract.set_type(translate(Type()));

    auto& currency = *contract.mutable_params();
    currency.set_version(1);
    currency.set_unit_of_account(translate(UnitToClaim(UnitOfAccount())));
    currency.set_short_name(UnallocatedCString{Name()});

    if (display_definition_.has_value()) {
        const auto count = display_definition_->ScaleCount();

        for (auto i = 0u; i < count; ++i) {
            const auto scale = display_definition_->Scale(i);
            const auto prefix = scale.Prefix();
            const auto suffix = scale.Suffix();
            auto& serialized = *currency.add_scales();
            serialized.set_version(1);
            serialized.set_name(short_name_);
            serialized.set_prefix(prefix.data(), prefix.size());
            serialized.set_suffix(suffix.data(), suffix.size());
            serialized.set_default_minimum_decimals(
                scale.DefaultMinDecimals().value_or(0));
            serialized.set_default_maximum_decimals(
                scale.DefaultMaxDecimals().value_or(0));
            for (const auto& ratio : scale.Ratios()) {
                auto& ratios = *serialized.add_ratios();
                ratios.set_version(1);
                ratios.set_base(ratio.first);
                ratios.set_power(ratio.second);
            }
        }
    }

    return contract;
}

auto Unit::Serialize(Writer&& out) const noexcept -> bool
{
    return serialize(contract(), std::move(out));
}

auto Unit::Serialize(Writer&& destination, bool includeNym) const -> bool
{
    auto serialized = protobuf::UnitDefinition{};

    if (false == Serialize(serialized, includeNym)) {
        LogError()()("Failed to serialize unit definition.").Flush();
        return false;
    }

    write(serialized, std::move(destination));

    return true;
}

auto Unit::Serialize(SerializedType& serialized, bool includeNym) const -> bool
{
    serialized = contract();

    if (includeNym && Signer()) {
        auto publicNym = protobuf::Nym{};
        if (false == Signer()->Internal().Serialize(publicNym)) {
            return false;
        }
        *(serialized.mutable_issuer_nym()) = publicNym;
    }

    return true;
}

auto Unit::SetAlias(std::string_view alias) noexcept -> bool
{
    InitAlias(alias);
    api_.Wallet().SetUnitDefinitionAlias(ID(), alias);

    return true;
}

auto Unit::SigVersion() const -> SerializedType
{
    auto contract = IDVersion();
    ID().Internal().Serialize(*contract.mutable_id());

    return contract;
}

auto Unit::update_signature(const PasswordPrompt& reason) -> bool
{
    if (!Signable::update_signature(reason)) { return false; }

    auto success = false;
    auto sigs = Signatures{};
    auto serialized = SigVersion();
    auto& signature = *serialized.mutable_signature();
    success = Signer()->Internal().Sign(
        serialized, crypto::SignatureRole::UnitDefinition, signature, reason);

    if (success) {
        sigs.emplace_back(new protobuf::Signature(signature));
        add_signatures(std::move(sigs));
    } else {
        LogError()()("Failed to create signature.").Flush();
    }

    return success;
}

auto Unit::validate() const -> bool
{
    auto validNym = false;

    if (Signer()) { validNym = Signer()->VerifyPseudonym(); }

    const auto validSyntax =
        protobuf::syntax::check(LogError(), contract(), true);
    const auto sigs = signatures();

    if (1_uz != sigs.size()) {
        LogError()()("Missing signature.").Flush();

        return false;
    }

    auto validSig = false;
    const auto& signature = sigs.front();

    if (signature) { validSig = verify_signature(*signature); }

    return (validNym && validSyntax && validSig);
}

auto Unit::verify_signature(const protobuf::Signature& signature) const -> bool
{
    if (!Signable::verify_signature(signature)) { return false; }

    auto serialized = SigVersion();
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return Signer()->Internal().Verify(serialized, sigProto);
}

// currently only "user" accounts (normal user asset accounts) are added to
// this list Any "special" accounts, such as basket reserve accounts, or voucher
// reserve accounts, or cash reserve accounts, are not included on this list.
auto Unit::VisitAccountRecords(
    const UnallocatedCString& dataFolder,
    AccountVisitor& visitor,
    const PasswordPrompt& reason) const -> bool
{
    const auto strInstrumentDefinitionID = ID().asBase58(api_.Crypto());
    auto record_file =
        api::internal::Paths::GetFilenameA(strInstrumentDefinitionID.c_str());
    const std::unique_ptr<OTDB::Storable> pStorable(OTDB::QueryObject(
        api_,
        OTDB::STORED_OBJ_STRING_MAP,
        dataFolder,
        api_.Internal().Paths().Contract(),
        record_file,
        "",
        ""));

    auto* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());

    // There was definitely a StringMap loaded from local storage.
    // (Even an empty one, possibly.) This is the only block that matters in
    // this function.
    //
    if (nullptr != pMap) {
        const auto& pNotaryID = visitor.GetNotaryID();

        assert_false(pNotaryID.empty());

        auto& theMap = pMap->the_map_;

        // todo: optimize: will probably have to use a database for this,
        // std::int64_t term.
        // (What if there are a million acct IDs in this flat file? Not
        // scaleable.)
        //
        for (auto& it : theMap) {
            const UnallocatedCString& str_acct_id =
                it.first;  // Containing the account ID.
            const UnallocatedCString& str_instrument_definition_id =
                it.second;  // Containing the instrument definition ID. (Just in
                            // case
                            // someone copied the wrong file here...)

            if (strInstrumentDefinitionID != str_instrument_definition_id) {
                LogError()()("Error: wrong instrument definition ID (")(
                    str_instrument_definition_id)(") when expecting: ")(
                    strInstrumentDefinitionID)(".")
                    .Flush();
            } else {
                const auto& wallet = api_.Wallet();
                const auto accountID =
                    api_.Factory().AccountIDFromBase58(str_acct_id);
                auto account = wallet.Internal().Account(accountID);

                if (false == bool(account)) {
                    LogError()()("Unable to load account ")(str_acct_id)(".")
                        .Flush();

                    continue;
                }

                if (false == visitor.Trigger(account.get(), reason)) {
                    LogError()()("Error: Trigger failed for account ")(
                        str_acct_id)
                        .Flush();
                }
            }
        }

        return true;
    }

    return true;
}
}  // namespace opentxs::contract::implementation
