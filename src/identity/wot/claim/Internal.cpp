// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/identity/wot/claim/Types.hpp"  // IWYU pragma: associated

#include <ContactEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"          // IWYU pragma: keep
#include "opentxs/identity/IdentityType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/ClaimType.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"

namespace opentxs::identity::wot::claim
{
using AttributeMap =
    frozen::unordered_map<claim::Attribute, proto::ContactItemAttribute, 4>;
using AttributeReverseMap =
    frozen::unordered_map<proto::ContactItemAttribute, claim::Attribute, 4>;
using ClaimTypeMap =
    frozen::unordered_map<claim::ClaimType, proto::ContactItemType, 429>;
using ClaimTypeReverseMap =
    frozen::unordered_map<proto::ContactItemType, claim::ClaimType, 428>;
using SectionTypeMap =
    frozen::unordered_map<claim::SectionType, proto::ContactSectionName, 11>;
using SectionTypeReverseMap =
    frozen::unordered_map<proto::ContactSectionName, claim::SectionType, 11>;
using UnitTypeMap = frozen::unordered_map<UnitType, claim::ClaimType, 305>;
using UnitTypeReverseMap =
    frozen::unordered_map<claim::ClaimType, UnitType, 304>;
using NymTypeMap = frozen::unordered_map<identity::Type, claim::ClaimType, 7>;
using NymTypeReverseMap =
    frozen::unordered_map<claim::ClaimType, identity::Type, 7>;

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
    using enum claim::Attribute;
    using enum proto::ContactItemAttribute;
    static constexpr auto map = AttributeMap{
        {Error, CITEMATTR_ERROR},
        {Active, CITEMATTR_ACTIVE},
        {Primary, CITEMATTR_PRIMARY},
        {Local, CITEMATTR_LOCAL},
    };

    return map;
}

auto claimtype_map() noexcept -> const ClaimTypeMap&
{
    using enum ClaimType;
    using enum proto::ContactItemType;
    static constexpr auto map = ClaimTypeMap{
        {Error, CITEMTYPE_ERROR},
        {Individual, CITEMTYPE_INDIVIDUAL},
        {Organization, CITEMTYPE_ORGANIZATION},
        {Business, CITEMTYPE_BUSINESS},
        {Government, CITEMTYPE_GOVERNMENT},
        {Server, CITEMTYPE_SERVER},
        {Prefix, CITEMTYPE_PREFIX},
        {Forename, CITEMTYPE_FORENAME},
        {Middlename, CITEMTYPE_MIDDLENAME},
        {Surname, CITEMTYPE_SURNAME},
        {Pedigree, CITEMTYPE_PEDIGREE},
        {Suffix, CITEMTYPE_SUFFIX},
        {Nickname, CITEMTYPE_NICKNAME},
        {Commonname, CITEMTYPE_COMMONNAME},
        {Passport, CITEMTYPE_PASSPORT},
        {National, CITEMTYPE_NATIONAL},
        {Provincial, CITEMTYPE_PROVINCIAL},
        {Military, CITEMTYPE_MILITARY},
        {Pgp, CITEMTYPE_PGP},
        {Otr, CITEMTYPE_OTR},
        {Ssl, CITEMTYPE_SSL},
        {Physical, CITEMTYPE_PHYSICAL},
        {Official, CITEMTYPE_OFFICIAL},
        {Birthplace, CITEMTYPE_BIRTHPLACE},
        {Home, CITEMTYPE_HOME},
        {Website, CITEMTYPE_WEBSITE},
        {Opentxs, CITEMTYPE_OPENTXS},
        {Phone, CITEMTYPE_PHONE},
        {Email, CITEMTYPE_EMAIL},
        {Skype, CITEMTYPE_SKYPE},
        {Wire, CITEMTYPE_WIRE},
        {Qq, CITEMTYPE_QQ},
        {Bitmessage, CITEMTYPE_BITMESSAGE},
        {Whatsapp, CITEMTYPE_WHATSAPP},
        {Telegram, CITEMTYPE_TELEGRAM},
        {Kik, CITEMTYPE_KIK},
        {Bbm, CITEMTYPE_BBM},
        {Wechat, CITEMTYPE_WECHAT},
        {Kakaotalk, CITEMTYPE_KAKAOTALK},
        {Facebook, CITEMTYPE_FACEBOOK},
        {Google, CITEMTYPE_GOOGLE},
        {Linkedin, CITEMTYPE_LINKEDIN},
        {Vk, CITEMTYPE_VK},
        {Aboutme, CITEMTYPE_ABOUTME},
        {Onename, CITEMTYPE_ONENAME},
        {Twitter, CITEMTYPE_TWITTER},
        {Medium, CITEMTYPE_MEDIUM},
        {Tumblr, CITEMTYPE_TUMBLR},
        {Yahoo, CITEMTYPE_YAHOO},
        {Myspace, CITEMTYPE_MYSPACE},
        {Meetup, CITEMTYPE_MEETUP},
        {Reddit, CITEMTYPE_REDDIT},
        {Hackernews, CITEMTYPE_HACKERNEWS},
        {Wikipedia, CITEMTYPE_WIKIPEDIA},
        {Angellist, CITEMTYPE_ANGELLIST},
        {Github, CITEMTYPE_GITHUB},
        {Bitbucket, CITEMTYPE_BITBUCKET},
        {Youtube, CITEMTYPE_YOUTUBE},
        {Vimeo, CITEMTYPE_VIMEO},
        {Twitch, CITEMTYPE_TWITCH},
        {Snapchat, CITEMTYPE_SNAPCHAT},
        {Vine, CITEMTYPE_VINE},
        {Instagram, CITEMTYPE_INSTAGRAM},
        {Pinterest, CITEMTYPE_PINTEREST},
        {Imgur, CITEMTYPE_IMGUR},
        {Flickr, CITEMTYPE_FLICKR},
        {Dribble, CITEMTYPE_DRIBBLE},
        {Behance, CITEMTYPE_BEHANCE},
        {Deviantart, CITEMTYPE_DEVIANTART},
        {Spotify, CITEMTYPE_SPOTIFY},
        {Itunes, CITEMTYPE_ITUNES},
        {Soundcloud, CITEMTYPE_SOUNDCLOUD},
        {Askfm, CITEMTYPE_ASKFM},
        {Ebay, CITEMTYPE_EBAY},
        {Etsy, CITEMTYPE_ETSY},
        {Openbazaar, CITEMTYPE_OPENBAZAAR},
        {Xboxlive, CITEMTYPE_XBOXLIVE},
        {Playstation, CITEMTYPE_PLAYSTATION},
        {Secondlife, CITEMTYPE_SECONDLIFE},
        {Warcraft, CITEMTYPE_WARCRAFT},
        {Alias, CITEMTYPE_ALIAS},
        {Acquaintance, CITEMTYPE_ACQUAINTANCE},
        {Friend, CITEMTYPE_FRIEND},
        {Spouse, CITEMTYPE_SPOUSE},
        {Sibling, CITEMTYPE_SIBLING},
        {Member, CITEMTYPE_MEMBER},
        {Colleague, CITEMTYPE_COLLEAGUE},
        {Parent, CITEMTYPE_PARENT},
        {Child, CITEMTYPE_CHILD},
        {Employer, CITEMTYPE_EMPLOYER},
        {Employee, CITEMTYPE_EMPLOYEE},
        {Citizen, CITEMTYPE_CITIZEN},
        {Photo, CITEMTYPE_PHOTO},
        {Gender, CITEMTYPE_GENDER},
        {Height, CITEMTYPE_HEIGHT},
        {Weight, CITEMTYPE_WEIGHT},
        {Hair, CITEMTYPE_HAIR},
        {Eye, CITEMTYPE_EYE},
        {Skin, CITEMTYPE_SKIN},
        {Ethnicity, CITEMTYPE_ETHNICITY},
        {Language, CITEMTYPE_LANGUAGE},
        {Degree, CITEMTYPE_DEGREE},
        {Certification, CITEMTYPE_CERTIFICATION},
        {Title, CITEMTYPE_TITLE},
        {Skill, CITEMTYPE_SKILL},
        {Award, CITEMTYPE_AWARD},
        {Likes, CITEMTYPE_LIKES},
        {Sexual, CITEMTYPE_SEXUAL},
        {Political, CITEMTYPE_POLITICAL},
        {Religious, CITEMTYPE_RELIGIOUS},
        {Birth, CITEMTYPE_BIRTH},
        {Secondarygraduation, CITEMTYPE_SECONDARYGRADUATION},
        {Universitygraduation, CITEMTYPE_UNIVERSITYGRADUATION},
        {Wedding, CITEMTYPE_WEDDING},
        {Accomplishment, CITEMTYPE_ACCOMPLISHMENT},
        {Btc, CITEMTYPE_BTC},
        {Eth, CITEMTYPE_ETH},
        {Xrp, CITEMTYPE_XRP},
        {Ltc, CITEMTYPE_LTC},
        {Dao, CITEMTYPE_DAO},
        {Xem, CITEMTYPE_XEM},
        {Dash, CITEMTYPE_DASH},
        {Maid, CITEMTYPE_MAID},
        {Lsk, CITEMTYPE_LSK},
        {Doge, CITEMTYPE_DOGE},
        {Dgd, CITEMTYPE_DGD},
        {Xmr, CITEMTYPE_XMR},
        {Waves, CITEMTYPE_WAVES},
        {Nxt, CITEMTYPE_NXT},
        {Sc, CITEMTYPE_SC},
        {Steem, CITEMTYPE_STEEM},
        {Amp, CITEMTYPE_AMP},
        {Xlm, CITEMTYPE_XLM},
        {Fct, CITEMTYPE_FCT},
        {Bts, CITEMTYPE_BTS},
        {Usd, CITEMTYPE_USD},
        {Eur, CITEMTYPE_EUR},
        {Gbp, CITEMTYPE_GBP},
        {Inr, CITEMTYPE_INR},
        {Aud, CITEMTYPE_AUD},
        {Cad, CITEMTYPE_CAD},
        {Sgd, CITEMTYPE_SGD},
        {Chf, CITEMTYPE_CHF},
        {Myr, CITEMTYPE_MYR},
        {Jpy, CITEMTYPE_JPY},
        {Cny, CITEMTYPE_CNY},
        {Nzd, CITEMTYPE_NZD},
        {Thb, CITEMTYPE_THB},
        {Huf, CITEMTYPE_HUF},
        {Aed, CITEMTYPE_AED},
        {Hkd, CITEMTYPE_HKD},
        {Mxn, CITEMTYPE_MXN},
        {Zar, CITEMTYPE_ZAR},
        {Php, CITEMTYPE_PHP},
        {Sek, CITEMTYPE_SEK},
        {Tnbtc, CITEMTYPE_TNBTC},
        {Tnxrp, CITEMTYPE_TNXRP},
        {Tnltx, CITEMTYPE_TNLTC},
        {Tnxem, CITEMTYPE_TNXEM},
        {Tndash, CITEMTYPE_TNDASH},
        {Tnmaid, CITEMTYPE_TNMAID},
        {Tnlsk, CITEMTYPE_TNLSK},
        {Tndoge, CITEMTYPE_TNDOGE},
        {Tnxmr, CITEMTYPE_TNXMR},
        {Tnwaves, CITEMTYPE_TNWAVES},
        {Tnnxt, CITEMTYPE_TNNXT},
        {Tnsc, CITEMTYPE_TNSC},
        {Tnsteem, CITEMTYPE_TNSTEEM},
        {Philosophy, CITEMTYPE_PHILOSOPHY},
        {Met, CITEMTYPE_MET},
        {Fan, CITEMTYPE_FAN},
        {Supervisor, CITEMTYPE_SUPERVISOR},
        {Subordinate, CITEMTYPE_SUBORDINATE},
        {Contact, CITEMTYPE_CONTACT},
        {Refreshed, CITEMTYPE_REFRESHED},
        {Bot, CITEMTYPE_BOT},
        {Bch, CITEMTYPE_BCH},
        {Tnbch, CITEMTYPE_TNBCH},
        {Owner, CITEMTYPE_OWNER},
        {Property, CITEMTYPE_PROPERTY},
        {Unknown, CITEMTYPE_UNKNOWN},
        {Ethereum_olympic, CITEMTYPE_ETHEREUM_OLYMPIC},
        {Ethereum_classic, CITEMTYPE_ETHEREUM_CLASSIC},
        {Ethereum_expanse, CITEMTYPE_ETHEREUM_EXPANSE},
        {Ethereum_morden, CITEMTYPE_ETHEREUM_MORDEN},
        {Ethereum_ropsten, CITEMTYPE_ETHEREUM_ROPSTEN},
        {Ethereum_rinkeby, CITEMTYPE_ETHEREUM_RINKEBY},
        {Ethereum_kovan, CITEMTYPE_ETHEREUM_KOVAN},
        {Ethereum_sokol, CITEMTYPE_ETHEREUM_SOKOL},
        {Ethereum_poa, CITEMTYPE_ETHEREUM_POA},
        {Pkt, CITEMTYPE_PKT},
        {Tnpkt, CITEMTYPE_TNPKT},
        {Regtest, CITEMTYPE_REGTEST},
        {Bnb, CITEMTYPE_BNB},
        {Sol, CITEMTYPE_SOL},
        {Usdt, CITEMTYPE_USDT},
        {Ada, CITEMTYPE_ADA},
        {Dot, CITEMTYPE_DOT},
        {Usdc, CITEMTYPE_USDC},
        {Shib, CITEMTYPE_SHIB},
        {Luna, CITEMTYPE_LUNA},
        {Avax, CITEMTYPE_AVAX},
        {Uni, CITEMTYPE_UNI},
        {Link, CITEMTYPE_LINK},
        {Wbtc, CITEMTYPE_WBTC},
        {Busd, CITEMTYPE_BUSD},
        {Matic, CITEMTYPE_MATIC},
        {Algo, CITEMTYPE_ALGO},
        {Vet, CITEMTYPE_VET},
        {Axs, CITEMTYPE_AXS},
        {Icp, CITEMTYPE_ICP},
        {Cro, CITEMTYPE_CRO},
        {Atom, CITEMTYPE_ATOM},
        {Theta, CITEMTYPE_THETA},
        {Fil, CITEMTYPE_FIL},
        {Trx, CITEMTYPE_TRX},
        {Ftt, CITEMTYPE_FTT},
        {Etc, CITEMTYPE_ETC},
        {Ftm, CITEMTYPE_FTM},
        {Dai, CITEMTYPE_DAI},
        {Btcb, CITEMTYPE_BTCB},
        {Egld, CITEMTYPE_EGLD},
        {Hbar, CITEMTYPE_HBAR},
        {Xtz, CITEMTYPE_XTZ},
        {Mana, CITEMTYPE_MANA},
        {Near, CITEMTYPE_NEAR},
        {Grt, CITEMTYPE_GRT},
        {Cake, CITEMTYPE_CAKE},
        {Eos, CITEMTYPE_EOS},
        {Flow, CITEMTYPE_FLOW},
        {Aave, CITEMTYPE_AAVE},
        {Klay, CITEMTYPE_KLAY},
        {Ksm, CITEMTYPE_KSM},
        {Xec, CITEMTYPE_XEC},
        {Miota, CITEMTYPE_MIOTA},
        {Hnt, CITEMTYPE_HNT},
        {Rune, CITEMTYPE_RUNE},
        {Bsv, CITEMTYPE_BSV},
        {Leo, CITEMTYPE_LEO},
        {Neo, CITEMTYPE_NEO},
        {One, CITEMTYPE_ONE},
        {Qnt, CITEMTYPE_QNT},
        {Ust, CITEMTYPE_UST},
        {Mkr, CITEMTYPE_MKR},
        {Enj, CITEMTYPE_ENJ},
        {Chz, CITEMTYPE_CHZ},
        {Ar, CITEMTYPE_AR},
        {Stx, CITEMTYPE_STX},
        {Btt, CITEMTYPE_BTT},
        {Hot, CITEMTYPE_HOT},
        {Sand, CITEMTYPE_SAND},
        {Omg, CITEMTYPE_OMG},
        {Celo, CITEMTYPE_CELO},
        {Zec, CITEMTYPE_ZEC},
        {Comp, CITEMTYPE_COMP},
        {Tfuel, CITEMTYPE_TFUEL},
        {Kda, CITEMTYPE_KDA},
        {Lrc, CITEMTYPE_LRC},
        {Qtum, CITEMTYPE_QTUM},
        {Crv, CITEMTYPE_CRV},
        {Ht, CITEMTYPE_HT},
        {Nexo, CITEMTYPE_NEXO},
        {Sushi, CITEMTYPE_SUSHI},
        {Kcs, CITEMTYPE_KCS},
        {Bat, CITEMTYPE_BAT},
        {Okb, CITEMTYPE_OKB},
        {Dcr, CITEMTYPE_DCR},
        {Icx, CITEMTYPE_ICX},
        {Rvn, CITEMTYPE_RVN},
        {Scrt, CITEMTYPE_SCRT},
        {Rev, CITEMTYPE_REV},
        {Audio, CITEMTYPE_AUDIO},
        {Zil, CITEMTYPE_ZIL},
        {Tusd, CITEMTYPE_TUSD},
        {Yfi, CITEMTYPE_YFI},
        {Mina, CITEMTYPE_MINA},
        {Perp, CITEMTYPE_PERP},
        {Xdc, CITEMTYPE_XDC},
        {Tel, CITEMTYPE_TEL},
        {Snx, CITEMTYPE_SNX},
        {Btg, CITEMTYPE_BTG},
        {Afn, CITEMTYPE_AFN},
        {All, CITEMTYPE_ALL},
        {Amd, CITEMTYPE_AMD},
        {Ang, CITEMTYPE_ANG},
        {Aoa, CITEMTYPE_AOA},
        {Ars, CITEMTYPE_ARS},
        {Awg, CITEMTYPE_AWG},
        {Azn, CITEMTYPE_AZN},
        {Bam, CITEMTYPE_BAM},
        {Bbd, CITEMTYPE_BBD},
        {Bdt, CITEMTYPE_BDT},
        {Bgn, CITEMTYPE_BGN},
        {Bhd, CITEMTYPE_BHD},
        {Bif, CITEMTYPE_BIF},
        {Bmd, CITEMTYPE_BMD},
        {Bnd, CITEMTYPE_BND},
        {Bob, CITEMTYPE_BOB},
        {Brl, CITEMTYPE_BRL},
        {Bsd, CITEMTYPE_BSD},
        {Btn, CITEMTYPE_BTN},
        {Bwp, CITEMTYPE_BWP},
        {Byn, CITEMTYPE_BYN},
        {Bzd, CITEMTYPE_BZD},
        {Cdf, CITEMTYPE_CDF},
        {Clp, CITEMTYPE_CLP},
        {Cop, CITEMTYPE_COP},
        {Crc, CITEMTYPE_CRC},
        {Cuc, CITEMTYPE_CUC},
        {Cup, CITEMTYPE_CUP},
        {Cve, CITEMTYPE_CVE},
        {Czk, CITEMTYPE_CZK},
        {Djf, CITEMTYPE_DJF},
        {Dkk, CITEMTYPE_DKK},
        {Dop, CITEMTYPE_DOP},
        {Dzd, CITEMTYPE_DZD},
        {Egp, CITEMTYPE_EGP},
        {Ern, CITEMTYPE_ERN},
        {Etb, CITEMTYPE_ETB},
        {Fjd, CITEMTYPE_FJD},
        {Fkp, CITEMTYPE_FKP},
        {Gel, CITEMTYPE_GEL},
        {Ggp, CITEMTYPE_GGP},
        {Ghs, CITEMTYPE_GHS},
        {Gip, CITEMTYPE_GIP},
        {Gmd, CITEMTYPE_GMD},
        {Gnf, CITEMTYPE_GNF},
        {Gtq, CITEMTYPE_GTQ},
        {Gyd, CITEMTYPE_GYD},
        {Hnl, CITEMTYPE_HNL},
        {Hrk, CITEMTYPE_HRK},
        {Htg, CITEMTYPE_HTG},
        {Idr, CITEMTYPE_IDR},
        {Ils, CITEMTYPE_ILS},
        {Imp, CITEMTYPE_IMP},
        {Iqd, CITEMTYPE_IQD},
        {Irr, CITEMTYPE_IRR},
        {Isk, CITEMTYPE_ISK},
        {Jep, CITEMTYPE_JEP},
        {Jmd, CITEMTYPE_JMD},
        {Jod, CITEMTYPE_JOD},
        {Kes, CITEMTYPE_KES},
        {Kgs, CITEMTYPE_KGS},
        {Khr, CITEMTYPE_KHR},
        {Kmf, CITEMTYPE_KMF},
        {Kpw, CITEMTYPE_KPW},
        {Krw, CITEMTYPE_KRW},
        {Kwd, CITEMTYPE_KWD},
        {Kyd, CITEMTYPE_KYD},
        {Kzt, CITEMTYPE_KZT},
        {Lak, CITEMTYPE_LAK},
        {Lbp, CITEMTYPE_LBP},
        {Lkr, CITEMTYPE_LKR},
        {Lrd, CITEMTYPE_LRD},
        {Lsl, CITEMTYPE_LSL},
        {Lyd, CITEMTYPE_LYD},
        {Mad, CITEMTYPE_MAD},
        {Mdl, CITEMTYPE_MDL},
        {Mga, CITEMTYPE_MGA},
        {Mkd, CITEMTYPE_MKD},
        {Mmk, CITEMTYPE_MMK},
        {Mnt, CITEMTYPE_MNT},
        {Mop, CITEMTYPE_MOP},
        {Mru, CITEMTYPE_MRU},
        {Mur, CITEMTYPE_MUR},
        {Mvr, CITEMTYPE_MVR},
        {Mwk, CITEMTYPE_MWK},
        {Mzn, CITEMTYPE_MZN},
        {Nad, CITEMTYPE_NAD},
        {Ngn, CITEMTYPE_NGN},
        {Nio, CITEMTYPE_NIO},
        {Nok, CITEMTYPE_NOK},
        {Npr, CITEMTYPE_NPR},
        {Omr, CITEMTYPE_OMR},
        {Pab, CITEMTYPE_PAB},
        {Pen, CITEMTYPE_PEN},
        {Pgk, CITEMTYPE_PGK},
        {Pkr, CITEMTYPE_PKR},
        {Pln, CITEMTYPE_PLN},
        {Pyg, CITEMTYPE_PYG},
        {Qar, CITEMTYPE_QAR},
        {Ron, CITEMTYPE_RON},
        {Rsd, CITEMTYPE_RSD},
        {Rub, CITEMTYPE_RUB},
        {Rwf, CITEMTYPE_RWF},
        {Sar, CITEMTYPE_SAR},
        {Sbd, CITEMTYPE_SBD},
        {Scr, CITEMTYPE_SCR},
        {Sdg, CITEMTYPE_SDG},
        {Shp, CITEMTYPE_SHP},
        {Sll, CITEMTYPE_SLL},
        {Sos, CITEMTYPE_SOS},
        {Spl, CITEMTYPE_SPL},
        {Srd, CITEMTYPE_SRD},
        {Stn, CITEMTYPE_STN},
        {Svc, CITEMTYPE_SVC},
        {Syp, CITEMTYPE_SYP},
        {Szl, CITEMTYPE_SZL},
        {Tjs, CITEMTYPE_TJS},
        {Tmt, CITEMTYPE_TMT},
        {Tnd, CITEMTYPE_TND},
        {Top, CITEMTYPE_TOP},
        {Try, CITEMTYPE_TRY},
        {Ttd, CITEMTYPE_TTD},
        {Tvd, CITEMTYPE_TVD},
        {Twd, CITEMTYPE_TWD},
        {Tzs, CITEMTYPE_TZS},
        {Uah, CITEMTYPE_UAH},
        {Ugx, CITEMTYPE_UGX},
        {Uyu, CITEMTYPE_UYU},
        {Uzs, CITEMTYPE_UZS},
        {Vef, CITEMTYPE_VEF},
        {Vnd, CITEMTYPE_VND},
        {Vuv, CITEMTYPE_VUV},
        {Wst, CITEMTYPE_WST},
        {Xaf, CITEMTYPE_XAF},
        {Xcd, CITEMTYPE_XCD},
        {Xdr, CITEMTYPE_XDR},
        {Xof, CITEMTYPE_XOF},
        {Xpf, CITEMTYPE_XPF},
        {Yer, CITEMTYPE_YER},
        {Zmw, CITEMTYPE_ZMW},
        {Zwd, CITEMTYPE_ZWD},
        {Custom, CITEMTYPE_CUSTOM},
        {Tnbsv, CITEMTYPE_TNBSV},
        {TnXec, CITEMTYPE_TNXEC},
        {Cspr, CITEMTYPE_CSPR},
        {TnCspr, CITEMTYPE_TNCSPR},
        {Tn4bch, CITEMTYPE_TN4BCH},
    };

    return map;
}

auto identitytype_map() noexcept -> const NymTypeMap&
{
    using enum identity::Type;
    using enum ClaimType;
    static constexpr auto map = NymTypeMap{
        {invalid, Error},
        {individual, Individual},
        {organization, Organization},
        {business, Business},
        {government, Government},
        {server, Server},
        {bot, Bot},
    };

    return map;
}

auto sectiontype_map() noexcept -> const SectionTypeMap&
{
    using enum SectionType;
    using enum proto::ContactSectionName;
    static constexpr auto map = SectionTypeMap{
        {Error, CONTACTSECTION_ERROR},
        {Scope, CONTACTSECTION_SCOPE},
        {Identifier, CONTACTSECTION_IDENTIFIER},
        {Address, CONTACTSECTION_ADDRESS},
        {Communication, CONTACTSECTION_COMMUNICATION},
        {Profile, CONTACTSECTION_PROFILE},
        {Relationship, CONTACTSECTION_RELATIONSHIP},
        {Descriptor, CONTACTSECTION_DESCRIPTOR},
        {Event, CONTACTSECTION_EVENT},
        {Contract, CONTACTSECTION_CONTRACT},
        {Procedure, CONTACTSECTION_PROCEDURE},
    };

    return map;
}

auto unittype_map() noexcept -> const UnitTypeMap&
{
    using enum UnitType;
    static constexpr auto map = UnitTypeMap{
        {Error, ClaimType::Error},
        {Btc, ClaimType::Btc},
        {Eth, ClaimType::Eth},
        {Xrp, ClaimType::Xrp},
        {Ltc, ClaimType::Ltc},
        {Dao, ClaimType::Dao},
        {Xem, ClaimType::Xem},
        {Dash, ClaimType::Dash},
        {Maid, ClaimType::Maid},
        {Lsk, ClaimType::Lsk},
        {Doge, ClaimType::Doge},
        {Dgd, ClaimType::Dgd},
        {Xmr, ClaimType::Xmr},
        {Waves, ClaimType::Waves},
        {Nxt, ClaimType::Nxt},
        {Sc, ClaimType::Sc},
        {Steem, ClaimType::Steem},
        {Amp, ClaimType::Amp},
        {Xlm, ClaimType::Xlm},
        {Fct, ClaimType::Fct},
        {Bts, ClaimType::Bts},
        {Usd, ClaimType::Usd},
        {Eur, ClaimType::Eur},
        {Gbp, ClaimType::Gbp},
        {Inr, ClaimType::Inr},
        {Aud, ClaimType::Aud},
        {Cad, ClaimType::Cad},
        {Sgd, ClaimType::Sgd},
        {Chf, ClaimType::Chf},
        {Myr, ClaimType::Myr},
        {Jpy, ClaimType::Jpy},
        {Cny, ClaimType::Cny},
        {Nzd, ClaimType::Nzd},
        {Thb, ClaimType::Thb},
        {Huf, ClaimType::Huf},
        {Aed, ClaimType::Aed},
        {Hkd, ClaimType::Hkd},
        {Mxn, ClaimType::Mxn},
        {Zar, ClaimType::Zar},
        {Php, ClaimType::Php},
        {Sek, ClaimType::Sek},
        {Tnbtc, ClaimType::Tnbtc},
        {Tnxrp, ClaimType::Tnxrp},
        {Tnltx, ClaimType::Tnltx},
        {Tnxem, ClaimType::Tnxem},
        {Tndash, ClaimType::Tndash},
        {Tnmaid, ClaimType::Tnmaid},
        {Tnlsk, ClaimType::Tnlsk},
        {Tndoge, ClaimType::Tndoge},
        {Tnxmr, ClaimType::Tnxmr},
        {Tnwaves, ClaimType::Tnwaves},
        {Tnnxt, ClaimType::Tnnxt},
        {Tnsc, ClaimType::Tnsc},
        {Tnsteem, ClaimType::Tnsteem},
        {Bch, ClaimType::Bch},
        {Tnbch, ClaimType::Tnbch},
        {Pkt, ClaimType::Pkt},
        {Tnpkt, ClaimType::Tnpkt},
        {Ethereum_olympic, ClaimType::Ethereum_olympic},
        {Ethereum_classic, ClaimType::Ethereum_classic},
        {Ethereum_expanse, ClaimType::Ethereum_expanse},
        {Ethereum_morden, ClaimType::Ethereum_morden},
        {Ethereum_ropsten, ClaimType::Ethereum_ropsten},
        {Ethereum_rinkeby, ClaimType::Ethereum_rinkeby},
        {Ethereum_kovan, ClaimType::Ethereum_kovan},
        {Ethereum_sokol, ClaimType::Ethereum_sokol},
        {Ethereum_poa, ClaimType::Ethereum_poa},
        {Regtest, ClaimType::Regtest},
        {Unknown, ClaimType::Unknown},
        {Bnb, ClaimType::Bnb},
        {Sol, ClaimType::Sol},
        {Usdt, ClaimType::Usdt},
        {Ada, ClaimType::Ada},
        {Dot, ClaimType::Dot},
        {Usdc, ClaimType::Usdc},
        {Shib, ClaimType::Shib},
        {Luna, ClaimType::Luna},
        {Avax, ClaimType::Avax},
        {Uni, ClaimType::Uni},
        {Link, ClaimType::Link},
        {Wbtc, ClaimType::Wbtc},
        {Busd, ClaimType::Busd},
        {MatiC, ClaimType::Matic},
        {Algo, ClaimType::Algo},
        {Vet, ClaimType::Vet},
        {Axs, ClaimType::Axs},
        {Icp, ClaimType::Icp},
        {Cro, ClaimType::Cro},
        {Atom, ClaimType::Atom},
        {Theta, ClaimType::Theta},
        {Fil, ClaimType::Fil},
        {Trx, ClaimType::Trx},
        {Ftt, ClaimType::Ftt},
        {Etc, ClaimType::Etc},
        {Ftm, ClaimType::Ftm},
        {Dai, ClaimType::Dai},
        {Btcb, ClaimType::Btcb},
        {Egld, ClaimType::Egld},
        {Hbar, ClaimType::Hbar},
        {Xtz, ClaimType::Xtz},
        {Mana, ClaimType::Mana},
        {Near, ClaimType::Near},
        {Grt, ClaimType::Grt},
        {Cake, ClaimType::Cake},
        {Eos, ClaimType::Eos},
        {Flow, ClaimType::Flow},
        {Aave, ClaimType::Aave},
        {Klay, ClaimType::Klay},
        {Ksm, ClaimType::Ksm},
        {Xec, ClaimType::Xec},
        {Miota, ClaimType::Miota},
        {Hnt, ClaimType::Hnt},
        {Rune, ClaimType::Rune},
        {Bsv, ClaimType::Bsv},
        {Leo, ClaimType::Leo},
        {Neo, ClaimType::Neo},
        {One, ClaimType::One},
        {Qnt, ClaimType::Qnt},
        {Ust, ClaimType::Ust},
        {Mkr, ClaimType::Mkr},
        {Enj, ClaimType::Enj},
        {Chz, ClaimType::Chz},
        {Ar, ClaimType::Ar},
        {Stx, ClaimType::Stx},
        {Btt, ClaimType::Btt},
        {Hot, ClaimType::Hot},
        {Sand, ClaimType::Sand},
        {Omg, ClaimType::Omg},
        {Celo, ClaimType::Celo},
        {Zec, ClaimType::Zec},
        {Comp, ClaimType::Comp},
        {Tfuel, ClaimType::Tfuel},
        {Kda, ClaimType::Kda},
        {Lrc, ClaimType::Lrc},
        {Qtum, ClaimType::Qtum},
        {Crv, ClaimType::Crv},
        {Ht, ClaimType::Ht},
        {Nexo, ClaimType::Nexo},
        {Sushi, ClaimType::Sushi},
        {Kcs, ClaimType::Kcs},
        {Bat, ClaimType::Bat},
        {Okb, ClaimType::Okb},
        {Dcr, ClaimType::Dcr},
        {Icx, ClaimType::Icx},
        {Rvn, ClaimType::Rvn},
        {Scrt, ClaimType::Scrt},
        {Rev, ClaimType::Rev},
        {Audio, ClaimType::Audio},
        {Zil, ClaimType::Zil},
        {Tusd, ClaimType::Tusd},
        {Yfi, ClaimType::Yfi},
        {Mina, ClaimType::Mina},
        {Perp, ClaimType::Perp},
        {Xdc, ClaimType::Xdc},
        {Tel, ClaimType::Tel},
        {Snx, ClaimType::Snx},
        {Btg, ClaimType::Btg},
        {Afn, ClaimType::Afn},
        {All, ClaimType::All},
        {Amd, ClaimType::Amd},
        {Ang, ClaimType::Ang},
        {Aoa, ClaimType::Aoa},
        {Ars, ClaimType::Ars},
        {Awg, ClaimType::Awg},
        {Azn, ClaimType::Azn},
        {Bam, ClaimType::Bam},
        {Bbd, ClaimType::Bbd},
        {Bdt, ClaimType::Bdt},
        {Bgn, ClaimType::Bgn},
        {Bhd, ClaimType::Bhd},
        {Bif, ClaimType::Bif},
        {Bmd, ClaimType::Bmd},
        {Bnd, ClaimType::Bnd},
        {Bob, ClaimType::Bob},
        {Brl, ClaimType::Brl},
        {Bsd, ClaimType::Bsd},
        {Btn, ClaimType::Btn},
        {Bwp, ClaimType::Bwp},
        {Byn, ClaimType::Byn},
        {Bzd, ClaimType::Bzd},
        {Cdf, ClaimType::Cdf},
        {Clp, ClaimType::Clp},
        {Cop, ClaimType::Cop},
        {Crc, ClaimType::Crc},
        {Cuc, ClaimType::Cuc},
        {Cup, ClaimType::Cup},
        {Cve, ClaimType::Cve},
        {Czk, ClaimType::Czk},
        {Djf, ClaimType::Djf},
        {Dkk, ClaimType::Dkk},
        {Dop, ClaimType::Dop},
        {Dzd, ClaimType::Dzd},
        {Egp, ClaimType::Egp},
        {Ern, ClaimType::Ern},
        {Etb, ClaimType::Etb},
        {Fjd, ClaimType::Fjd},
        {Fkp, ClaimType::Fkp},
        {Gel, ClaimType::Gel},
        {Ggp, ClaimType::Ggp},
        {Ghs, ClaimType::Ghs},
        {Gip, ClaimType::Gip},
        {Gmd, ClaimType::Gmd},
        {Gnf, ClaimType::Gnf},
        {Gtq, ClaimType::Gtq},
        {Gyd, ClaimType::Gyd},
        {Hnl, ClaimType::Hnl},
        {Hrk, ClaimType::Hrk},
        {Htg, ClaimType::Htg},
        {Idr, ClaimType::Idr},
        {Ils, ClaimType::Ils},
        {Imp, ClaimType::Imp},
        {Iqd, ClaimType::Iqd},
        {Irr, ClaimType::Irr},
        {Isk, ClaimType::Isk},
        {Jep, ClaimType::Jep},
        {Jmd, ClaimType::Jmd},
        {Jod, ClaimType::Jod},
        {Kes, ClaimType::Kes},
        {Kgs, ClaimType::Kgs},
        {Khr, ClaimType::Khr},
        {Kmf, ClaimType::Kmf},
        {Kpw, ClaimType::Kpw},
        {Krw, ClaimType::Krw},
        {Kwd, ClaimType::Kwd},
        {Kyd, ClaimType::Kyd},
        {Kzt, ClaimType::Kzt},
        {Lak, ClaimType::Lak},
        {Lbp, ClaimType::Lbp},
        {Lkr, ClaimType::Lkr},
        {Lrd, ClaimType::Lrd},
        {Lsl, ClaimType::Lsl},
        {Lyd, ClaimType::Lyd},
        {Mad, ClaimType::Mad},
        {Mdl, ClaimType::Mdl},
        {Mga, ClaimType::Mga},
        {Mkd, ClaimType::Mkd},
        {Mmk, ClaimType::Mmk},
        {Mnt, ClaimType::Mnt},
        {Mop, ClaimType::Mop},
        {Mru, ClaimType::Mru},
        {Mur, ClaimType::Mur},
        {Mvr, ClaimType::Mvr},
        {Mwk, ClaimType::Mwk},
        {Mzn, ClaimType::Mzn},
        {Nad, ClaimType::Nad},
        {Ngn, ClaimType::Ngn},
        {Nio, ClaimType::Nio},
        {Nok, ClaimType::Nok},
        {Npr, ClaimType::Npr},
        {Omr, ClaimType::Omr},
        {Pab, ClaimType::Pab},
        {Pen, ClaimType::Pen},
        {Pgk, ClaimType::Pgk},
        {Pkr, ClaimType::Pkr},
        {Pln, ClaimType::Pln},
        {Pyg, ClaimType::Pyg},
        {Qar, ClaimType::Qar},
        {Ron, ClaimType::Ron},
        {Rsd, ClaimType::Rsd},
        {Rub, ClaimType::Rub},
        {Rwf, ClaimType::Rwf},
        {Sar, ClaimType::Sar},
        {Sbd, ClaimType::Sbd},
        {Scr, ClaimType::Scr},
        {Sdg, ClaimType::Sdg},
        {Shp, ClaimType::Shp},
        {Sll, ClaimType::Sll},
        {Sos, ClaimType::Sos},
        {Spl, ClaimType::Spl},
        {Srd, ClaimType::Srd},
        {Stn, ClaimType::Stn},
        {Svc, ClaimType::Svc},
        {Syp, ClaimType::Syp},
        {Szl, ClaimType::Szl},
        {Tjs, ClaimType::Tjs},
        {Tmt, ClaimType::Tmt},
        {Tnd, ClaimType::Tnd},
        {Top, ClaimType::Top},
        {Try, ClaimType::Try},
        {Ttd, ClaimType::Ttd},
        {Tvd, ClaimType::Tvd},
        {Twd, ClaimType::Twd},
        {Tzs, ClaimType::Tzs},
        {Uah, ClaimType::Uah},
        {Ugx, ClaimType::Ugx},
        {Uyu, ClaimType::Uyu},
        {Uzs, ClaimType::Uzs},
        {Vef, ClaimType::Vef},
        {Vnd, ClaimType::Vnd},
        {Vuv, ClaimType::Vuv},
        {Wst, ClaimType::Wst},
        {Xaf, ClaimType::Xaf},
        {Xcd, ClaimType::Xcd},
        {Xdr, ClaimType::Xdr},
        {Xof, ClaimType::Xof},
        {Xpf, ClaimType::Xpf},
        {Yer, ClaimType::Yer},
        {Zmw, ClaimType::Zmw},
        {Zwd, ClaimType::Zwd},
        {Custom, ClaimType::Custom},
        {Tnbsv, ClaimType::Tnbsv},
        {TnXec, ClaimType::TnXec},
        {Cspr, ClaimType::Cspr},
        {TnCspr, ClaimType::TnCspr},
        {Tn4bch, ClaimType::Tn4bch},
    };

    return map;
}

auto ClaimToNym(const identity::wot::claim::ClaimType in) noexcept
    -> identity::Type
{
    static const auto map =
        frozen::invert_unordered_map(identity::wot::claim::identitytype_map());

    try {

        return map.at(in);
    } catch (...) {

        return identity::Type::invalid;
    }
}

auto ClaimToUnit(const identity::wot::claim::ClaimType in) noexcept -> UnitType
{
    static const auto map =
        frozen::invert_unordered_map(identity::wot::claim::unittype_map());

    try {
        return map.at(in);
    } catch (...) {
        return UnitType::Error;
    }
}
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity
{
auto NymToClaim(const identity::Type in) noexcept
    -> identity::wot::claim::ClaimType
{
    try {
        return identity::wot::claim::identitytype_map().at(in);
    } catch (...) {
        return identity::wot::claim::ClaimType::Error;
    }
}
}  // namespace opentxs::identity

namespace opentxs::proto
{
auto translate(const ContactItemAttribute in) noexcept
    -> identity::wot::claim::Attribute
{
    static const auto map =
        frozen::invert_unordered_map(identity::wot::claim::attribute_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::wot::claim::Attribute::Error;
    }
}

auto translate(const ContactItemType in) noexcept
    -> identity::wot::claim::ClaimType
{
    static const auto map =
        frozen::invert_unordered_map(identity::wot::claim::claimtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::wot::claim::ClaimType::Error;
    }
}

auto translate(const ContactSectionName in) noexcept
    -> identity::wot::claim::SectionType
{
    static const auto map =
        frozen::invert_unordered_map(identity::wot::claim::sectiontype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::wot::claim::SectionType::Error;
    }
}
}  // namespace opentxs::proto

namespace opentxs::identity::wot::claim
{
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
}  // namespace opentxs::identity::wot::claim

namespace opentxs
{
auto UnitToClaim(const UnitType in) noexcept -> identity::wot::claim::ClaimType
{
    try {
        return identity::wot::claim::unittype_map().at(in);
    } catch (...) {
        return identity::wot::claim::ClaimType::Error;
    }
}
}  // namespace opentxs
