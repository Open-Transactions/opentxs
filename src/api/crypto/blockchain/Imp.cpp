// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/Imp.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <bech32.h>
#include <segwit_addr.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"    // IWYU pragma: keep
#include "opentxs/crypto/Bip43Purpose.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Bip44Type.hpp"     // IWYU pragma: keep
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"
#include "util/HDIndex.hpp"

#define PATH_VERSION 1
#define COMPRESSED_PUBKEY_SIZE 33

namespace opentxs::api::crypto
{
enum class Prefix : std::uint8_t {
    Unknown = 0,
    BitcoinP2PKH,
    BitcoinP2SH,
    BitcoinTestnetP2PKH,
    BitcoinTestnetP2SH,
    LitecoinP2PKH,
    LitecoinP2SH,
    LitecoinTestnetP2SH,
    PKTP2PKH,
    PKTP2SH,
    DashP2PKH,
    DashP2SH,
    DashTestnetP2PKH,
    DashTestnetP2SH,
};

using Style = crypto::Blockchain::Style;
using AddressMap = UnallocatedMap<Prefix, UnallocatedCString>;
using AddressReverseMap = UnallocatedMap<UnallocatedCString, Prefix>;
using StylePair = std::pair<Style, opentxs::blockchain::Type>;
// Style, preferred prefix, additional prefixes
using StyleMap =
    UnallocatedMap<StylePair, std::pair<Prefix, UnallocatedSet<Prefix>>>;
using StyleReverseMap = UnallocatedMap<Prefix, UnallocatedSet<StylePair>>;
using HrpMap = UnallocatedMap<opentxs::blockchain::Type, UnallocatedCString>;
using HrpReverseMap =
    UnallocatedMap<UnallocatedCString, opentxs::blockchain::Type>;

auto reverse(const StyleMap& in) noexcept -> StyleReverseMap;
auto reverse(const StyleMap& in) noexcept -> StyleReverseMap
{
    auto output = StyleReverseMap{};
    std::for_each(std::begin(in), std::end(in), [&](const auto& data) {
        const auto& [metadata, prefixData] = data;
        const auto& [preferred, additional] = prefixData;
        output[preferred].emplace(metadata);

        for (const auto& prefix : additional) {
            output[prefix].emplace(metadata);
        }
    });

    return output;
}

const AddressReverseMap address_prefix_reverse_map_{
    {"00", Prefix::BitcoinP2PKH},
    {"05", Prefix::BitcoinP2SH},
    {"10", Prefix::DashP2SH},
    {"13", Prefix::DashTestnetP2SH},
    {"30", Prefix::LitecoinP2PKH},
    {"32", Prefix::LitecoinP2SH},
    {"38", Prefix::PKTP2SH},
    {"3a", Prefix::LitecoinTestnetP2SH},
    {"4c", Prefix::DashP2PKH},
    {"6f", Prefix::BitcoinTestnetP2PKH},
    {"75", Prefix::PKTP2PKH},
    {"8c", Prefix::DashTestnetP2PKH},
    {"c4", Prefix::BitcoinTestnetP2SH},
};
const AddressMap address_prefix_map_{reverse_map(address_prefix_reverse_map_)};
const StyleMap address_style_map_{
    {{Style::P2PKH, opentxs::blockchain::Type::UnitTest},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::BitcoinCash_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::BitcoinCash_testnet4},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::BitcoinCash},
     {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::eCash_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::eCash},
     {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Bitcoin_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Bitcoin},
     {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Litecoin_testnet4},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Litecoin},
     {Prefix::LitecoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::PKT_testnet},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::PKT}, {Prefix::PKTP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::BitcoinSV_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::BitcoinSV},
     {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Dash_testnet3},
     {Prefix::DashTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Dash}, {Prefix::DashP2PKH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::UnitTest},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::BitcoinCash_testnet3},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::BitcoinCash_testnet4},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::BitcoinCash},
     {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::eCash_testnet3},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::eCash},
     {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Bitcoin_testnet3},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Bitcoin},
     {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Litecoin_testnet4},
     {Prefix::LitecoinTestnetP2SH, {Prefix::BitcoinTestnetP2SH}}},
    {{Style::P2SH, opentxs::blockchain::Type::Litecoin},
     {Prefix::LitecoinP2SH, {Prefix::BitcoinP2SH}}},
    {{Style::P2SH, opentxs::blockchain::Type::PKT_testnet},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::PKT}, {Prefix::PKTP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::BitcoinSV_testnet3},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::BitcoinSV},
     {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Dash_testnet3},
     {Prefix::DashTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Dash}, {Prefix::DashP2SH, {}}},
};
const StyleReverseMap address_style_reverse_map_{reverse(address_style_map_)};
const HrpMap hrp_map_{
    {opentxs::blockchain::Type::Bitcoin, "bc"},
    {opentxs::blockchain::Type::Bitcoin_testnet3, "tb"},
    {opentxs::blockchain::Type::Litecoin, "ltc"},
    {opentxs::blockchain::Type::Litecoin_testnet4, "tltc"},
    {opentxs::blockchain::Type::PKT, "pkt"},
    {opentxs::blockchain::Type::PKT_testnet, "tpk"},
    {opentxs::blockchain::Type::UnitTest, "bcrt"},
};
const HrpReverseMap hrp_reverse_map_{reverse_map(hrp_map_)};
}  // namespace opentxs::api::crypto

namespace opentxs::api::crypto::imp
{
Blockchain::Imp::Imp(
    const api::Session& api,
    const api::session::Contacts& contacts,
    api::crypto::Blockchain& parent) noexcept
    : api_(api)
    , contacts_(contacts)
    , blank_(api_.Factory().Data(), Style::Unknown, {}, false)
    , balance_oracle_endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
    , parent_(parent)
    , lock_()
    , nym_lock_()
    , accounts_(api_)
    , wallets_(api_, contacts_, parent)
{
}

auto Blockchain::Imp::Account(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain) const noexcept(false)
    -> const opentxs::blockchain::crypto::Account&
{
    if (false == validate_nym(nymID)) {
        using namespace std::literals;
        const auto error = CString{"Unable to load "sv}
                               .append(print(chain))
                               .append(" account for nym ("sv)
                               .append(nymID.asBase58(api_.Crypto())) +
                           ')';

        throw std::runtime_error{error.c_str()};
    }

    return Wallet(chain).Account(nymID);
}

auto Blockchain::Imp::AccountList(const identifier::Nym& nym) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    return wallets_.AccountList(nym);
}

auto Blockchain::Imp::AccountList(const opentxs::blockchain::Type chain)
    const noexcept -> UnallocatedSet<identifier::Account>
{
    return wallets_.AccountList(chain);
}

auto Blockchain::Imp::AccountList() const noexcept
    -> UnallocatedSet<identifier::Account>
{
    return wallets_.AccountList();
}

auto Blockchain::Imp::ActivityDescription(
    const identifier::Nym&,
    const identifier::Generic&,
    const std::string_view,
    alloc::Default,
    alloc::Default) const noexcept -> UnallocatedCString
{
    return {};
}

auto Blockchain::Imp::ActivityDescription(
    const identifier::Nym&,
    const opentxs::blockchain::Type,
    const opentxs::blockchain::block::Transaction&) const noexcept
    -> UnallocatedCString
{
    return {};
}

auto Blockchain::Imp::address_prefix(
    const Style style,
    const opentxs::blockchain::Type chain) const noexcept(false) -> ByteArray
{
    return api_.Factory().DataFromHex(
        address_prefix_map_.at(address_style_map_.at({style, chain}).first));
}

auto Blockchain::Imp::API() const noexcept -> const api::Crypto&
{
    return api_.Crypto();
}

auto Blockchain::Imp::AssignContact(
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    const Subchain subchain,
    const Bip32Index index,
    const identifier::Generic& contactID) const noexcept -> bool
{
    if (false == validate_nym(nymID)) { return false; }

    auto lock = Lock{nym_mutex(nymID)};

    const auto chain = UnitToBlockchain(
        api_.Storage().BlockchainSubaccountAccountType(nymID, accountID));

    OT_ASSERT(opentxs::blockchain::Type::UnknownBlockchain != chain);

    try {
        const auto& node =
            wallets_.Get(chain).Account(nymID).Subaccount(accountID);

        try {
            const auto& element = node.BalanceElement(subchain, index);
            const auto existing = element.Contact();

            if (contactID == existing) { return true; }

            return node.Internal().SetContact(subchain, index, contactID);
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("Failed to load balance element")
                .Flush();

            return false;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to load account").Flush();

        return false;
    }
}

auto Blockchain::Imp::AssignLabel(
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    const Subchain subchain,
    const Bip32Index index,
    const std::string_view label) const noexcept -> bool
{
    if (false == validate_nym(nymID)) { return false; }

    auto lock = Lock{nym_mutex(nymID)};

    const auto chain = UnitToBlockchain(
        api_.Storage().BlockchainSubaccountAccountType(nymID, accountID));

    OT_ASSERT(opentxs::blockchain::Type::UnknownBlockchain != chain);

    try {
        const auto& node =
            wallets_.Get(chain).Account(nymID).Subaccount(accountID);

        try {
            const auto& element = node.BalanceElement(subchain, index);

            if (label == element.Label()) { return true; }

            return node.Internal().SetLabel(subchain, index, label);
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("Failed to load balance element")
                .Flush();

            return false;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to load account").Flush();

        return false;
    }
}

auto Blockchain::Imp::AssignTransactionMemo(
    const TxidHex& id,
    const std::string_view label,
    alloc::Default monotonic) const noexcept -> bool
{
    return false;
}

auto Blockchain::Imp::bip44_type(const UnitType type) const noexcept
    -> Bip44Type
{
    using enum UnitType;
    using enum Bip44Type;

    switch (type) {
        case Btc: {

            return BITCOIN;
        }
        case Ltc: {

            return LITECOIN;
        }
        case Doge: {

            return DOGECOIN;
        }
        case Dash: {

            return DASH;
        }
        case Bch: {

            return BITCOINCASH;
        }
        case Pkt: {

            return PKT;
        }
        case Bsv: {

            return BITCOINSV;
        }
        case Xec: {

            return ECASH;
        }
        case Cspr: {

            return CASPER;
        }
        case Tn4bch:
        case Tnbch:
        case Tnbtc:
        case Tnxrp:
        case Tnltx:
        case Tnxem:
        case Tndash:
        case Tnmaid:
        case Tnlsk:
        case Tndoge:
        case Tnxmr:
        case Tnwaves:
        case Tnnxt:
        case Tnsc:
        case Tnsteem:
        case Tnpkt:
        case Tnbsv:
        case TnXec:
        case TnCspr:
        case Regtest: {
            return TESTNET;
        }
        case Error:
        case Eth:
        case Xrp:
        case Dao:
        case Xem:
        case Maid:
        case Lsk:
        case Dgd:
        case Xmr:
        case Waves:
        case Nxt:
        case Sc:
        case Steem:
        case Amp:
        case Xlm:
        case Fct:
        case Bts:
        case Usd:
        case Eur:
        case Gbp:
        case Inr:
        case Aud:
        case Cad:
        case Sgd:
        case Chf:
        case Myr:
        case Jpy:
        case Cny:
        case Nzd:
        case Thb:
        case Huf:
        case Aed:
        case Hkd:
        case Mxn:
        case Zar:
        case Php:
        case Sek:
        case Ethereum_olympic:
        case Ethereum_classic:
        case Ethereum_expanse:
        case Ethereum_morden:
        case Ethereum_ropsten:
        case Ethereum_rinkeby:
        case Ethereum_kovan:
        case Ethereum_sokol:
        case Ethereum_poa:
        case Bnb:
        case Sol:
        case Usdt:
        case Ada:
        case Dot:
        case Usdc:
        case Shib:
        case Luna:
        case Avax:
        case Uni:
        case Link:
        case Wbtc:
        case Busd:
        case MatiC:
        case Algo:
        case Vet:
        case Axs:
        case Icp:
        case Cro:
        case Atom:
        case Theta:
        case Fil:
        case Trx:
        case Ftt:
        case Etc:
        case Ftm:
        case Dai:
        case Btcb:
        case Egld:
        case Hbar:
        case Xtz:
        case Mana:
        case Near:
        case Grt:
        case Cake:
        case Eos:
        case Flow:
        case Aave:
        case Klay:
        case Ksm:
        case Miota:
        case Hnt:
        case Rune:
        case Leo:
        case Neo:
        case One:
        case Qnt:
        case Ust:
        case Mkr:
        case Enj:
        case Chz:
        case Ar:
        case Stx:
        case Btt:
        case Hot:
        case Sand:
        case Omg:
        case Celo:
        case Zec:
        case Comp:
        case Tfuel:
        case Kda:
        case Lrc:
        case Qtum:
        case Crv:
        case Ht:
        case Nexo:
        case Sushi:
        case Kcs:
        case Bat:
        case Okb:
        case Dcr:
        case Icx:
        case Rvn:
        case Scrt:
        case Rev:
        case Audio:
        case Zil:
        case Tusd:
        case Yfi:
        case Mina:
        case Perp:
        case Xdc:
        case Tel:
        case Snx:
        case Btg:
        case Afn:
        case All:
        case Amd:
        case Ang:
        case Aoa:
        case Ars:
        case Awg:
        case Azn:
        case Bam:
        case Bbd:
        case Bdt:
        case Bgn:
        case Bhd:
        case Bif:
        case Bmd:
        case Bnd:
        case Bob:
        case Brl:
        case Bsd:
        case Btn:
        case Bwp:
        case Byn:
        case Bzd:
        case Cdf:
        case Clp:
        case Cop:
        case Crc:
        case Cuc:
        case Cup:
        case Cve:
        case Czk:
        case Djf:
        case Dkk:
        case Dop:
        case Dzd:
        case Egp:
        case Ern:
        case Etb:
        case Fjd:
        case Fkp:
        case Gel:
        case Ggp:
        case Ghs:
        case Gip:
        case Gmd:
        case Gnf:
        case Gtq:
        case Gyd:
        case Hnl:
        case Hrk:
        case Htg:
        case Idr:
        case Ils:
        case Imp:
        case Iqd:
        case Irr:
        case Isk:
        case Jep:
        case Jmd:
        case Jod:
        case Kes:
        case Kgs:
        case Khr:
        case Kmf:
        case Kpw:
        case Krw:
        case Kwd:
        case Kyd:
        case Kzt:
        case Lak:
        case Lbp:
        case Lkr:
        case Lrd:
        case Lsl:
        case Lyd:
        case Mad:
        case Mdl:
        case Mga:
        case Mkd:
        case Mmk:
        case Mnt:
        case Mop:
        case Mru:
        case Mur:
        case Mvr:
        case Mwk:
        case Mzn:
        case Nad:
        case Ngn:
        case Nio:
        case Nok:
        case Npr:
        case Omr:
        case Pab:
        case Pen:
        case Pgk:
        case Pkr:
        case Pln:
        case Pyg:
        case Qar:
        case Ron:
        case Rsd:
        case Rub:
        case Rwf:
        case Sar:
        case Sbd:
        case Scr:
        case Sdg:
        case Shp:
        case Sll:
        case Sos:
        case Spl:
        case Srd:
        case Stn:
        case Svc:
        case Syp:
        case Szl:
        case Tjs:
        case Tmt:
        case Tnd:
        case Top:
        case Try:
        case Ttd:
        case Tvd:
        case Twd:
        case Tzs:
        case Uah:
        case Ugx:
        case Uyu:
        case Uzs:
        case Vef:
        case Vnd:
        case Vuv:
        case Wst:
        case Xaf:
        case Xcd:
        case Xdr:
        case Xof:
        case Xpf:
        case Yer:
        case Zmw:
        case Zwd:
        case Custom:
        case Unknown:
        default: {
            OT_FAIL;
        }
    }
}

auto Blockchain::Imp::CalculateAddress(
    const opentxs::blockchain::Type chain,
    const Style format,
    const Data& pubkey) const noexcept -> UnallocatedCString
{
    auto data = api_.Factory().Data();

    switch (format) {
        case Style::P2WPKH:
        case Style::P2PKH: {
            try {
                data = PubkeyHash(chain, pubkey);
            } catch (...) {
                LogError()(OT_PRETTY_CLASS())("Invalid public key.").Flush();

                return {};
            }
        } break;
        case Style::Unknown:
        case Style::P2SH:
        case Style::P2WSH:
        case Style::P2TR:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported address style (")(
                static_cast<std::uint16_t>(format))(")")
                .Flush();

            return {};
        }
    }

    return EncodeAddress(format, chain, data);
}

auto Blockchain::Imp::Confirm(
    const Key key,
    const opentxs::blockchain::block::TransactionHash& tx) const noexcept
    -> bool
{
    try {
        const auto& [accountID, subchain, index] = key;

        return get_node(accountID).Internal().Confirm(subchain, index, tx);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Blockchain::Imp::DecodeAddress(
    const std::string_view encoded) const noexcept -> DecodedAddress
{
    static constexpr auto check =
        [](DecodedAddress& output) -> DecodedAddress& {
        auto& [data, style, chains, supported] = output;
        supported = false;

        if (0 == data.size()) { return output; }
        if (Style::Unknown == style) { return output; }
        if (0 == chains.size()) { return output; }

        for (const auto& chain : chains) {
            try {
                if (opentxs::blockchain::params::get(chain).IsAllowed(style)) {
                    supported = true;

                    break;
                }
            } catch (...) {
            }
        }

        return output;
    };
    auto output = decode_bech23(encoded);

    if (output.has_value()) { return check(output.value()); }

    output = decode_legacy(encoded);

    if (output.has_value()) { return check(output.value()); }

    return blank_;
}

auto Blockchain::Imp::decode_bech23(const std::string_view encoded)
    const noexcept -> std::optional<DecodedAddress>
{
    auto output{blank_};
    auto& [data, style, chains, supported] = output;

    try {
        // TODO bech32::decode should accept string_view
        const auto compat = UnallocatedCString{encoded};
        const auto result = bech32::decode(compat);
        using Encoding = bech32::Encoding;

        switch (result.encoding) {
            case Encoding::BECH32:
            case Encoding::BECH32M: {
            } break;
            case Encoding::INVALID:
            default: {
                throw std::runtime_error("not bech32");
            }
        }

        const auto [version, bytes] = segwit_addr::decode(result.hrp, compat);

        try {
            switch (version) {
                case 0: {
                    switch (bytes.size()) {
                        case 20: {
                            style = Style::P2WPKH;
                        } break;
                        case 32: {
                            style = Style::P2WSH;
                        } break;
                        default: {
                            throw std::runtime_error{
                                "unknown version 0 program"};
                        }
                    }
                } break;
                case 1: {
                    switch (bytes.size()) {
                        case 32: {
                            style = Style::P2TR;
                        } break;
                        default: {
                            throw std::runtime_error{
                                "unknown version 1 program"};
                        }
                    }
                } break;
                case -1:
                default: {
                    throw std::runtime_error{"Unsupported version"};
                }
            }

            copy(reader(bytes), data.WriteInto());
            chains.emplace(hrp_reverse_map_.at(result.hrp));

            return output;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return blank_;
        }
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto Blockchain::Imp::decode_legacy(const std::string_view encoded)
    const noexcept -> std::optional<DecodedAddress>
{
    auto output{blank_};
    auto& [data, style, chains, supported] = output;

    try {
        const auto bytes = [&] {
            auto out = ByteArray{};
            const auto rc = api_.Crypto().Encode().Base58CheckDecode(
                encoded, out.WriteInto());

            if (false == rc) {
                throw std::runtime_error("base58 decode failure");
            }

            return out;
        }();
        auto type = api_.Factory().Data();

        try {
            switch (bytes.size()) {
                case 21: {
                    bytes.Extract(1, type, 0);
                    auto prefix{Prefix::Unknown};

                    try {
                        prefix = address_prefix_reverse_map_.at(type.asHex());
                    } catch (...) {
                        throw std::runtime_error(
                            "unable to decode version byte");
                    }

                    const auto& map = address_style_reverse_map_.at(prefix);

                    for (const auto& [decodeStyle, decodeChain] : map) {
                        style = decodeStyle;
                        chains.emplace(decodeChain);
                    }

                    bytes.Extract(20, data, 1);
                } break;
                default: {
                    throw std::runtime_error("unknown address format");
                }
            }

            return output;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return blank_;
        }
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return std::nullopt;
    }
}

auto Blockchain::Imp::EncodeAddress(
    const Style style,
    const opentxs::blockchain::Type chain,
    const Data& data) const noexcept -> UnallocatedCString
{
    switch (style) {
        case Style::P2WPKH: {

            return p2wpkh(chain, data);
        }
        case Style::P2PKH: {

            return p2pkh(chain, data);
        }
        case Style::P2SH: {

            return p2sh(chain, data);
        }
        case Style::Unknown:
        case Style::P2WSH:
        case Style::P2TR:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported address style (")(
                static_cast<std::uint16_t>(style))(")")
                .Flush();

            return {};
        }
    }
}

auto Blockchain::Imp::GetKey(const Key& id) const noexcept(false)
    -> const opentxs::blockchain::crypto::Element&
{
    const auto& [account, subchain, index] = id;
    using Type = opentxs::blockchain::crypto::SubaccountType;

    switch (accounts_.Type(account)) {
        case Type::HD: {
            const auto& hd = HDSubaccount(accounts_.Owner(account), account);

            return hd.BalanceElement(subchain, index);
        }
        case Type::PaymentCode: {
            const auto& pc =
                PaymentCodeSubaccount(accounts_.Owner(account), account);

            return pc.BalanceElement(subchain, index);
        }
        case Type::Imported:
        case Type::Error:
        case Type::Notification:
        default: {
            throw std::out_of_range("key not found");
        }
    }
}

auto Blockchain::Imp::get_node(const identifier::Account& accountID) const
    noexcept(false) -> opentxs::blockchain::crypto::Subaccount&
{
    const auto& nymID = accounts_.Owner(accountID);
    const auto& wallet = [&]() -> auto& {
        const auto type =
            api_.Storage().BlockchainSubaccountAccountType(nymID, accountID);

        if (UnitType::Error == type) {
            const auto error =
                UnallocatedCString{"unable to determine unit type for "
                                   "blockchain subaccount "} +
                accountID.asBase58(api_.Crypto()) + " belonging to nym " +
                nymID.asBase58(api_.Crypto());

            throw std::out_of_range(error);
        }

        return wallets_.Get(UnitToBlockchain(type));
    }();
    const auto& account = wallet.Account(nymID);
    const auto& subaccount =
        [&]() -> const opentxs::blockchain::crypto::Subaccount& {
        switch (accounts_.Type(accountID)) {
            case opentxs::blockchain::crypto::SubaccountType::HD: {

                return account.GetHD().at(accountID);
            }
            case opentxs::blockchain::crypto::SubaccountType::PaymentCode: {

                return account.GetPaymentCode().at(accountID);
            }
            case opentxs::blockchain::crypto::SubaccountType::Imported:
            case opentxs::blockchain::crypto::SubaccountType::Error:
            case opentxs::blockchain::crypto::SubaccountType::Notification:
            default: {
                throw std::out_of_range("subaccount type not supported");
            }
        }
    }();

    return subaccount.Internal();
}

auto Blockchain::Imp::HDSubaccount(
    const identifier::Nym& nymID,
    const identifier::Account& accountID) const noexcept(false)
    -> const opentxs::blockchain::crypto::HD&
{
    const auto type =
        api_.Storage().BlockchainSubaccountAccountType(nymID, accountID);

    if (UnitType::Error == type) {
        const auto error = UnallocatedCString{"HD account "} +
                           accountID.asBase58(api_.Crypto()) + " for " +
                           nymID.asBase58(api_.Crypto()) + " does not exist";

        throw std::out_of_range(error);
    }

    auto& wallet = wallets_.Get(UnitToBlockchain(type));
    auto& account = wallet.Account(nymID);

    return account.GetHD().at(accountID);
}

auto Blockchain::Imp::IndexItem(const ReadView bytes) const noexcept
    -> opentxs::blockchain::block::ElementHash
{
    return {};
}

auto Blockchain::Imp::Init() noexcept -> void { accounts_.Populate(); }

auto Blockchain::Imp::init_path(
    const std::string_view root,
    const UnitType chain,
    const Bip32Index account,
    const opentxs::blockchain::crypto::HDProtocol standard,
    proto::HDPath& path) const noexcept -> void
{
    using Standard = opentxs::blockchain::crypto::HDProtocol;
    path.set_version(PATH_VERSION);
    path.set_root(root.data(), root.size());

    switch (standard) {
        case Standard::BIP_32: {
            path.add_child(HDIndex{account, Bip32Child::HARDENED});
        } break;
        case Standard::BIP_44: {
            path.add_child(
                HDIndex{Bip43Purpose::HDWALLET, Bip32Child::HARDENED});
            path.add_child(HDIndex{bip44_type(chain), Bip32Child::HARDENED});
            path.add_child(account);
        } break;
        case Standard::BIP_49: {
            path.add_child(
                HDIndex{Bip43Purpose::P2SH_P2WPKH, Bip32Child::HARDENED});
            path.add_child(HDIndex{bip44_type(chain), Bip32Child::HARDENED});
            path.add_child(account);
        } break;
        case Standard::BIP_84: {
            path.add_child(HDIndex{Bip43Purpose::P2WPKH, Bip32Child::HARDENED});
            path.add_child(HDIndex{bip44_type(chain), Bip32Child::HARDENED});
            path.add_child(account);
        } break;
        case Standard::Error:
        default: {
            OT_FAIL;
        }
    }
}

auto Blockchain::Imp::KeyEndpoint() const noexcept -> std::string_view
{
    static const auto blank = CString{};

    return blank;
}

auto Blockchain::Imp::KeyGenerated(
    const opentxs::blockchain::Type,
    const identifier::Nym&,
    const identifier::Account&,
    const opentxs::blockchain::crypto::SubaccountType,
    const opentxs::blockchain::crypto::Subchain) const noexcept -> void
{
}

auto Blockchain::Imp::LoadTransaction(
    const TxidHex&,
    alloc::Default alloc,
    alloc::Default) const noexcept -> opentxs::blockchain::block::Transaction
{
    return {alloc};
}

auto Blockchain::Imp::LoadTransaction(
    const Txid&,
    alloc::Default alloc,
    alloc::Default) const noexcept -> opentxs::blockchain::block::Transaction
{
    return {alloc};
}

auto Blockchain::Imp::LookupAccount(
    const identifier::Account& id) const noexcept -> AccountData
{
    return wallets_.LookupAccount(id);
}

auto Blockchain::Imp::LookupContacts(const Data&) const noexcept -> ContactList
{
    return {};
}

auto Blockchain::Imp::NewHDSubaccount(
    const identifier::Nym& nymID,
    const opentxs::blockchain::crypto::HDProtocol standard,
    const opentxs::blockchain::Type derivationChain,
    const opentxs::blockchain::Type targetChain,
    const PasswordPrompt& reason) const noexcept -> identifier::Account
{
    static const auto blank = identifier::Account{};

    if (false == validate_nym(nymID)) { return blank; }

    if (opentxs::blockchain::Type::UnknownBlockchain == derivationChain) {
        LogError()(OT_PRETTY_CLASS())("Invalid derivationChain").Flush();

        return blank;
    }

    if (opentxs::blockchain::Type::UnknownBlockchain == targetChain) {
        LogError()(OT_PRETTY_CLASS())("Invalid targetChain").Flush();

        return blank;
    }

    auto nym = api_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym does not exist.").Flush();

        return blank;
    }

    auto nymPath = proto::HDPath{};

    if (false == nym->Internal().Path(nymPath)) {
        LogError()(OT_PRETTY_CLASS())("No nym path.").Flush();

        return blank;
    }

    if (0 == nymPath.root().size()) {
        LogError()(OT_PRETTY_CLASS())("Missing root.").Flush();

        return blank;
    }

    if (2 > nymPath.child().size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid path.").Flush();

        return blank;
    }

    auto accountPath = proto::HDPath{};
    init_path(
        nymPath.root(),
        BlockchainToUnit(derivationChain),
        HDIndex{nymPath.child(1), Bip32Child::HARDENED},
        standard,
        accountPath);

    try {
        auto accountID{blank};
        auto& tree = wallets_.Get(targetChain).Account(nymID);
        tree.Internal().AddHDNode(accountPath, standard, reason, accountID);

        OT_ASSERT(false == accountID.empty());

        LogVerbose()(OT_PRETTY_CLASS())("Created new HD subaccount ")(
            accountID, api_.Crypto())(" for ")(print(targetChain))(" account ")(
            tree.AccountID(), api_.Crypto())(" owned by ")(nymID.asBase58(
            api_.Crypto()))(" using path ")(opentxs::crypto::Print(accountPath))
            .Flush();
        accounts_.New(
            opentxs::blockchain::crypto::SubaccountType::HD,
            targetChain,
            accountID,
            nymID);
        notify_new_account(
            accountID,
            nymID,
            targetChain,
            opentxs::blockchain::crypto::SubaccountType::HD);

        return accountID;
    } catch (...) {
        LogVerbose()(OT_PRETTY_CLASS())("Failed to create account").Flush();

        return blank;
    }
}

auto Blockchain::Imp::NewNym(const identifier::Nym& id) const noexcept -> void
{
    for (const auto& chain : opentxs::blockchain::SupportedChains()) {
        Wallet(chain).Account(id);
    }
}

auto Blockchain::Imp::NewPaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath path,
    const opentxs::blockchain::Type chain,
    const PasswordPrompt& reason) const noexcept -> identifier::Account
{
    auto lock = Lock{nym_mutex(nymID)};

    return new_payment_code(lock, nymID, local, remote, path, chain, reason);
}

auto Blockchain::Imp::new_payment_code(
    const Lock&,
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath path,
    const opentxs::blockchain::Type chain,
    const PasswordPrompt& reason) const noexcept -> identifier::Account
{
    static const auto blank = identifier::Account{};

    if (false == validate_nym(nymID)) { return blank; }

    if (opentxs::blockchain::Type::UnknownBlockchain == chain) {
        LogError()(OT_PRETTY_CLASS())("Invalid chain").Flush();

        return blank;
    }

    auto nym = api_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym does not exist.").Flush();

        return blank;
    }

    if (0 == path.root().size()) {
        LogError()(OT_PRETTY_CLASS())("Missing root.").Flush();

        return blank;
    }

    if (3 > path.child().size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid path: ")(
            opentxs::crypto::Print(path))
            .Flush();

        return blank;
    }

    try {
        auto accountID{blank};
        auto& tree = wallets_.Get(chain).Account(nymID);
        tree.Internal().AddUpdatePaymentCode(
            local, remote, path, reason, accountID);

        OT_ASSERT(false == accountID.empty());

        LogVerbose()(OT_PRETTY_CLASS())("Created new payment code subaccount ")(
            accountID, api_.Crypto())(" for  ")(print(chain))(" account ")(
            tree.AccountID(),
            api_.Crypto())(" owned by ")(nymID, api_.Crypto())(
            " in reference to remote payment code ")(remote)
            .Flush();
        accounts_.New(
            opentxs::blockchain::crypto::SubaccountType::PaymentCode,
            chain,
            accountID,
            nymID);
        notify_new_account(
            accountID,
            nymID,
            chain,
            opentxs::blockchain::crypto::SubaccountType::PaymentCode);

        return accountID;
    } catch (...) {
        LogVerbose()(OT_PRETTY_CLASS())("Failed to create account").Flush();

        return blank;
    }
}

auto Blockchain::Imp::nym_mutex(const identifier::Nym& nym) const noexcept
    -> std::mutex&
{
    auto lock = Lock{lock_};

    return nym_lock_[nym];
}

auto Blockchain::Imp::Owner(const Key& key) const noexcept
    -> const identifier::Nym&
{
    const auto& [account, subchain, index] = key;
    static const auto blank = identifier::Nym{};

    if (Subchain::Outgoing == subchain) { return blank; }

    return Owner(account);
}

auto Blockchain::Imp::p2pkh(
    const opentxs::blockchain::Type chain,
    const Data& pubkeyHash) const noexcept -> UnallocatedCString
{
    try {
        auto preimage = address_prefix(Style::P2PKH, chain);

        OT_ASSERT(1 == preimage.size());

        preimage += pubkeyHash;

        OT_ASSERT(21 == preimage.size());

        auto out = UnallocatedCString{};
        const auto rc = api_.Crypto().Encode().Base58CheckEncode(
            preimage.Bytes(), writer(out));

        if (false == rc) { throw std::runtime_error{"base58 encode failure"}; }

        return out;
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Unsupported chain (")(print(chain))(")")
            .Flush();

        return "";
    }
}

auto Blockchain::Imp::p2sh(
    const opentxs::blockchain::Type chain,
    const Data& pubkeyHash) const noexcept -> UnallocatedCString
{
    try {
        auto preimage = address_prefix(Style::P2SH, chain);

        OT_ASSERT(1 == preimage.size());

        preimage += pubkeyHash;

        OT_ASSERT(21 == preimage.size());

        auto out = UnallocatedCString{};
        const auto rc = api_.Crypto().Encode().Base58CheckEncode(
            preimage.Bytes(), writer(out));

        if (false == rc) { throw std::runtime_error{"base58 encode failure"}; }

        return out;
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Unsupported chain (")(print(chain))(")")
            .Flush();

        return "";
    }
}

auto Blockchain::Imp::p2wpkh(
    const opentxs::blockchain::Type chain,
    const Data& hash) const noexcept -> UnallocatedCString
{
    try {
        const auto& hrp = hrp_map_.at(chain);
        const auto prog = [&] {
            auto out = UnallocatedVector<std::uint8_t>{};
            std::transform(
                hash.begin(),
                hash.end(),
                std::back_inserter(out),
                [](const auto& byte) {
                    return std::to_integer<std::uint8_t>(byte);
                });

            return out;
        }();

        return segwit_addr::encode(hrp, 0, prog);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Unsupported chain (")(print(chain))(")")
            .Flush();

        return "";
    }
}

auto Blockchain::Imp::PaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const identifier::Account& accountID) const noexcept(false)
    -> const opentxs::blockchain::crypto::PaymentCode&
{
    const auto type = api_.Storage().Bip47Chain(nymID, accountID);

    if (UnitType::Error == type) {
        const auto error = UnallocatedCString{"Payment code account "} +
                           accountID.asBase58(api_.Crypto()) + " for " +
                           nymID.asBase58(api_.Crypto()) + " does not exist";

        throw std::out_of_range(error);
    }

    auto& wallet = wallets_.Get(UnitToBlockchain(type));
    auto& account = wallet.Account(nymID);

    return account.GetPaymentCode().at(accountID);
}

auto Blockchain::Imp::PaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath path,
    const opentxs::blockchain::Type chain,
    const PasswordPrompt& reason) const noexcept(false)
    -> const opentxs::blockchain::crypto::PaymentCode&
{
    auto lock = Lock{nym_mutex(nymID)};
    const auto accountID =
        opentxs::blockchain::crypto::internal::PaymentCode::GetID(
            api_, chain, local, remote);
    const auto type = api_.Storage().Bip47Chain(nymID, accountID);

    if (UnitType::Error == type) {
        const auto id =
            new_payment_code(lock, nymID, local, remote, path, chain, reason);

        if (accountID != id) {
            throw std::out_of_range("Failed to create account");
        }
    }

    auto& balanceList = wallets_.Get(chain);
    auto& tree = balanceList.Account(nymID);

    return tree.GetPaymentCode().at(accountID);
}

auto Blockchain::Imp::ProcessContact(const Contact&, alloc::Default)
    const noexcept -> bool
{
    return false;
}

auto Blockchain::Imp::ProcessMergedContact(
    const Contact&,
    const Contact&,
    alloc::Default) const noexcept -> bool
{
    return false;
}

auto Blockchain::Imp::ProcessTransactions(
    const opentxs::blockchain::Type,
    Set<opentxs::blockchain::block::Transaction>&&,
    const PasswordPrompt&,
    alloc::Default) const noexcept -> bool
{
    return false;
}

auto Blockchain::Imp::PubkeyHash(
    [[maybe_unused]] const opentxs::blockchain::Type chain,
    const Data& pubkey) const noexcept(false) -> ByteArray
{
    if (pubkey.empty()) { throw std::runtime_error("Empty pubkey"); }

    if (COMPRESSED_PUBKEY_SIZE != pubkey.size()) {
        throw std::runtime_error("Incorrect pubkey size");
    }

    auto output = ByteArray{};

    if (false == api_.Crypto().Hash().Digest(
                     opentxs::crypto::HashType::Bitcoin,
                     pubkey.Bytes(),
                     output.WriteInto())) {
        throw std::runtime_error("Unable to calculate hash.");
    }

    return output;
}

auto Blockchain::Imp::RecipientContact(const Key& key) const noexcept
    -> identifier::Generic
{
    static const auto blank = identifier::Generic{};
    const auto& [accountID, subchain, index] = key;
    using Subchain = opentxs::blockchain::crypto::Subchain;

    if (is_notification(subchain)) { return blank; }

    const auto& owner = Owner(accountID);

    try {
        if (owner.empty()) {
            throw std::runtime_error{"Failed to load account owner"};
        }

        const auto& element = GetKey(key);

        switch (subchain) {
            case Subchain::Internal:
            case Subchain::External:
            case Subchain::Incoming: {

                return contacts_.NymToContact(owner);
            }
            case Subchain::Outgoing: {

                return element.Contact();
            }
            default: {

                return blank;
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return blank;
    }
}

auto Blockchain::Imp::Release(const Key key) const noexcept -> bool
{
    try {
        const auto& [accountID, subchain, index] = key;

        return get_node(accountID).Internal().Unreserve(subchain, index);
    } catch (...) {

        return false;
    }
}

auto Blockchain::Imp::ReportScan(
    const opentxs::blockchain::Type,
    const identifier::Nym&,
    const opentxs::blockchain::crypto::SubaccountType,
    const identifier::Account&,
    const Subchain,
    const opentxs::blockchain::block::Position&) const noexcept -> void
{
}

auto Blockchain::Imp::SenderContact(const Key& key) const noexcept
    -> identifier::Generic
{
    static const auto blank = identifier::Generic{};
    const auto& [accountID, subchain, index] = key;
    using Subchain = opentxs::blockchain::crypto::Subchain;

    if (is_notification(subchain)) { return blank; }

    const auto& owner = Owner(accountID);

    try {
        if (owner.empty()) {
            throw std::runtime_error{"Failed to load account owner"};
        }

        const auto& element = GetKey(key);

        switch (subchain) {
            case Subchain::Internal:
            case Subchain::Outgoing: {

                return contacts_.NymToContact(owner);
            }
            case Subchain::External:
            case Subchain::Incoming: {

                return element.Contact();
            }
            default: {

                return blank;
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return blank;
    }
}

auto Blockchain::Imp::Start(std::shared_ptr<const api::Session> api) noexcept
    -> void
{
    for (const auto& chain : opentxs::blockchain::SupportedChains()) {
        Wallet(chain);
    }
}

auto Blockchain::Imp::Unconfirm(
    const Key key,
    const opentxs::blockchain::block::TransactionHash& tx,
    const Time time,
    alloc::Default) const noexcept -> bool
{
    try {
        const auto& [accountID, subchain, index] = key;

        return get_node(accountID).Internal().Unconfirm(
            subchain, index, tx, time);
    } catch (...) {

        return false;
    }
}

auto Blockchain::Imp::UpdateElement(std::span<const ReadView>, alloc::Default)
    const noexcept -> void
{
}

auto Blockchain::Imp::validate_nym(const identifier::Nym& nymID) const noexcept
    -> bool
{
    if (false == nymID.empty()) {
        if (0 < api_.Wallet().LocalNyms().count(nymID)) { return true; }
    }

    return false;
}

auto Blockchain::Imp::Wallet(const opentxs::blockchain::Type chain) const
    noexcept(false) -> const opentxs::blockchain::crypto::Wallet&
{
    if (0 == opentxs::blockchain::DefinedChains().count(chain)) {
        throw std::runtime_error("Invalid chain");
    }

    return wallets_.Get(chain);
}
}  // namespace opentxs::api::crypto::imp
