// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <limits>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

namespace opentxs
{
enum class UnitType : std::uint32_t {
    Error = 0,
    Btc = 1,
    Bch = 2,
    Eth = 3,
    Ethereum = 3,
    Ethereum_frontier = 3,
    Ethereum_homestead = 3,
    Ethereum_metropolis = 3,
    Xrp = 4,
    Ltc = 5,
    Dao = 6,
    Xem = 7,
    Dash = 8,
    Maid = 9,
    Lsk = 10,
    Doge = 11,
    Dgd = 12,
    Xmr = 13,
    Waves = 14,
    Nxt = 15,
    Sc = 16,
    Steem = 17,
    Amp = 18,
    Xlm = 19,
    Fct = 20,
    Bts = 21,
    Usd = 22,
    Eur = 23,
    Gbp = 24,
    Inr = 25,
    Aud = 26,
    Cad = 27,
    Sgd = 28,
    Chf = 29,
    Myr = 30,
    Jpy = 31,
    Cny = 32,
    Nzd = 33,
    Thb = 34,
    Huf = 35,
    Aed = 36,
    Hkd = 37,
    Mxn = 38,
    Zar = 39,
    Php = 40,
    Sek = 41,
    Pkt = 42,
    Tnbtc = 43,
    Tnbch = 44,
    Tnxrp = 45,
    Tnltx = 46,
    Tnxem = 47,
    Tndash = 48,
    Tnmaid = 49,
    Tnlsk = 50,
    Tndoge = 51,
    Tnxmr = 52,
    Tnwaves = 53,
    Tnnxt = 54,
    Tnsc = 55,
    Tnsteem = 56,
    Tnpkt = 57,
    Ethereum_olympic = 58,
    Ethereum_classic = 59,
    Ethereum_expanse = 60,
    Ethereum_morden = 61,
    Ethereum_ropsten = 62,
    Ethereum_rinkeby = 63,
    Ethereum_kovan = 64,
    Ethereum_sokol = 65,
    Ethereum_core = 66,
    Regtest = 67,
    Bnb = 68,     // Binance Coin
    Sol = 69,     // Solana
    Usdt = 70,    // Tether
    Ada = 71,     // Cardano
    Dot = 72,     // Polkadot
    Usdc = 73,    // USD Coin
    Shib = 74,    // SHIBA INU
    Luna = 75,    // Terra
    Avax = 76,    // Avalanche
    Uni = 77,     // Uniswap
    Link = 78,    // Chainlink
    Wbtc = 79,    // Wrapped Bitcoin
    Busd = 80,    // Binance USD
    MatiC = 81,   // Polygon
    Algo = 82,    // Algorand
    Vet = 83,     // VeChain
    Axs = 84,     // Axie Infinity
    Icp = 85,     // Internet Computer
    Cro = 86,     // Crypto.com Coin
    Atom = 87,    // Cosmos
    Theta = 88,   // THETA
    Fil = 89,     // Filecoin
    Trx = 90,     // TRON
    Ftt = 91,     // FTX Token
    Etc = 92,     // Ethereum Classic
    Ftm = 93,     // Fantom
    Dai = 94,     // Dai
    Btcb = 95,    // Bitcoin BEP2
    Egld = 96,    // Elrond
    Hbar = 97,    // Hedera
    Xtz = 98,     // Tezos
    Mana = 99,    // Decentraland
    Near = 100,   // NEAR Protocol
    Grt = 101,    // The Graph
    Cake = 102,   // PancakeSwap
    Eos = 103,    // EOS
    Flow = 104,   // Flow
    Aave = 105,   // Aave
    Klay = 106,   // Klaytn
    Ksm = 107,    // Kusama
    Xec = 108,    // eCash
    Miota = 109,  // IOTA
    Hnt = 110,    // Helium
    Rune = 111,   // THORChain
    Bsv = 112,    // Bitcoin SV
    Leo = 113,    // UNUS SED LEO
    Neo = 114,    // Neo
    One = 115,    // Harmony
    Qnt = 116,    // Quant
    Ust = 117,    // TerraUSD
    Mkr = 118,    // Maker
    Enj = 119,    // Enjin Coin
    Chz = 120,    // Chiliz
    Ar = 121,     // Arweave
    Stx = 122,    // Stacks
    Btt = 123,    // BitTorrent
    Hot = 124,    // Holo
    Sand = 125,   // The Sandbox
    Omg = 126,    // OMG Network
    Celo = 127,   // Celo
    Zec = 128,    // Zcash
    Comp = 129,   // Compound
    Tfuel = 130,  // Theta Fuel
    Kda = 131,    // Kadena
    Lrc = 132,    // Loopring
    Qtum = 133,   // Qtum
    Crv = 134,    // Curve DAO Token
    Ht = 135,     // Huobi Token
    Nexo = 136,   // Nexo
    Sushi = 137,  // SushiSwap
    Kcs = 138,    // KuCoin Token
    Bat = 139,    // Basic Attention Token
    Okb = 140,    // OKB
    Dcr = 141,    // Decred
    Icx = 142,    // ICON
    Rvn = 143,    // Ravencoin
    Scrt = 144,   // Secret
    Rev = 145,    // Revain
    Audio = 146,  // Audius
    Zil = 147,    // Zilliqa
    Tusd = 148,   // TrueUSD
    Yfi = 149,    // yearn.finance
    Mina = 150,   // Mina
    Perp = 151,   // Perpetual Protocol
    Xdc = 152,    // XDC Network
    Tel = 153,    // Telcoin
    Snx = 154,    // Synthetix
    Btg = 155,    // Bitcoin Gold
    Afn = 156,    // Afghanistan Afghani
    All = 157,    // Albania Lek
    Amd = 158,    // Armenia Dram
    Ang = 159,    // Netherlands Antilles Guilder
    Aoa = 160,    // Angola Kwanza
    Ars = 161,    // Argentina Peso
    Awg = 162,    // Aruba Guilder
    Azn = 163,    // Azerbaijan Manat
    Bam = 164,    // Bosnia and Herzegovina Convertible Mark
    Bbd = 165,    // Barbados Dollar
    Bdt = 166,    // Bangladesh Taka
    Bgn = 167,    // Bulgaria Lev
    Bhd = 168,    // Bahrain Dinar
    Bif = 169,    // Burundi Franc
    Bmd = 170,    // Bermuda Dollar
    Bnd = 171,    // Brunei Darussalam Dollar
    Bob = 172,    // Bolivia Bolíviano
    Brl = 173,    // Brazil Real
    Bsd = 174,    // Bahamas Dollar
    Btn = 175,    // Bhutan Ngultrum
    Bwp = 176,    // Botswana Pula
    Byn = 177,    // Belarus Ruble
    Bzd = 178,    // Belize Dollar
    Cdf = 179,    // Congo/Kinshasa Franc
    Clp = 180,    // Chile Peso
    Cop = 181,    // Colombia Peso
    Crc = 182,    // Costa Rica Colon
    Cuc = 183,    // Cuba Convertible Peso
    Cup = 184,    // Cuba Peso
    Cve = 185,    // Cape Verde Escudo
    Czk = 186,    // Czech Republic Koruna
    Djf = 187,    // Djibouti Franc
    Dkk = 188,    // Denmark Krone
    Dop = 189,    // Dominican Republic Peso
    Dzd = 190,    // Algeria Dinar
    Egp = 191,    // Egypt Pound
    Ern = 192,    // Eritrea Nakfa
    Etb = 193,    // Ethiopia Birr
    Fjd = 194,    // Fiji Dollar
    Fkp = 195,    // Falkland Islands (Malvinas) Pound
    Gel = 196,    // Georgia Lari
    Ggp = 197,    // Guernsey Pound
    Ghs = 198,    // Ghana Cedi
    Gip = 199,    // Gibraltar Pound
    Gmd = 200,    // Gambia Dalasi
    Gnf = 201,    // Guinea Franc
    Gtq = 202,    // Guatemala Quetzal
    Gyd = 203,    // Guyana Dollar
    Hnl = 204,    // Honduras Lempira
    Hrk = 205,    // Croatia Kuna
    Htg = 206,    // Haiti Gourde
    Idr = 207,    // Indonesia Rupiah
    Ils = 208,    // Israel Shekel
    Imp = 209,    // Isle of Man Pound
    Iqd = 210,    // Iraq Dinar
    Irr = 211,    // Iran Rial
    Isk = 212,    // Iceland Krona
    Jep = 213,    // Jersey Pound
    Jmd = 214,    // Jamaica Dollar
    Jod = 215,    // Jordan Dinar
    Kes = 216,    // Kenya Shilling
    Kgs = 217,    // Kyrgyzstan Som
    Khr = 218,    // Cambodia Riel
    Kmf = 219,    // Comorian Franc
    Kpw = 220,    // Korea (North) Won
    Krw = 221,    // Korea (South) Won
    Kwd = 222,    // Kuwait Dinar
    Kyd = 223,    // Cayman Islands Dollar
    Kzt = 224,    // Kazakhstan Tenge
    Lak = 225,    // Laos Kip
    Lbp = 226,    // Lebanon Pound
    Lkr = 227,    // Sri Lanka Rupee
    Lrd = 228,    // Liberia Dollar
    Lsl = 229,    // Lesotho Loti
    Lyd = 230,    // Libya Dinar
    Mad = 231,    // Morocco Dirham
    Mdl = 232,    // Moldova Leu
    Mga = 233,    // Madagascar Ariary
    Mkd = 234,    // Macedonia Denar
    Mmk = 235,    // Myanmar (Burma) Kyat
    Mnt = 236,    // Mongolia Tughrik
    Mop = 237,    // Macau Pataca
    Mru = 238,    // Mauritania Ouguiya
    Mur = 239,    // Mauritius Rupee
    Mvr = 240,    // Maldives (Maldive Islands) Rufiyaa
    Mwk = 241,    // Malawi Kwacha
    Mzn = 242,    // Mozambique Metical
    Nad = 243,    // Namibia Dollar
    Ngn = 244,    // Nigeria Naira
    Nio = 245,    // Nicaragua Cordoba
    Nok = 246,    // Norway Krone
    Npr = 247,    // Nepal Rupee
    Omr = 248,    // Oman Rial
    Pab = 249,    // Panama Balboa
    Pen = 250,    // Peru Sol
    Pgk = 251,    // Papua New Guinea Kina
    Pkr = 252,    // Pakistan Rupee
    Pln = 253,    // Poland Zloty
    Pyg = 254,    // Paraguay Guarani
    Qar = 255,    // Qatar Riyal
    Ron = 256,    // Romania Leu
    Rsd = 257,    // Serbia Dinar
    Rub = 258,    // Russia Ruble
    Rwf = 259,    // Rwanda Franc
    Sar = 260,    // Saudi Arabia Riyal
    Sbd = 261,    // Solomon Islands Dollar
    Scr = 262,    // Seychelles Rupee
    Sdg = 263,    // Sudan Pound
    Shp = 264,    // Saint Helena Pound
    Sll = 265,    // Sierra Leone Leone
    Sos = 266,    // Somalia Shilling
    Spl = 267,    // Seborga Luigino
    Srd = 268,    // Suriname Dollar
    Stn = 269,    // São Tomé and Príncipe Dobra
    Svc = 270,    // El Salvador Colon
    Syp = 271,    // Syria Pound
    Szl = 272,    // eSwatini Lilangeni
    Tjs = 273,    // Tajikistan Somoni
    Tmt = 274,    // Turkmenistan Manat
    Tnd = 275,    // Tunisia Dinar
    Top = 276,    // Tonga Pa'anga
    Try = 277,    // Turkey Lira
    Ttd = 278,    // Trinidad and Tobago Dollar
    Tvd = 279,    // Tuvalu Dollar
    Twd = 280,    // Taiwan New Dollar
    Tzs = 281,    // Tanzania Shilling
    Uah = 282,    // Ukraine Hryvnia
    Ugx = 283,    // Uganda Shilling
    Uyu = 284,    // Uruguay Peso
    Uzs = 285,    // Uzbekistan Som
    Vef = 286,    // Venezuela Bolívar
    Vnd = 287,    // Viet Nam Dong
    Vuv = 288,    // Vanuatu Vatu
    Wst = 289,    // Samoa Tala
    Xaf = 290,    // Communauté Financière Africaine (BEAC) CFA Franc BEAC
    Xcd = 291,    // East Caribbean Dollar
    Xdr = 292,    // International Monetary Fund (IMF) Special Drawing Rights
    Xof = 293,    // Communauté Financière Africaine (BCEAO) Franc
    Xpf = 294,    // Comptoirs Français du Pacifique (CFP) Franc
    Yer = 295,    // Yemen Rial
    Zmw = 296,    // Zambia Kwacha
    Zwd = 297,    // Zimbabwe Dollar
    Custom = 298,
    Tnbsv = 299,
    TnXec = 300,  // eCash testnet3
    Cspr = 301,
    TnCspr = 302,
    Tn4bch = 303,
    Ethereum_goerli = 304,
    Ethereum_sepolia = 305,
    Ethereum_holesovice = 306,
    Unknown = std::numeric_limits<std::uint32_t>::max(),
};
}  // namespace opentxs
