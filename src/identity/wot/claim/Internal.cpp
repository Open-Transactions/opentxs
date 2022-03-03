// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "internal/identity/wot/claim/Types.hpp"  // IWYU pragma: associated

#include <robin_hood.h>

#include "opentxs/core/UnitType.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"
#include "util/Container.hpp"

namespace opentxs::identity::wot::claim
{
using AttributeMap = robin_hood::
    unordered_flat_map<claim::Attribute, proto::ContactItemAttribute>;
using AttributeReverseMap = robin_hood::
    unordered_flat_map<proto::ContactItemAttribute, claim::Attribute>;
using ClaimTypeMap =
    robin_hood::unordered_flat_map<claim::ClaimType, proto::ContactItemType>;
using ClaimTypeReverseMap =
    robin_hood::unordered_flat_map<proto::ContactItemType, claim::ClaimType>;
using SectionTypeMap = robin_hood::
    unordered_flat_map<claim::SectionType, proto::ContactSectionName>;
using SectionTypeReverseMap = robin_hood::
    unordered_flat_map<proto::ContactSectionName, claim::SectionType>;
using UnitTypeMap = robin_hood::unordered_flat_map<UnitType, claim::ClaimType>;
using UnitTypeReverseMap =
    robin_hood::unordered_flat_map<claim::ClaimType, UnitType>;
using NymTypeMap =
    robin_hood::unordered_flat_map<identity::Type, claim::ClaimType>;
using NymTypeReverseMap =
    robin_hood::unordered_flat_map<claim::ClaimType, identity::Type>;

auto attribute_map() noexcept -> const AttributeMap&;
auto claimtype_map() noexcept -> const ClaimTypeMap&;
auto identitytype_map() noexcept -> const NymTypeMap&;
auto sectiontype_map() noexcept -> const SectionTypeMap&;
auto unittype_map() noexcept -> const UnitTypeMap&;
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity::wot::claim
{
auto attribute_map() noexcept -> const AttributeMap&
{
    static const auto map = AttributeMap{
        {Attribute::Error, proto::CITEMATTR_ERROR},
        {Attribute::Active, proto::CITEMATTR_ACTIVE},
        {Attribute::Primary, proto::CITEMATTR_PRIMARY},
        {Attribute::Local, proto::CITEMATTR_LOCAL},
    };

    return map;
}

auto claimtype_map() noexcept -> const ClaimTypeMap&
{
    static const auto map = ClaimTypeMap{
        {ClaimType::Error, proto::CITEMTYPE_ERROR},
        {ClaimType::Individual, proto::CITEMTYPE_INDIVIDUAL},
        {ClaimType::Organization, proto::CITEMTYPE_ORGANIZATION},
        {ClaimType::Business, proto::CITEMTYPE_BUSINESS},
        {ClaimType::Government, proto::CITEMTYPE_GOVERNMENT},
        {ClaimType::Server, proto::CITEMTYPE_SERVER},
        {ClaimType::Prefix, proto::CITEMTYPE_PREFIX},
        {ClaimType::Forename, proto::CITEMTYPE_FORENAME},
        {ClaimType::Middlename, proto::CITEMTYPE_MIDDLENAME},
        {ClaimType::Surname, proto::CITEMTYPE_SURNAME},
        {ClaimType::Pedigree, proto::CITEMTYPE_PEDIGREE},
        {ClaimType::Suffix, proto::CITEMTYPE_SUFFIX},
        {ClaimType::Nickname, proto::CITEMTYPE_NICKNAME},
        {ClaimType::Commonname, proto::CITEMTYPE_COMMONNAME},
        {ClaimType::Passport, proto::CITEMTYPE_PASSPORT},
        {ClaimType::National, proto::CITEMTYPE_NATIONAL},
        {ClaimType::Provincial, proto::CITEMTYPE_PROVINCIAL},
        {ClaimType::Military, proto::CITEMTYPE_MILITARY},
        {ClaimType::Pgp, proto::CITEMTYPE_PGP},
        {ClaimType::Otr, proto::CITEMTYPE_OTR},
        {ClaimType::Ssl, proto::CITEMTYPE_SSL},
        {ClaimType::Physical, proto::CITEMTYPE_PHYSICAL},
        {ClaimType::Official, proto::CITEMTYPE_OFFICIAL},
        {ClaimType::Birthplace, proto::CITEMTYPE_BIRTHPLACE},
        {ClaimType::Home, proto::CITEMTYPE_HOME},
        {ClaimType::Website, proto::CITEMTYPE_WEBSITE},
        {ClaimType::Opentxs, proto::CITEMTYPE_OPENTXS},
        {ClaimType::Phone, proto::CITEMTYPE_PHONE},
        {ClaimType::Email, proto::CITEMTYPE_EMAIL},
        {ClaimType::Skype, proto::CITEMTYPE_SKYPE},
        {ClaimType::Wire, proto::CITEMTYPE_WIRE},
        {ClaimType::Qq, proto::CITEMTYPE_QQ},
        {ClaimType::Bitmessage, proto::CITEMTYPE_BITMESSAGE},
        {ClaimType::Whatsapp, proto::CITEMTYPE_WHATSAPP},
        {ClaimType::Telegram, proto::CITEMTYPE_TELEGRAM},
        {ClaimType::Kik, proto::CITEMTYPE_KIK},
        {ClaimType::Bbm, proto::CITEMTYPE_BBM},
        {ClaimType::Wechat, proto::CITEMTYPE_WECHAT},
        {ClaimType::Kakaotalk, proto::CITEMTYPE_KAKAOTALK},
        {ClaimType::Facebook, proto::CITEMTYPE_FACEBOOK},
        {ClaimType::Google, proto::CITEMTYPE_GOOGLE},
        {ClaimType::Linkedin, proto::CITEMTYPE_LINKEDIN},
        {ClaimType::Vk, proto::CITEMTYPE_VK},
        {ClaimType::Aboutme, proto::CITEMTYPE_ABOUTME},
        {ClaimType::Onename, proto::CITEMTYPE_ONENAME},
        {ClaimType::Twitter, proto::CITEMTYPE_TWITTER},
        {ClaimType::Medium, proto::CITEMTYPE_MEDIUM},
        {ClaimType::Tumblr, proto::CITEMTYPE_TUMBLR},
        {ClaimType::Yahoo, proto::CITEMTYPE_YAHOO},
        {ClaimType::Myspace, proto::CITEMTYPE_MYSPACE},
        {ClaimType::Meetup, proto::CITEMTYPE_MEETUP},
        {ClaimType::Reddit, proto::CITEMTYPE_REDDIT},
        {ClaimType::Hackernews, proto::CITEMTYPE_HACKERNEWS},
        {ClaimType::Wikipedia, proto::CITEMTYPE_WIKIPEDIA},
        {ClaimType::Angellist, proto::CITEMTYPE_ANGELLIST},
        {ClaimType::Github, proto::CITEMTYPE_GITHUB},
        {ClaimType::Bitbucket, proto::CITEMTYPE_BITBUCKET},
        {ClaimType::Youtube, proto::CITEMTYPE_YOUTUBE},
        {ClaimType::Vimeo, proto::CITEMTYPE_VIMEO},
        {ClaimType::Twitch, proto::CITEMTYPE_TWITCH},
        {ClaimType::Snapchat, proto::CITEMTYPE_SNAPCHAT},
        {ClaimType::Vine, proto::CITEMTYPE_VINE},
        {ClaimType::Instagram, proto::CITEMTYPE_INSTAGRAM},
        {ClaimType::Pinterest, proto::CITEMTYPE_PINTEREST},
        {ClaimType::Imgur, proto::CITEMTYPE_IMGUR},
        {ClaimType::Flickr, proto::CITEMTYPE_FLICKR},
        {ClaimType::Dribble, proto::CITEMTYPE_DRIBBLE},
        {ClaimType::Behance, proto::CITEMTYPE_BEHANCE},
        {ClaimType::Deviantart, proto::CITEMTYPE_DEVIANTART},
        {ClaimType::Spotify, proto::CITEMTYPE_SPOTIFY},
        {ClaimType::Itunes, proto::CITEMTYPE_ITUNES},
        {ClaimType::Soundcloud, proto::CITEMTYPE_SOUNDCLOUD},
        {ClaimType::Askfm, proto::CITEMTYPE_ASKFM},
        {ClaimType::Ebay, proto::CITEMTYPE_EBAY},
        {ClaimType::Etsy, proto::CITEMTYPE_ETSY},
        {ClaimType::Openbazaar, proto::CITEMTYPE_OPENBAZAAR},
        {ClaimType::Xboxlive, proto::CITEMTYPE_XBOXLIVE},
        {ClaimType::Playstation, proto::CITEMTYPE_PLAYSTATION},
        {ClaimType::Secondlife, proto::CITEMTYPE_SECONDLIFE},
        {ClaimType::Warcraft, proto::CITEMTYPE_WARCRAFT},
        {ClaimType::Alias, proto::CITEMTYPE_ALIAS},
        {ClaimType::Acquaintance, proto::CITEMTYPE_ACQUAINTANCE},
        {ClaimType::Friend, proto::CITEMTYPE_FRIEND},
        {ClaimType::Spouse, proto::CITEMTYPE_SPOUSE},
        {ClaimType::Sibling, proto::CITEMTYPE_SIBLING},
        {ClaimType::Member, proto::CITEMTYPE_MEMBER},
        {ClaimType::Colleague, proto::CITEMTYPE_COLLEAGUE},
        {ClaimType::Parent, proto::CITEMTYPE_PARENT},
        {ClaimType::Child, proto::CITEMTYPE_CHILD},
        {ClaimType::Employer, proto::CITEMTYPE_EMPLOYER},
        {ClaimType::Employee, proto::CITEMTYPE_EMPLOYEE},
        {ClaimType::Citizen, proto::CITEMTYPE_CITIZEN},
        {ClaimType::Photo, proto::CITEMTYPE_PHOTO},
        {ClaimType::Gender, proto::CITEMTYPE_GENDER},
        {ClaimType::Height, proto::CITEMTYPE_HEIGHT},
        {ClaimType::Weight, proto::CITEMTYPE_WEIGHT},
        {ClaimType::Hair, proto::CITEMTYPE_HAIR},
        {ClaimType::Eye, proto::CITEMTYPE_EYE},
        {ClaimType::Skin, proto::CITEMTYPE_SKIN},
        {ClaimType::Ethnicity, proto::CITEMTYPE_ETHNICITY},
        {ClaimType::Language, proto::CITEMTYPE_LANGUAGE},
        {ClaimType::Degree, proto::CITEMTYPE_DEGREE},
        {ClaimType::Certification, proto::CITEMTYPE_CERTIFICATION},
        {ClaimType::Title, proto::CITEMTYPE_TITLE},
        {ClaimType::Skill, proto::CITEMTYPE_SKILL},
        {ClaimType::Award, proto::CITEMTYPE_AWARD},
        {ClaimType::Likes, proto::CITEMTYPE_LIKES},
        {ClaimType::Sexual, proto::CITEMTYPE_SEXUAL},
        {ClaimType::Political, proto::CITEMTYPE_POLITICAL},
        {ClaimType::Religious, proto::CITEMTYPE_RELIGIOUS},
        {ClaimType::Birth, proto::CITEMTYPE_BIRTH},
        {ClaimType::Secondarygraduation, proto::CITEMTYPE_SECONDARYGRADUATION},
        {ClaimType::Universitygraduation,
         proto::CITEMTYPE_UNIVERSITYGRADUATION},
        {ClaimType::Wedding, proto::CITEMTYPE_WEDDING},
        {ClaimType::Accomplishment, proto::CITEMTYPE_ACCOMPLISHMENT},
        {ClaimType::Btc, proto::CITEMTYPE_BTC},
        {ClaimType::Eth, proto::CITEMTYPE_ETH},
        {ClaimType::Xrp, proto::CITEMTYPE_XRP},
        {ClaimType::Ltc, proto::CITEMTYPE_LTC},
        {ClaimType::Dao, proto::CITEMTYPE_DAO},
        {ClaimType::Xem, proto::CITEMTYPE_XEM},
        {ClaimType::Dash, proto::CITEMTYPE_DASH},
        {ClaimType::Maid, proto::CITEMTYPE_MAID},
        {ClaimType::Lsk, proto::CITEMTYPE_LSK},
        {ClaimType::Doge, proto::CITEMTYPE_DOGE},
        {ClaimType::Dgd, proto::CITEMTYPE_DGD},
        {ClaimType::Xmr, proto::CITEMTYPE_XMR},
        {ClaimType::Waves, proto::CITEMTYPE_WAVES},
        {ClaimType::Nxt, proto::CITEMTYPE_NXT},
        {ClaimType::Sc, proto::CITEMTYPE_SC},
        {ClaimType::Steem, proto::CITEMTYPE_STEEM},
        {ClaimType::Amp, proto::CITEMTYPE_AMP},
        {ClaimType::Xlm, proto::CITEMTYPE_XLM},
        {ClaimType::Fct, proto::CITEMTYPE_FCT},
        {ClaimType::Bts, proto::CITEMTYPE_BTS},
        {ClaimType::Usd, proto::CITEMTYPE_USD},
        {ClaimType::Eur, proto::CITEMTYPE_EUR},
        {ClaimType::Gbp, proto::CITEMTYPE_GBP},
        {ClaimType::Inr, proto::CITEMTYPE_INR},
        {ClaimType::Aud, proto::CITEMTYPE_AUD},
        {ClaimType::Cad, proto::CITEMTYPE_CAD},
        {ClaimType::Sgd, proto::CITEMTYPE_SGD},
        {ClaimType::Chf, proto::CITEMTYPE_CHF},
        {ClaimType::Myr, proto::CITEMTYPE_MYR},
        {ClaimType::Jpy, proto::CITEMTYPE_JPY},
        {ClaimType::Cny, proto::CITEMTYPE_CNY},
        {ClaimType::Nzd, proto::CITEMTYPE_NZD},
        {ClaimType::Thb, proto::CITEMTYPE_THB},
        {ClaimType::Huf, proto::CITEMTYPE_HUF},
        {ClaimType::Aed, proto::CITEMTYPE_AED},
        {ClaimType::Hkd, proto::CITEMTYPE_HKD},
        {ClaimType::Mxn, proto::CITEMTYPE_MXN},
        {ClaimType::Zar, proto::CITEMTYPE_ZAR},
        {ClaimType::Php, proto::CITEMTYPE_PHP},
        {ClaimType::Sek, proto::CITEMTYPE_SEK},
        {ClaimType::Tnbtc, proto::CITEMTYPE_TNBTC},
        {ClaimType::Tnxrp, proto::CITEMTYPE_TNXRP},
        {ClaimType::Tnltx, proto::CITEMTYPE_TNLTC},
        {ClaimType::Tnxem, proto::CITEMTYPE_TNXEM},
        {ClaimType::Tndash, proto::CITEMTYPE_TNDASH},
        {ClaimType::Tnmaid, proto::CITEMTYPE_TNMAID},
        {ClaimType::Tnlsk, proto::CITEMTYPE_TNLSK},
        {ClaimType::Tndoge, proto::CITEMTYPE_TNDOGE},
        {ClaimType::Tnxmr, proto::CITEMTYPE_TNXMR},
        {ClaimType::Tnwaves, proto::CITEMTYPE_TNWAVES},
        {ClaimType::Tnnxt, proto::CITEMTYPE_TNNXT},
        {ClaimType::Tnsc, proto::CITEMTYPE_TNSC},
        {ClaimType::Tnsteem, proto::CITEMTYPE_TNSTEEM},
        {ClaimType::Philosophy, proto::CITEMTYPE_PHILOSOPHY},
        {ClaimType::Met, proto::CITEMTYPE_MET},
        {ClaimType::Fan, proto::CITEMTYPE_FAN},
        {ClaimType::Supervisor, proto::CITEMTYPE_SUPERVISOR},
        {ClaimType::Subordinate, proto::CITEMTYPE_SUBORDINATE},
        {ClaimType::Contact, proto::CITEMTYPE_CONTACT},
        {ClaimType::Refreshed, proto::CITEMTYPE_REFRESHED},
        {ClaimType::Bot, proto::CITEMTYPE_BOT},
        {ClaimType::Bch, proto::CITEMTYPE_BCH},
        {ClaimType::Tnbch, proto::CITEMTYPE_TNBCH},
        {ClaimType::Owner, proto::CITEMTYPE_OWNER},
        {ClaimType::Property, proto::CITEMTYPE_PROPERTY},
        {ClaimType::Unknown, proto::CITEMTYPE_UNKNOWN},
        {ClaimType::Ethereum_olympic, proto::CITEMTYPE_ETHEREUM_OLYMPIC},
        {ClaimType::Ethereum_classic, proto::CITEMTYPE_ETHEREUM_CLASSIC},
        {ClaimType::Ethereum_expanse, proto::CITEMTYPE_ETHEREUM_EXPANSE},
        {ClaimType::Ethereum_morden, proto::CITEMTYPE_ETHEREUM_MORDEN},
        {ClaimType::Ethereum_ropsten, proto::CITEMTYPE_ETHEREUM_ROPSTEN},
        {ClaimType::Ethereum_rinkeby, proto::CITEMTYPE_ETHEREUM_RINKEBY},
        {ClaimType::Ethereum_kovan, proto::CITEMTYPE_ETHEREUM_KOVAN},
        {ClaimType::Ethereum_sokol, proto::CITEMTYPE_ETHEREUM_SOKOL},
        {ClaimType::Ethereum_poa, proto::CITEMTYPE_ETHEREUM_POA},
        {ClaimType::Pkt, proto::CITEMTYPE_PKT},
        {ClaimType::Tnpkt, proto::CITEMTYPE_TNPKT},
        {ClaimType::Regtest, proto::CITEMTYPE_REGTEST},
        {ClaimType::Bnb, proto::CITEMTYPE_BNB},
        {ClaimType::Sol, proto::CITEMTYPE_SOL},
        {ClaimType::Usdt, proto::CITEMTYPE_USDT},
        {ClaimType::Ada, proto::CITEMTYPE_ADA},
        {ClaimType::Dot, proto::CITEMTYPE_DOT},
        {ClaimType::Usdc, proto::CITEMTYPE_USDC},
        {ClaimType::Shib, proto::CITEMTYPE_SHIB},
        {ClaimType::Luna, proto::CITEMTYPE_LUNA},
        {ClaimType::Avax, proto::CITEMTYPE_AVAX},
        {ClaimType::Uni, proto::CITEMTYPE_UNI},
        {ClaimType::Link, proto::CITEMTYPE_LINK},
        {ClaimType::Wbtc, proto::CITEMTYPE_WBTC},
        {ClaimType::Busd, proto::CITEMTYPE_BUSD},
        {ClaimType::Matic, proto::CITEMTYPE_MATIC},
        {ClaimType::Algo, proto::CITEMTYPE_ALGO},
        {ClaimType::Vet, proto::CITEMTYPE_VET},
        {ClaimType::Axs, proto::CITEMTYPE_AXS},
        {ClaimType::Icp, proto::CITEMTYPE_ICP},
        {ClaimType::Cro, proto::CITEMTYPE_CRO},
        {ClaimType::Atom, proto::CITEMTYPE_ATOM},
        {ClaimType::Theta, proto::CITEMTYPE_THETA},
        {ClaimType::Fil, proto::CITEMTYPE_FIL},
        {ClaimType::Trx, proto::CITEMTYPE_TRX},
        {ClaimType::Ftt, proto::CITEMTYPE_FTT},
        {ClaimType::Etc, proto::CITEMTYPE_ETC},
        {ClaimType::Ftm, proto::CITEMTYPE_FTM},
        {ClaimType::Dai, proto::CITEMTYPE_DAI},
        {ClaimType::Btcb, proto::CITEMTYPE_BTCB},
        {ClaimType::Egld, proto::CITEMTYPE_EGLD},
        {ClaimType::Hbar, proto::CITEMTYPE_HBAR},
        {ClaimType::Xtz, proto::CITEMTYPE_XTZ},
        {ClaimType::Mana, proto::CITEMTYPE_MANA},
        {ClaimType::Near, proto::CITEMTYPE_NEAR},
        {ClaimType::Grt, proto::CITEMTYPE_GRT},
        {ClaimType::Cake, proto::CITEMTYPE_CAKE},
        {ClaimType::Eos, proto::CITEMTYPE_EOS},
        {ClaimType::Flow, proto::CITEMTYPE_FLOW},
        {ClaimType::Aave, proto::CITEMTYPE_AAVE},
        {ClaimType::Klay, proto::CITEMTYPE_KLAY},
        {ClaimType::Ksm, proto::CITEMTYPE_KSM},
        {ClaimType::Xec, proto::CITEMTYPE_XEC},
        {ClaimType::Miota, proto::CITEMTYPE_MIOTA},
        {ClaimType::Hnt, proto::CITEMTYPE_HNT},
        {ClaimType::Rune, proto::CITEMTYPE_RUNE},
        {ClaimType::Bsv, proto::CITEMTYPE_BSV},
        {ClaimType::Leo, proto::CITEMTYPE_LEO},
        {ClaimType::Neo, proto::CITEMTYPE_NEO},
        {ClaimType::One, proto::CITEMTYPE_ONE},
        {ClaimType::Qnt, proto::CITEMTYPE_QNT},
        {ClaimType::Ust, proto::CITEMTYPE_UST},
        {ClaimType::Mkr, proto::CITEMTYPE_MKR},
        {ClaimType::Enj, proto::CITEMTYPE_ENJ},
        {ClaimType::Chz, proto::CITEMTYPE_CHZ},
        {ClaimType::Ar, proto::CITEMTYPE_AR},
        {ClaimType::Stx, proto::CITEMTYPE_STX},
        {ClaimType::Btt, proto::CITEMTYPE_BTT},
        {ClaimType::Hot, proto::CITEMTYPE_HOT},
        {ClaimType::Sand, proto::CITEMTYPE_SAND},
        {ClaimType::Omg, proto::CITEMTYPE_OMG},
        {ClaimType::Celo, proto::CITEMTYPE_CELO},
        {ClaimType::Zec, proto::CITEMTYPE_ZEC},
        {ClaimType::Comp, proto::CITEMTYPE_COMP},
        {ClaimType::Tfuel, proto::CITEMTYPE_TFUEL},
        {ClaimType::Kda, proto::CITEMTYPE_KDA},
        {ClaimType::Lrc, proto::CITEMTYPE_LRC},
        {ClaimType::Qtum, proto::CITEMTYPE_QTUM},
        {ClaimType::Crv, proto::CITEMTYPE_CRV},
        {ClaimType::Ht, proto::CITEMTYPE_HT},
        {ClaimType::Nexo, proto::CITEMTYPE_NEXO},
        {ClaimType::Sushi, proto::CITEMTYPE_SUSHI},
        {ClaimType::Kcs, proto::CITEMTYPE_KCS},
        {ClaimType::Bat, proto::CITEMTYPE_BAT},
        {ClaimType::Okb, proto::CITEMTYPE_OKB},
        {ClaimType::Dcr, proto::CITEMTYPE_DCR},
        {ClaimType::Icx, proto::CITEMTYPE_ICX},
        {ClaimType::Rvn, proto::CITEMTYPE_RVN},
        {ClaimType::Scrt, proto::CITEMTYPE_SCRT},
        {ClaimType::Rev, proto::CITEMTYPE_REV},
        {ClaimType::Audio, proto::CITEMTYPE_AUDIO},
        {ClaimType::Zil, proto::CITEMTYPE_ZIL},
        {ClaimType::Tusd, proto::CITEMTYPE_TUSD},
        {ClaimType::Yfi, proto::CITEMTYPE_YFI},
        {ClaimType::Mina, proto::CITEMTYPE_MINA},
        {ClaimType::Perp, proto::CITEMTYPE_PERP},
        {ClaimType::Xdc, proto::CITEMTYPE_XDC},
        {ClaimType::Tel, proto::CITEMTYPE_TEL},
        {ClaimType::Snx, proto::CITEMTYPE_SNX},
        {ClaimType::Btg, proto::CITEMTYPE_BTG},
        {ClaimType::Afn, proto::CITEMTYPE_AFN},
        {ClaimType::All, proto::CITEMTYPE_ALL},
        {ClaimType::Amd, proto::CITEMTYPE_AMD},
        {ClaimType::Ang, proto::CITEMTYPE_ANG},
        {ClaimType::Aoa, proto::CITEMTYPE_AOA},
        {ClaimType::Ars, proto::CITEMTYPE_ARS},
        {ClaimType::Awg, proto::CITEMTYPE_AWG},
        {ClaimType::Azn, proto::CITEMTYPE_AZN},
        {ClaimType::Bam, proto::CITEMTYPE_BAM},
        {ClaimType::Bbd, proto::CITEMTYPE_BBD},
        {ClaimType::Bdt, proto::CITEMTYPE_BDT},
        {ClaimType::Bgn, proto::CITEMTYPE_BGN},
        {ClaimType::Bhd, proto::CITEMTYPE_BHD},
        {ClaimType::Bif, proto::CITEMTYPE_BIF},
        {ClaimType::Bmd, proto::CITEMTYPE_BMD},
        {ClaimType::Bnd, proto::CITEMTYPE_BND},
        {ClaimType::Bob, proto::CITEMTYPE_BOB},
        {ClaimType::Brl, proto::CITEMTYPE_BRL},
        {ClaimType::Bsd, proto::CITEMTYPE_BSD},
        {ClaimType::Btn, proto::CITEMTYPE_BTN},
        {ClaimType::Bwp, proto::CITEMTYPE_BWP},
        {ClaimType::Byn, proto::CITEMTYPE_BYN},
        {ClaimType::Bzd, proto::CITEMTYPE_BZD},
        {ClaimType::Cdf, proto::CITEMTYPE_CDF},
        {ClaimType::Clp, proto::CITEMTYPE_CLP},
        {ClaimType::Cop, proto::CITEMTYPE_COP},
        {ClaimType::Crc, proto::CITEMTYPE_CRC},
        {ClaimType::Cuc, proto::CITEMTYPE_CUC},
        {ClaimType::Cup, proto::CITEMTYPE_CUP},
        {ClaimType::Cve, proto::CITEMTYPE_CVE},
        {ClaimType::Czk, proto::CITEMTYPE_CZK},
        {ClaimType::Djf, proto::CITEMTYPE_DJF},
        {ClaimType::Dkk, proto::CITEMTYPE_DKK},
        {ClaimType::Dop, proto::CITEMTYPE_DOP},
        {ClaimType::Dzd, proto::CITEMTYPE_DZD},
        {ClaimType::Egp, proto::CITEMTYPE_EGP},
        {ClaimType::Ern, proto::CITEMTYPE_ERN},
        {ClaimType::Etb, proto::CITEMTYPE_ETB},
        {ClaimType::Fjd, proto::CITEMTYPE_FJD},
        {ClaimType::Fkp, proto::CITEMTYPE_FKP},
        {ClaimType::Gel, proto::CITEMTYPE_GEL},
        {ClaimType::Ggp, proto::CITEMTYPE_GGP},
        {ClaimType::Ghs, proto::CITEMTYPE_GHS},
        {ClaimType::Gip, proto::CITEMTYPE_GIP},
        {ClaimType::Gmd, proto::CITEMTYPE_GMD},
        {ClaimType::Gnf, proto::CITEMTYPE_GNF},
        {ClaimType::Gtq, proto::CITEMTYPE_GTQ},
        {ClaimType::Gyd, proto::CITEMTYPE_GYD},
        {ClaimType::Hnl, proto::CITEMTYPE_HNL},
        {ClaimType::Hrk, proto::CITEMTYPE_HRK},
        {ClaimType::Htg, proto::CITEMTYPE_HTG},
        {ClaimType::Idr, proto::CITEMTYPE_IDR},
        {ClaimType::Ils, proto::CITEMTYPE_ILS},
        {ClaimType::Imp, proto::CITEMTYPE_IMP},
        {ClaimType::Iqd, proto::CITEMTYPE_IQD},
        {ClaimType::Irr, proto::CITEMTYPE_IRR},
        {ClaimType::Isk, proto::CITEMTYPE_ISK},
        {ClaimType::Jep, proto::CITEMTYPE_JEP},
        {ClaimType::Jmd, proto::CITEMTYPE_JMD},
        {ClaimType::Jod, proto::CITEMTYPE_JOD},
        {ClaimType::Kes, proto::CITEMTYPE_KES},
        {ClaimType::Kgs, proto::CITEMTYPE_KGS},
        {ClaimType::Khr, proto::CITEMTYPE_KHR},
        {ClaimType::Kmf, proto::CITEMTYPE_KMF},
        {ClaimType::Kpw, proto::CITEMTYPE_KPW},
        {ClaimType::Krw, proto::CITEMTYPE_KRW},
        {ClaimType::Kwd, proto::CITEMTYPE_KWD},
        {ClaimType::Kyd, proto::CITEMTYPE_KYD},
        {ClaimType::Kzt, proto::CITEMTYPE_KZT},
        {ClaimType::Lak, proto::CITEMTYPE_LAK},
        {ClaimType::Lbp, proto::CITEMTYPE_LBP},
        {ClaimType::Lkr, proto::CITEMTYPE_LKR},
        {ClaimType::Lrd, proto::CITEMTYPE_LRD},
        {ClaimType::Lsl, proto::CITEMTYPE_LSL},
        {ClaimType::Lyd, proto::CITEMTYPE_LYD},
        {ClaimType::Mad, proto::CITEMTYPE_MAD},
        {ClaimType::Mdl, proto::CITEMTYPE_MDL},
        {ClaimType::Mga, proto::CITEMTYPE_MGA},
        {ClaimType::Mkd, proto::CITEMTYPE_MKD},
        {ClaimType::Mmk, proto::CITEMTYPE_MMK},
        {ClaimType::Mnt, proto::CITEMTYPE_MNT},
        {ClaimType::Mop, proto::CITEMTYPE_MOP},
        {ClaimType::Mru, proto::CITEMTYPE_MRU},
        {ClaimType::Mur, proto::CITEMTYPE_MUR},
        {ClaimType::Mvr, proto::CITEMTYPE_MVR},
        {ClaimType::Mwk, proto::CITEMTYPE_MWK},
        {ClaimType::Mzn, proto::CITEMTYPE_MZN},
        {ClaimType::Nad, proto::CITEMTYPE_NAD},
        {ClaimType::Ngn, proto::CITEMTYPE_NGN},
        {ClaimType::Nio, proto::CITEMTYPE_NIO},
        {ClaimType::Nok, proto::CITEMTYPE_NOK},
        {ClaimType::Npr, proto::CITEMTYPE_NPR},
        {ClaimType::Omr, proto::CITEMTYPE_OMR},
        {ClaimType::Pab, proto::CITEMTYPE_PAB},
        {ClaimType::Pen, proto::CITEMTYPE_PEN},
        {ClaimType::Pgk, proto::CITEMTYPE_PGK},
        {ClaimType::Pkr, proto::CITEMTYPE_PKR},
        {ClaimType::Pln, proto::CITEMTYPE_PLN},
        {ClaimType::Pyg, proto::CITEMTYPE_PYG},
        {ClaimType::Qar, proto::CITEMTYPE_QAR},
        {ClaimType::Ron, proto::CITEMTYPE_RON},
        {ClaimType::Rsd, proto::CITEMTYPE_RSD},
        {ClaimType::Rub, proto::CITEMTYPE_RUB},
        {ClaimType::Rwf, proto::CITEMTYPE_RWF},
        {ClaimType::Sar, proto::CITEMTYPE_SAR},
        {ClaimType::Sbd, proto::CITEMTYPE_SBD},
        {ClaimType::Scr, proto::CITEMTYPE_SCR},
        {ClaimType::Sdg, proto::CITEMTYPE_SDG},
        {ClaimType::Shp, proto::CITEMTYPE_SHP},
        {ClaimType::Sll, proto::CITEMTYPE_SLL},
        {ClaimType::Sos, proto::CITEMTYPE_SOS},
        {ClaimType::Spl, proto::CITEMTYPE_SPL},
        {ClaimType::Srd, proto::CITEMTYPE_SRD},
        {ClaimType::Stn, proto::CITEMTYPE_STN},
        {ClaimType::Svc, proto::CITEMTYPE_SVC},
        {ClaimType::Syp, proto::CITEMTYPE_SYP},
        {ClaimType::Szl, proto::CITEMTYPE_SZL},
        {ClaimType::Tjs, proto::CITEMTYPE_TJS},
        {ClaimType::Tmt, proto::CITEMTYPE_TMT},
        {ClaimType::Tnd, proto::CITEMTYPE_TND},
        {ClaimType::Top, proto::CITEMTYPE_TOP},
        {ClaimType::Try, proto::CITEMTYPE_TRY},
        {ClaimType::Ttd, proto::CITEMTYPE_TTD},
        {ClaimType::Tvd, proto::CITEMTYPE_TVD},
        {ClaimType::Twd, proto::CITEMTYPE_TWD},
        {ClaimType::Tzs, proto::CITEMTYPE_TZS},
        {ClaimType::Uah, proto::CITEMTYPE_UAH},
        {ClaimType::Ugx, proto::CITEMTYPE_UGX},
        {ClaimType::Uyu, proto::CITEMTYPE_UYU},
        {ClaimType::Uzs, proto::CITEMTYPE_UZS},
        {ClaimType::Vef, proto::CITEMTYPE_VEF},
        {ClaimType::Vnd, proto::CITEMTYPE_VND},
        {ClaimType::Vuv, proto::CITEMTYPE_VUV},
        {ClaimType::Wst, proto::CITEMTYPE_WST},
        {ClaimType::Xaf, proto::CITEMTYPE_XAF},
        {ClaimType::Xcd, proto::CITEMTYPE_XCD},
        {ClaimType::Xdr, proto::CITEMTYPE_XDR},
        {ClaimType::Xof, proto::CITEMTYPE_XOF},
        {ClaimType::Xpf, proto::CITEMTYPE_XPF},
        {ClaimType::Yer, proto::CITEMTYPE_YER},
        {ClaimType::Zmw, proto::CITEMTYPE_ZMW},
        {ClaimType::Zwd, proto::CITEMTYPE_ZWD},
        {ClaimType::Custom, proto::CITEMTYPE_CUSTOM},
    };

    return map;
}

auto identitytype_map() noexcept -> const NymTypeMap&
{
    static const auto map = NymTypeMap{
        {identity::Type::invalid, ClaimType::Error},
        {identity::Type::individual, ClaimType::Individual},
        {identity::Type::organization, ClaimType::Organization},
        {identity::Type::business, ClaimType::Business},
        {identity::Type::government, ClaimType::Government},
        {identity::Type::server, ClaimType::Server},
        {identity::Type::bot, ClaimType::Bot},
    };

    return map;
}

auto sectiontype_map() noexcept -> const SectionTypeMap&
{
    static const auto map = SectionTypeMap{
        {SectionType::Error, proto::CONTACTSECTION_ERROR},
        {SectionType::Scope, proto::CONTACTSECTION_SCOPE},
        {SectionType::Identifier, proto::CONTACTSECTION_IDENTIFIER},
        {SectionType::Address, proto::CONTACTSECTION_ADDRESS},
        {SectionType::Communication, proto::CONTACTSECTION_COMMUNICATION},
        {SectionType::Profile, proto::CONTACTSECTION_PROFILE},
        {SectionType::Relationship, proto::CONTACTSECTION_RELATIONSHIP},
        {SectionType::Descriptor, proto::CONTACTSECTION_DESCRIPTOR},
        {SectionType::Event, proto::CONTACTSECTION_EVENT},
        {SectionType::Contract, proto::CONTACTSECTION_CONTRACT},
        {SectionType::Procedure, proto::CONTACTSECTION_PROCEDURE},
    };

    return map;
}

auto unittype_map() noexcept -> const UnitTypeMap&
{
    static const auto map = UnitTypeMap{
        {UnitType::Error, ClaimType::Error},
        {UnitType::Btc, ClaimType::Btc},
        {UnitType::Eth, ClaimType::Eth},
        {UnitType::Xrp, ClaimType::Xrp},
        {UnitType::Ltc, ClaimType::Ltc},
        {UnitType::Dao, ClaimType::Dao},
        {UnitType::Xem, ClaimType::Xem},
        {UnitType::Dash, ClaimType::Dash},
        {UnitType::Maid, ClaimType::Maid},
        {UnitType::Lsk, ClaimType::Lsk},
        {UnitType::Doge, ClaimType::Doge},
        {UnitType::Dgd, ClaimType::Dgd},
        {UnitType::Xmr, ClaimType::Xmr},
        {UnitType::Waves, ClaimType::Waves},
        {UnitType::Nxt, ClaimType::Nxt},
        {UnitType::Sc, ClaimType::Sc},
        {UnitType::Steem, ClaimType::Steem},
        {UnitType::Amp, ClaimType::Amp},
        {UnitType::Xlm, ClaimType::Xlm},
        {UnitType::Fct, ClaimType::Fct},
        {UnitType::Bts, ClaimType::Bts},
        {UnitType::Usd, ClaimType::Usd},
        {UnitType::Eur, ClaimType::Eur},
        {UnitType::Gbp, ClaimType::Gbp},
        {UnitType::Inr, ClaimType::Inr},
        {UnitType::Aud, ClaimType::Aud},
        {UnitType::Cad, ClaimType::Cad},
        {UnitType::Sgd, ClaimType::Sgd},
        {UnitType::Chf, ClaimType::Chf},
        {UnitType::Myr, ClaimType::Myr},
        {UnitType::Jpy, ClaimType::Jpy},
        {UnitType::Cny, ClaimType::Cny},
        {UnitType::Nzd, ClaimType::Nzd},
        {UnitType::Thb, ClaimType::Thb},
        {UnitType::Huf, ClaimType::Huf},
        {UnitType::Aed, ClaimType::Aed},
        {UnitType::Hkd, ClaimType::Hkd},
        {UnitType::Mxn, ClaimType::Mxn},
        {UnitType::Zar, ClaimType::Zar},
        {UnitType::Php, ClaimType::Php},
        {UnitType::Sek, ClaimType::Sek},
        {UnitType::Tnbtc, ClaimType::Tnbtc},
        {UnitType::Tnxrp, ClaimType::Tnxrp},
        {UnitType::Tnltx, ClaimType::Tnltx},
        {UnitType::Tnxem, ClaimType::Tnxem},
        {UnitType::Tndash, ClaimType::Tndash},
        {UnitType::Tnmaid, ClaimType::Tnmaid},
        {UnitType::Tnlsk, ClaimType::Tnlsk},
        {UnitType::Tndoge, ClaimType::Tndoge},
        {UnitType::Tnxmr, ClaimType::Tnxmr},
        {UnitType::Tnwaves, ClaimType::Tnwaves},
        {UnitType::Tnnxt, ClaimType::Tnnxt},
        {UnitType::Tnsc, ClaimType::Tnsc},
        {UnitType::Tnsteem, ClaimType::Tnsteem},
        {UnitType::Bch, ClaimType::Bch},
        {UnitType::Tnbch, ClaimType::Tnbch},
        {UnitType::Pkt, ClaimType::Pkt},
        {UnitType::Tnpkt, ClaimType::Tnpkt},
        {UnitType::Ethereum_olympic, ClaimType::Ethereum_olympic},
        {UnitType::Ethereum_classic, ClaimType::Ethereum_classic},
        {UnitType::Ethereum_expanse, ClaimType::Ethereum_expanse},
        {UnitType::Ethereum_morden, ClaimType::Ethereum_morden},
        {UnitType::Ethereum_ropsten, ClaimType::Ethereum_ropsten},
        {UnitType::Ethereum_rinkeby, ClaimType::Ethereum_rinkeby},
        {UnitType::Ethereum_kovan, ClaimType::Ethereum_kovan},
        {UnitType::Ethereum_sokol, ClaimType::Ethereum_sokol},
        {UnitType::Ethereum_poa, ClaimType::Ethereum_poa},
        {UnitType::Regtest, ClaimType::Regtest},
        {UnitType::Unknown, ClaimType::Unknown},
        {UnitType::Bnb, ClaimType::Bnb},
        {UnitType::Sol, ClaimType::Sol},
        {UnitType::Usdt, ClaimType::Usdt},
        {UnitType::Ada, ClaimType::Ada},
        {UnitType::Dot, ClaimType::Dot},
        {UnitType::Usdc, ClaimType::Usdc},
        {UnitType::Shib, ClaimType::Shib},
        {UnitType::Luna, ClaimType::Luna},
        {UnitType::Avax, ClaimType::Avax},
        {UnitType::Uni, ClaimType::Uni},
        {UnitType::Link, ClaimType::Link},
        {UnitType::Wbtc, ClaimType::Wbtc},
        {UnitType::Busd, ClaimType::Busd},
        {UnitType::MatiC, ClaimType::Matic},
        {UnitType::Algo, ClaimType::Algo},
        {UnitType::Vet, ClaimType::Vet},
        {UnitType::Axs, ClaimType::Axs},
        {UnitType::Icp, ClaimType::Icp},
        {UnitType::Cro, ClaimType::Cro},
        {UnitType::Atom, ClaimType::Atom},
        {UnitType::Theta, ClaimType::Theta},
        {UnitType::Fil, ClaimType::Fil},
        {UnitType::Trx, ClaimType::Trx},
        {UnitType::Ftt, ClaimType::Ftt},
        {UnitType::Etc, ClaimType::Etc},
        {UnitType::Ftm, ClaimType::Ftm},
        {UnitType::Dai, ClaimType::Dai},
        {UnitType::Btcb, ClaimType::Btcb},
        {UnitType::Egld, ClaimType::Egld},
        {UnitType::Hbar, ClaimType::Hbar},
        {UnitType::Xtz, ClaimType::Xtz},
        {UnitType::Mana, ClaimType::Mana},
        {UnitType::Near, ClaimType::Near},
        {UnitType::Grt, ClaimType::Grt},
        {UnitType::Cake, ClaimType::Cake},
        {UnitType::Eos, ClaimType::Eos},
        {UnitType::Flow, ClaimType::Flow},
        {UnitType::Aave, ClaimType::Aave},
        {UnitType::Klay, ClaimType::Klay},
        {UnitType::Ksm, ClaimType::Ksm},
        {UnitType::Xec, ClaimType::Xec},
        {UnitType::Miota, ClaimType::Miota},
        {UnitType::Hnt, ClaimType::Hnt},
        {UnitType::Rune, ClaimType::Rune},
        {UnitType::Bsv, ClaimType::Bsv},
        {UnitType::Leo, ClaimType::Leo},
        {UnitType::Neo, ClaimType::Neo},
        {UnitType::One, ClaimType::One},
        {UnitType::Qnt, ClaimType::Qnt},
        {UnitType::Ust, ClaimType::Ust},
        {UnitType::Mkr, ClaimType::Mkr},
        {UnitType::Enj, ClaimType::Enj},
        {UnitType::Chz, ClaimType::Chz},
        {UnitType::Ar, ClaimType::Ar},
        {UnitType::Stx, ClaimType::Stx},
        {UnitType::Btt, ClaimType::Btt},
        {UnitType::Hot, ClaimType::Hot},
        {UnitType::Sand, ClaimType::Sand},
        {UnitType::Omg, ClaimType::Omg},
        {UnitType::Celo, ClaimType::Celo},
        {UnitType::Zec, ClaimType::Zec},
        {UnitType::Comp, ClaimType::Comp},
        {UnitType::Tfuel, ClaimType::Tfuel},
        {UnitType::Kda, ClaimType::Kda},
        {UnitType::Lrc, ClaimType::Lrc},
        {UnitType::Qtum, ClaimType::Qtum},
        {UnitType::Crv, ClaimType::Crv},
        {UnitType::Ht, ClaimType::Ht},
        {UnitType::Nexo, ClaimType::Nexo},
        {UnitType::Sushi, ClaimType::Sushi},
        {UnitType::Kcs, ClaimType::Kcs},
        {UnitType::Bat, ClaimType::Bat},
        {UnitType::Okb, ClaimType::Okb},
        {UnitType::Dcr, ClaimType::Dcr},
        {UnitType::Icx, ClaimType::Icx},
        {UnitType::Rvn, ClaimType::Rvn},
        {UnitType::Scrt, ClaimType::Scrt},
        {UnitType::Rev, ClaimType::Rev},
        {UnitType::Audio, ClaimType::Audio},
        {UnitType::Zil, ClaimType::Zil},
        {UnitType::Tusd, ClaimType::Tusd},
        {UnitType::Yfi, ClaimType::Yfi},
        {UnitType::Mina, ClaimType::Mina},
        {UnitType::Perp, ClaimType::Perp},
        {UnitType::Xdc, ClaimType::Xdc},
        {UnitType::Tel, ClaimType::Tel},
        {UnitType::Snx, ClaimType::Snx},
        {UnitType::Btg, ClaimType::Btg},
        {UnitType::Afn, ClaimType::Afn},
        {UnitType::All, ClaimType::All},
        {UnitType::Amd, ClaimType::Amd},
        {UnitType::Ang, ClaimType::Ang},
        {UnitType::Aoa, ClaimType::Aoa},
        {UnitType::Ars, ClaimType::Ars},
        {UnitType::Awg, ClaimType::Awg},
        {UnitType::Azn, ClaimType::Azn},
        {UnitType::Bam, ClaimType::Bam},
        {UnitType::Bbd, ClaimType::Bbd},
        {UnitType::Bdt, ClaimType::Bdt},
        {UnitType::Bgn, ClaimType::Bgn},
        {UnitType::Bhd, ClaimType::Bhd},
        {UnitType::Bif, ClaimType::Bif},
        {UnitType::Bmd, ClaimType::Bmd},
        {UnitType::Bnd, ClaimType::Bnd},
        {UnitType::Bob, ClaimType::Bob},
        {UnitType::Brl, ClaimType::Brl},
        {UnitType::Bsd, ClaimType::Bsd},
        {UnitType::Btn, ClaimType::Btn},
        {UnitType::Bwp, ClaimType::Bwp},
        {UnitType::Byn, ClaimType::Byn},
        {UnitType::Bzd, ClaimType::Bzd},
        {UnitType::Cdf, ClaimType::Cdf},
        {UnitType::Clp, ClaimType::Clp},
        {UnitType::Cop, ClaimType::Cop},
        {UnitType::Crc, ClaimType::Crc},
        {UnitType::Cuc, ClaimType::Cuc},
        {UnitType::Cup, ClaimType::Cup},
        {UnitType::Cve, ClaimType::Cve},
        {UnitType::Czk, ClaimType::Czk},
        {UnitType::Djf, ClaimType::Djf},
        {UnitType::Dkk, ClaimType::Dkk},
        {UnitType::Dop, ClaimType::Dop},
        {UnitType::Dzd, ClaimType::Dzd},
        {UnitType::Egp, ClaimType::Egp},
        {UnitType::Ern, ClaimType::Ern},
        {UnitType::Etb, ClaimType::Etb},
        {UnitType::Fjd, ClaimType::Fjd},
        {UnitType::Fkp, ClaimType::Fkp},
        {UnitType::Gel, ClaimType::Gel},
        {UnitType::Ggp, ClaimType::Ggp},
        {UnitType::Ghs, ClaimType::Ghs},
        {UnitType::Gip, ClaimType::Gip},
        {UnitType::Gmd, ClaimType::Gmd},
        {UnitType::Gnf, ClaimType::Gnf},
        {UnitType::Gtq, ClaimType::Gtq},
        {UnitType::Gyd, ClaimType::Gyd},
        {UnitType::Hnl, ClaimType::Hnl},
        {UnitType::Hrk, ClaimType::Hrk},
        {UnitType::Htg, ClaimType::Htg},
        {UnitType::Idr, ClaimType::Idr},
        {UnitType::Ils, ClaimType::Ils},
        {UnitType::Imp, ClaimType::Imp},
        {UnitType::Iqd, ClaimType::Iqd},
        {UnitType::Irr, ClaimType::Irr},
        {UnitType::Isk, ClaimType::Isk},
        {UnitType::Jep, ClaimType::Jep},
        {UnitType::Jmd, ClaimType::Jmd},
        {UnitType::Jod, ClaimType::Jod},
        {UnitType::Kes, ClaimType::Kes},
        {UnitType::Kgs, ClaimType::Kgs},
        {UnitType::Khr, ClaimType::Khr},
        {UnitType::Kmf, ClaimType::Kmf},
        {UnitType::Kpw, ClaimType::Kpw},
        {UnitType::Krw, ClaimType::Krw},
        {UnitType::Kwd, ClaimType::Kwd},
        {UnitType::Kyd, ClaimType::Kyd},
        {UnitType::Kzt, ClaimType::Kzt},
        {UnitType::Lak, ClaimType::Lak},
        {UnitType::Lbp, ClaimType::Lbp},
        {UnitType::Lkr, ClaimType::Lkr},
        {UnitType::Lrd, ClaimType::Lrd},
        {UnitType::Lsl, ClaimType::Lsl},
        {UnitType::Lyd, ClaimType::Lyd},
        {UnitType::Mad, ClaimType::Mad},
        {UnitType::Mdl, ClaimType::Mdl},
        {UnitType::Mga, ClaimType::Mga},
        {UnitType::Mkd, ClaimType::Mkd},
        {UnitType::Mmk, ClaimType::Mmk},
        {UnitType::Mnt, ClaimType::Mnt},
        {UnitType::Mop, ClaimType::Mop},
        {UnitType::Mru, ClaimType::Mru},
        {UnitType::Mur, ClaimType::Mur},
        {UnitType::Mvr, ClaimType::Mvr},
        {UnitType::Mwk, ClaimType::Mwk},
        {UnitType::Mzn, ClaimType::Mzn},
        {UnitType::Nad, ClaimType::Nad},
        {UnitType::Ngn, ClaimType::Ngn},
        {UnitType::Nio, ClaimType::Nio},
        {UnitType::Nok, ClaimType::Nok},
        {UnitType::Npr, ClaimType::Npr},
        {UnitType::Omr, ClaimType::Omr},
        {UnitType::Pab, ClaimType::Pab},
        {UnitType::Pen, ClaimType::Pen},
        {UnitType::Pgk, ClaimType::Pgk},
        {UnitType::Pkr, ClaimType::Pkr},
        {UnitType::Pln, ClaimType::Pln},
        {UnitType::Pyg, ClaimType::Pyg},
        {UnitType::Qar, ClaimType::Qar},
        {UnitType::Ron, ClaimType::Ron},
        {UnitType::Rsd, ClaimType::Rsd},
        {UnitType::Rub, ClaimType::Rub},
        {UnitType::Rwf, ClaimType::Rwf},
        {UnitType::Sar, ClaimType::Sar},
        {UnitType::Sbd, ClaimType::Sbd},
        {UnitType::Scr, ClaimType::Scr},
        {UnitType::Sdg, ClaimType::Sdg},
        {UnitType::Shp, ClaimType::Shp},
        {UnitType::Sll, ClaimType::Sll},
        {UnitType::Sos, ClaimType::Sos},
        {UnitType::Spl, ClaimType::Spl},
        {UnitType::Srd, ClaimType::Srd},
        {UnitType::Stn, ClaimType::Stn},
        {UnitType::Svc, ClaimType::Svc},
        {UnitType::Syp, ClaimType::Syp},
        {UnitType::Szl, ClaimType::Szl},
        {UnitType::Tjs, ClaimType::Tjs},
        {UnitType::Tmt, ClaimType::Tmt},
        {UnitType::Tnd, ClaimType::Tnd},
        {UnitType::Top, ClaimType::Top},
        {UnitType::Try, ClaimType::Try},
        {UnitType::Ttd, ClaimType::Ttd},
        {UnitType::Tvd, ClaimType::Tvd},
        {UnitType::Twd, ClaimType::Twd},
        {UnitType::Tzs, ClaimType::Tzs},
        {UnitType::Uah, ClaimType::Uah},
        {UnitType::Ugx, ClaimType::Ugx},
        {UnitType::Uyu, ClaimType::Uyu},
        {UnitType::Uzs, ClaimType::Uzs},
        {UnitType::Vef, ClaimType::Vef},
        {UnitType::Vnd, ClaimType::Vnd},
        {UnitType::Vuv, ClaimType::Vuv},
        {UnitType::Wst, ClaimType::Wst},
        {UnitType::Xaf, ClaimType::Xaf},
        {UnitType::Xcd, ClaimType::Xcd},
        {UnitType::Xdr, ClaimType::Xdr},
        {UnitType::Xof, ClaimType::Xof},
        {UnitType::Xpf, ClaimType::Xpf},
        {UnitType::Yer, ClaimType::Yer},
        {UnitType::Zmw, ClaimType::Zmw},
        {UnitType::Zwd, ClaimType::Zwd},
        {UnitType::Custom, ClaimType::Custom},
    };

    return map;
}
}  // namespace opentxs::identity::wot::claim

namespace opentxs
{
auto ClaimToNym(const identity::wot::claim::ClaimType in) noexcept
    -> identity::Type
{
    static const auto map = reverse_arbitrary_map<
        identity::Type,
        identity::wot::claim::ClaimType,
        identity::wot::claim::NymTypeReverseMap>(
        identity::wot::claim::identitytype_map());

    try {

        return map.at(in);
    } catch (...) {

        return identity::Type::invalid;
    }
}

auto ClaimToUnit(const identity::wot::claim::ClaimType in) noexcept -> UnitType
{
    static const auto map = reverse_arbitrary_map<
        UnitType,
        identity::wot::claim::ClaimType,
        identity::wot::claim::UnitTypeReverseMap>(
        identity::wot::claim::unittype_map());

    try {
        return map.at(in);
    } catch (...) {
        return UnitType::Error;
    }
}

auto NymToClaim(const identity::Type in) noexcept
    -> identity::wot::claim::ClaimType
{
    try {
        return identity::wot::claim::identitytype_map().at(in);
    } catch (...) {
        return identity::wot::claim::ClaimType::Error;
    }
}

auto translate(const identity::wot::claim::Attribute in) noexcept
    -> proto::ContactItemAttribute
{
    try {
        return identity::wot::claim::attribute_map().at(in);
    } catch (...) {
        return proto::CITEMATTR_ERROR;
    }
}

auto translate(const identity::wot::claim::ClaimType in) noexcept
    -> proto::ContactItemType
{
    try {
        return identity::wot::claim::claimtype_map().at(in);
    } catch (...) {
        return proto::CITEMTYPE_ERROR;
    }
}

auto translate(const identity::wot::claim::SectionType in) noexcept
    -> proto::ContactSectionName
{
    try {
        return identity::wot::claim::sectiontype_map().at(in);
    } catch (...) {
        return proto::CONTACTSECTION_ERROR;
    }
}

auto translate(const proto::ContactItemAttribute in) noexcept
    -> identity::wot::claim::Attribute
{
    static const auto map = reverse_arbitrary_map<
        identity::wot::claim::Attribute,
        proto::ContactItemAttribute,
        identity::wot::claim::AttributeReverseMap>(
        identity::wot::claim::attribute_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::wot::claim::Attribute::Error;
    }
}

auto translate(const proto::ContactItemType in) noexcept
    -> identity::wot::claim::ClaimType
{
    static const auto map = reverse_arbitrary_map<
        identity::wot::claim::ClaimType,
        proto::ContactItemType,
        identity::wot::claim::ClaimTypeReverseMap>(
        identity::wot::claim::claimtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::wot::claim::ClaimType::Error;
    }
}

auto translate(const proto::ContactSectionName in) noexcept
    -> identity::wot::claim::SectionType
{
    static const auto map = reverse_arbitrary_map<
        identity::wot::claim::SectionType,
        proto::ContactSectionName,
        identity::wot::claim::SectionTypeReverseMap>(
        identity::wot::claim::sectiontype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::wot::claim::SectionType::Error;
    }
}

auto UnitToClaim(const UnitType in) noexcept -> identity::wot::claim::ClaimType
{
    try {
        return identity::wot::claim::unittype_map().at(in);
    } catch (...) {
        return identity::wot::claim::ClaimType::Error;
    }
}
}  // namespace opentxs
