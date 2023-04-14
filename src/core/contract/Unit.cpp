// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/Unit.hpp"  // IWYU pragma: associated

#include <CurrencyParams.pb.h>
#include <DisplayScale.pb.h>
#include <Nym.pb.h>
#include <ScaleRatio.pb.h>
#include <Signature.pb.h>
#include <UnitDefinition.pb.h>
#include <cmath>  // IWYU pragma: keep
#include <memory>
#include <sstream>  // IWYU pragma: keep
#include <string_view>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/Contract.hpp"
#include "internal/core/contract/CurrencyContract.hpp"
#include "internal/core/contract/SecurityContract.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/AccountVisitor.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
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
    const proto::UnitDefinition serialized) noexcept
    -> std::shared_ptr<contract::Unit>
{
    switch (translate(serialized.type())) {
        case contract::UnitType::Currency: {

            return CurrencyContract(api, nym, serialized);
        }
        case contract::UnitType::Security: {

            return SecurityContract(api, nym, serialized);
        }
        case contract::UnitType::Basket: {

            return BasketContract(api, nym, serialized);
        }
        case contract::UnitType::Error:
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
    : Signable(
          api,
          nym,
          version,
          terms,
          shortname,
          identifier::UnitDefinition{},
          {})
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
          api.Factory().UnitIDFromBase58(serialized.id()),
          serialized.has_signature()
              ? Signatures{std::make_shared<proto::Signature>(
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
    Lock lock(lock_);

    if (theAccount.GetInstrumentDefinitionID() != id_) {
        LogError()(OT_PRETTY_CLASS())("Error: theAccount doesn't have the same "
                                      "asset type ID as *this does.")
            .Flush();
        return false;
    }

    const auto theAcctID = api_.Factory().Internal().Identifier(theAccount);
    const auto strAcctID = String::Factory(theAcctID);

    const auto strInstrumentDefinitionID = String::Factory(id(lock));
    auto record_file =
        api::Legacy::GetFilenameA(strInstrumentDefinitionID->Get());

    OTDB::Storable* pStorable = nullptr;
    std::unique_ptr<OTDB::Storable> theAngel;
    OTDB::StringMap* pMap = nullptr;

    if (OTDB::Exists(
            api_,
            dataFolder,
            api_.Internal().Legacy().Contract(),
            record_file,
            "",
            "")) {  // the file already exists; let's
                    // try to load it up.
        pStorable = OTDB::QueryObject(
            api_,
            OTDB::STORED_OBJ_STRING_MAP,
            dataFolder,
            api_.Internal().Legacy().Contract(),
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
        LogError()(OT_PRETTY_CLASS())(
            "Error: Failed trying to load or create the account records "
            "file for instrument definition: ")(
            strInstrumentDefinitionID.get())(".")
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

        if (false ==
            strInstrumentDefinitionID->Compare(str2.c_str()))  // should
                                                               // never
        // happen.
        {
            LogError()(OT_PRETTY_CLASS())(
                "Error: wrong instrument definition found in "
                "account records "
                "file. For instrument definition: ")(
                strInstrumentDefinitionID.get())(". For account: ")(
                strAcctID.get())(". Found wrong instrument definition: ")(
                str2)(".")
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
    theMap[strAcctID->Get()] = strInstrumentDefinitionID->Get();

    // Then save it back to local storage:
    //
    if (!OTDB::StoreObject(
            api_,
            *pMap,
            dataFolder,
            api_.Internal().Legacy().Contract(),
            record_file,
            "",
            "")) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed trying to StoreObject, while saving updated "
            "account records file for instrument definition: ")(
            strInstrumentDefinitionID.get())(" to contain account ID: ")(
            strAcctID.get())(".")
            .Flush();
        return false;
    }

    // Okay, we saved the updated file, with the account added. (done, success.)
    //
    return true;
}

auto Unit::contract(const Lock& lock) const -> SerializedType
{
    auto contract = SigVersion(lock);

    if (1 <= signatures_.size()) {
        *(contract.mutable_signature()) = *(signatures_.front());
    }

    return contract;
}

auto Unit::DisplayStatistics(String& strContents) const -> bool
{
    const auto type = [&] {
        switch (Type()) {
            case contract::UnitType::Currency: {

                return "currency"sv;
            }
            case contract::UnitType::Security: {

                return "security"sv;
            }
            case contract::UnitType::Basket: {

                return "basket currency"sv;
            }
            case contract::UnitType::Error:
            default: {

                return "error"sv;
            }
        }
    }();
    strContents.Concatenate(" Asset Type: "sv)
        .Concatenate(type)
        .Concatenate(" InstrumentDefinitionID: "sv)
        .Concatenate(id_.asBase58(api_.Crypto()))
        .Concatenate("\n\n"sv);

    return true;
}

auto Unit::EraseAccountRecord(
    const UnallocatedCString& dataFolder,
    const identifier::Generic& theAcctID) const -> bool
{
    Lock lock(lock_);

    const auto strAcctID = String::Factory(theAcctID);

    const auto strInstrumentDefinitionID = String::Factory(id(lock));
    UnallocatedCString strAcctRecordFile =
        api::Legacy::GetFilenameA(strInstrumentDefinitionID->Get());

    OTDB::Storable* pStorable = nullptr;
    std::unique_ptr<OTDB::Storable> theAngel;
    OTDB::StringMap* pMap = nullptr;

    if (OTDB::Exists(
            api_,
            dataFolder,
            api_.Internal().Legacy().Contract(),
            strAcctRecordFile,
            "",
            "")) {  // the file already exists; let's
                    // try to load it up.
        pStorable = OTDB::QueryObject(
            api_,
            OTDB::STORED_OBJ_STRING_MAP,
            dataFolder,
            api_.Internal().Legacy().Contract(),
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
        LogError()(OT_PRETTY_CLASS())(
            "Error: Failed trying to load or create the account records "
            "file for instrument definition: ")(
            strInstrumentDefinitionID.get())(".")
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
            api_.Internal().Legacy().Contract(),
            strAcctRecordFile,
            "",
            "")) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed trying to StoreObject, while saving updated "
            "account records file for instrument definition: ")(
            strInstrumentDefinitionID.get())(" to erase account ID: ")(
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
        auto scales = display::Definition::Scales{};

        for (const auto& scale : params.scales()) {
            scales.emplace_back(std::pair(
                scale.name(),
                display::Scale{
                    scale.prefix(),
                    scale.suffix(),
                    [&] {
                        auto out = Vector<display::Scale::Ratio>{};

                        for (const auto& ratio : scale.ratios()) {
                            out.emplace_back(
                                std::pair{ratio.base(), ratio.power()});
                        }

                        return out;
                    }(),
                    scale.default_minimum_decimals(),
                    scale.default_maximum_decimals()}));
        }

        if (0 < scales.size()) {
            return {display::Definition(
                display::Definition::Name(params.short_name()),
                std::move(scales))};
        }
    }
    return {};
}

auto Unit::GetID(const Lock& lock) const -> identifier::Generic
{
    return GetID(api_, IDVersion(lock));
}

auto Unit::GetID(const api::Session& api, const SerializedType& contract)
    -> identifier::Generic
{
    return api.Factory().InternalSession().UnitIDFromPreimage(contract);
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

auto Unit::IDVersion(const Lock& lock) const -> SerializedType
{
    OT_ASSERT(verify_write_lock(lock));

    SerializedType contract;
    contract.set_version(version_);
    contract.clear_id();          // reinforcing that this field must be blank.
    contract.clear_signature();   // reinforcing that this field must be blank.
    contract.clear_issuer_nym();  // reinforcing that this field must be blank.

    if (nym_) {
        auto nymID = String::Factory();
        nym_->GetIdentifier(nymID);
        contract.set_issuer(nymID->Get());
    }

    redemption_increment_.Serialize(
        writer(contract.mutable_redemption_increment()));
    contract.set_name(short_name_);
    contract.set_terms(conditions_);
    contract.set_type(translate(Type()));

    auto& currency = *contract.mutable_params();
    currency.set_version(1);
    currency.set_unit_of_account(translate(UnitToClaim(UnitOfAccount())));
    currency.set_short_name(Name());

    if (display_definition_.has_value()) {
        for (const auto& [i, scale] : display_definition_->DisplayScales()) {
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

auto Unit::Serialize() const noexcept -> ByteArray
{
    Lock lock(lock_);

    return api_.Factory().InternalSession().Data(contract(lock));
}

auto Unit::Serialize(Writer&& destination, bool includeNym) const -> bool
{
    auto serialized = proto::UnitDefinition{};
    if (false == Serialize(serialized, includeNym)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize unit definition.")
            .Flush();
        return false;
    }

    write(serialized, std::move(destination));

    return true;
}

auto Unit::Serialize(SerializedType& serialized, bool includeNym) const -> bool
{
    Lock lock(lock_);

    serialized = contract(lock);

    if (includeNym && nym_) {
        auto publicNym = proto::Nym{};
        if (false == nym_->Internal().Serialize(publicNym)) { return false; }
        *(serialized.mutable_issuer_nym()) = publicNym;
    }

    return true;
}

auto Unit::SetAlias(const UnallocatedCString& alias) noexcept -> bool
{
    InitAlias(alias);
    api_.Wallet().SetUnitDefinitionAlias(
        api_.Factory().Internal().UnitIDConvertSafe(id_), alias);

    return true;
}

auto Unit::SigVersion(const Lock& lock) const -> SerializedType
{
    auto contract = IDVersion(lock);
    contract.set_id(id(lock).asBase58(api_.Crypto()).c_str());

    return contract;
}

auto Unit::update_signature(const Lock& lock, const PasswordPrompt& reason)
    -> bool
{
    if (!Signable::update_signature(lock, reason)) { return false; }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    success = nym_->Internal().Sign(
        serialized, crypto::SignatureRole::UnitDefinition, signature, reason);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to create signature.").Flush();
    }

    return success;
}

auto Unit::validate(const Lock& lock) const -> bool
{
    bool validNym = false;

    if (nym_) { validNym = nym_->VerifyPseudonym(); }

    const bool validSyntax = proto::Validate(contract(lock), VERBOSE, true);

    if (1 > signatures_.size()) {
        LogError()(OT_PRETTY_CLASS())("Missing signature.").Flush();

        return false;
    }

    bool validSig = false;
    const auto& signature = *signatures_.cbegin();

    if (signature) { validSig = verify_signature(lock, *signature); }

    return (validNym && validSyntax && validSig);
}

auto Unit::verify_signature(const Lock& lock, const proto::Signature& signature)
    const -> bool
{
    if (!Signable::verify_signature(lock, signature)) { return false; }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->Internal().Verify(serialized, sigProto);
}

// currently only "user" accounts (normal user asset accounts) are added to
// this list Any "special" accounts, such as basket reserve accounts, or voucher
// reserve accounts, or cash reserve accounts, are not included on this list.
auto Unit::VisitAccountRecords(
    const UnallocatedCString& dataFolder,
    AccountVisitor& visitor,
    const PasswordPrompt& reason) const -> bool
{
    Lock lock(lock_);
    const auto strInstrumentDefinitionID = String::Factory(id(lock));
    auto record_file =
        api::Legacy::GetFilenameA(strInstrumentDefinitionID->Get());
    std::unique_ptr<OTDB::Storable> pStorable(OTDB::QueryObject(
        api_,
        OTDB::STORED_OBJ_STRING_MAP,
        dataFolder,
        api_.Internal().Legacy().Contract(),
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

        OT_ASSERT(false == pNotaryID.empty());

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

            if (!strInstrumentDefinitionID->Compare(
                    str_instrument_definition_id.c_str())) {
                LogError()(OT_PRETTY_CLASS())("Error: wrong "
                                              "instrument definition ID (")(
                    str_instrument_definition_id)(") when expecting: ")(
                    strInstrumentDefinitionID.get())(".")
                    .Flush();
            } else {
                const auto& wallet = api_.Wallet();
                const auto accountID =
                    api_.Factory().IdentifierFromBase58(str_acct_id);
                auto account = wallet.Internal().Account(accountID);

                if (false == bool(account)) {
                    LogError()(OT_PRETTY_CLASS())("Unable to load account ")(
                        str_acct_id)(".")
                        .Flush();

                    continue;
                }

                if (false == visitor.Trigger(account.get(), reason)) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Error: Trigger failed for account ")(str_acct_id)
                        .Flush();
                }
            }
        }

        return true;
    }

    return true;
}
}  // namespace opentxs::contract::implementation
