// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <string_view>

#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "ottest/Basic.hpp"  // IWYU pragma: keep

namespace ottest
{
namespace b = ot::blockchain::bitcoin::block;

using Script = b::Script;
using Position = b::script::Position;
using Pattern = b::script::Pattern;
using enum b::script::OP;
using namespace opentxs::literals;

const ot::UnallocatedVector<std::byte> expected_1_{
    std::byte{0},   std::byte{79},  std::byte{80},  std::byte{81},
    std::byte{82},  std::byte{83},  std::byte{84},  std::byte{85},
    std::byte{86},  std::byte{87},  std::byte{88},  std::byte{89},
    std::byte{90},  std::byte{91},  std::byte{92},  std::byte{93},
    std::byte{94},  std::byte{95},  std::byte{96},  std::byte{97},
    std::byte{98},  std::byte{99},  std::byte{100}, std::byte{101},
    std::byte{102}, std::byte{103}, std::byte{104}, std::byte{105},
    std::byte{106}, std::byte{107}, std::byte{108}, std::byte{109},
    std::byte{110}, std::byte{111}, std::byte{112}, std::byte{113},
    std::byte{114}, std::byte{115}, std::byte{116}, std::byte{117},
    std::byte{118}, std::byte{119}, std::byte{120}, std::byte{121},
    std::byte{122}, std::byte{123}, std::byte{124}, std::byte{125},
    std::byte{126}, std::byte{127}, std::byte{128}, std::byte{129},
    std::byte{130}, std::byte{131}, std::byte{132}, std::byte{133},
    std::byte{134}, std::byte{135}, std::byte{136}, std::byte{137},
    std::byte{138}, std::byte{139}, std::byte{140}, std::byte{141},
    std::byte{142}, std::byte{143}, std::byte{144}, std::byte{145},
    std::byte{146}, std::byte{147}, std::byte{148}, std::byte{149},
    std::byte{150}, std::byte{151}, std::byte{152}, std::byte{153},
    std::byte{154}, std::byte{155}, std::byte{156}, std::byte{157},
    std::byte{158}, std::byte{159}, std::byte{160}, std::byte{161},
    std::byte{162}, std::byte{163}, std::byte{164}, std::byte{165},
    std::byte{166}, std::byte{167}, std::byte{168}, std::byte{169},
    std::byte{170}, std::byte{171}, std::byte{172}, std::byte{173},
    std::byte{174}, std::byte{175}, std::byte{176}, std::byte{177},
    std::byte{178}, std::byte{179}, std::byte{180}, std::byte{181},
    std::byte{182}, std::byte{183}, std::byte{184}, std::byte{185},
    std::byte{253}, std::byte{254}, std::byte{255},
};
const ot::UnallocatedVector<b::script::OP> vector_1_{
    ZERO,
    ONE_NEGATE,
    RESERVED,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    ELEVEN,
    TWELVE,
    THIRTEEN,
    FOURTEEN,
    FIFTEEN,
    SIXTEEN,
    NOP,
    VER,
    IF,
    NOTIF,
    VERIF,
    VERNOTIF,
    ELSE,
    ENDIF,
    VERIFY,
    RETURN,
    TOALTSTACK,
    FROMALTSTACK,
    TWO_DROP,
    TWO_DUP,
    THREE_DUP,
    TWO_OVER,
    TWO_ROT,
    TWO_SWAP,
    IFDUP,
    DEPTH,
    DROP,
    DUP,
    NIP,
    OVER,
    PICK,
    ROLL,
    ROT,
    SWAP,
    TUCK,
    CAT,
    SUBSTR,
    LEFT,
    RIGHT,
    SIZE,
    INVERT,
    AND,
    OR,
    XOR,
    EQUAL,
    EQUALVERIFY,
    RESERVED1,
    RESERVED2,
    ONE_ADD,
    ONE_SUB,
    TWO_MUL,
    TWO_DIV,
    NEGATE,
    ABS,
    NOT,
    ZERO_NOTEQUAL,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LSHIFT,
    RSHIFT,
    BOOLAND,
    BOOLOR,
    NUMEQUAL,
    NUMEQUALVERIFY,
    NUMNOTEQUAL,
    LESSTHAN,
    GREATERTHAN,
    LESSTHANOREQUAL,
    GREATERTHANOREQUAL,
    MIN,
    MAX,
    WITHIN,
    RIPEMD160,
    SHA1,
    SHA256,
    HASH160,
    HASH256,
    CODESEPARATOR,
    CHECKSIG,
    CHECKSIGVERIFY,
    CHECKMULTISIG,
    CHECKMULTISIGVERIFY,
    NOP1,
    NOP2,
    NOP3,
    NOP4,
    NOP5,
    NOP6,
    NOP7,
    NOP8,
    NOP9,
    NOP10,
    PUBKEYHASH,
    PUBKEY,
    INVALIDOPCODE,
};
const ot::UnallocatedVector<b::script::OP> vector_2_{
    False,
    ONE_NEGATE,
    RESERVED,
    True,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    ELEVEN,
    TWELVE,
    THIRTEEN,
    FOURTEEN,
    FIFTEEN,
    SIXTEEN,
    NOP,
    VER,
    IF,
    NOTIF,
    VERIF,
    VERNOTIF,
    ELSE,
    ENDIF,
    VERIFY,
    RETURN,
    TOALTSTACK,
    FROMALTSTACK,
    TWO_DROP,
    TWO_DUP,
    THREE_DUP,
    TWO_OVER,
    TWO_ROT,
    TWO_SWAP,
    IFDUP,
    DEPTH,
    DROP,
    DUP,
    NIP,
    OVER,
    PICK,
    ROLL,
    ROT,
    SWAP,
    TUCK,
    CAT,
    SUBSTR,
    LEFT,
    RIGHT,
    SIZE,
    INVERT,
    AND,
    OR,
    XOR,
    EQUAL,
    EQUALVERIFY,
    RESERVED1,
    RESERVED2,
    ONE_ADD,
    ONE_SUB,
    TWO_MUL,
    TWO_DIV,
    NEGATE,
    ABS,
    NOT,
    ZERO_NOTEQUAL,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LSHIFT,
    RSHIFT,
    BOOLAND,
    BOOLOR,
    NUMEQUAL,
    NUMEQUALVERIFY,
    NUMNOTEQUAL,
    LESSTHAN,
    GREATERTHAN,
    LESSTHANOREQUAL,
    GREATERTHANOREQUAL,
    MIN,
    MAX,
    WITHIN,
    RIPEMD160,
    SHA1,
    SHA256,
    HASH160,
    HASH256,
    CODESEPARATOR,
    CHECKSIG,
    CHECKSIGVERIFY,
    CHECKMULTISIG,
    CHECKMULTISIGVERIFY,
    NOP1,
    CHECKLOCKTIMEVERIFY,
    CHECKSEQUENCEVERIFY,
    NOP4,
    NOP5,
    NOP6,
    NOP7,
    NOP8,
    NOP9,
    NOP10,
    PUBKEYHASH,
    PUBKEY,
    INVALIDOPCODE,
};

const ot::UnallocatedVector<std::byte> uncompressed_pubkey_1_{
    std::byte{0x04}, std::byte{0xae}, std::byte{0x1a}, std::byte{0x62},
    std::byte{0xfe}, std::byte{0x09}, std::byte{0xc5}, std::byte{0xf5},
    std::byte{0x1b}, std::byte{0x13}, std::byte{0x90}, std::byte{0x5f},
    std::byte{0x07}, std::byte{0xf0}, std::byte{0x6b}, std::byte{0x99},
    std::byte{0xa2}, std::byte{0xf7}, std::byte{0x15}, std::byte{0x9b},
    std::byte{0x22}, std::byte{0x25}, std::byte{0xf3}, std::byte{0x74},
    std::byte{0xcd}, std::byte{0x37}, std::byte{0x8d}, std::byte{0x71},
    std::byte{0x30}, std::byte{0x2f}, std::byte{0xa2}, std::byte{0x84},
    std::byte{0x14}, std::byte{0xe7}, std::byte{0xaa}, std::byte{0xb3},
    std::byte{0x73}, std::byte{0x97}, std::byte{0xf5}, std::byte{0x54},
    std::byte{0xa7}, std::byte{0xdf}, std::byte{0x5f}, std::byte{0x14},
    std::byte{0x2c}, std::byte{0x21}, std::byte{0xc1}, std::byte{0xb7},
    std::byte{0x30}, std::byte{0x3b}, std::byte{0x8a}, std::byte{0x06},
    std::byte{0x26}, std::byte{0xf1}, std::byte{0xba}, std::byte{0xde},
    std::byte{0xd5}, std::byte{0xc7}, std::byte{0x2a}, std::byte{0x70},
    std::byte{0x4f}, std::byte{0x7e}, std::byte{0x6c}, std::byte{0xd8},
    std::byte{0x4c},
};

const ot::UnallocatedVector<std::byte> compressed_pubkey_1_{
    std::byte{0x03}, std::byte{0xdf}, std::byte{0x51}, std::byte{0x98},
    std::byte{0x4d}, std::byte{0x6b}, std::byte{0x8b}, std::byte{0x8b},
    std::byte{0x1c}, std::byte{0xc6}, std::byte{0x93}, std::byte{0xe2},
    std::byte{0x39}, std::byte{0x49}, std::byte{0x1f}, std::byte{0x77},
    std::byte{0xa3}, std::byte{0x6c}, std::byte{0x9e}, std::byte{0x9d},
    std::byte{0xfe}, std::byte{0x4a}, std::byte{0x48}, std::byte{0x6e},
    std::byte{0x99}, std::byte{0x72}, std::byte{0xa1}, std::byte{0x8e},
    std::byte{0x03}, std::byte{0x61}, std::byte{0x0a}, std::byte{0x0d},
    std::byte{0x22},
};

const ot::UnallocatedVector<std::byte> compressed_pubkey_2_{
    std::byte{0x02}, std::byte{0x4f}, std::byte{0x35}, std::byte{0x5b},
    std::byte{0xdc}, std::byte{0xb7}, std::byte{0xcc}, std::byte{0x0a},
    std::byte{0xf7}, std::byte{0x28}, std::byte{0xef}, std::byte{0x3c},
    std::byte{0xce}, std::byte{0xb9}, std::byte{0x61}, std::byte{0x5d},
    std::byte{0x90}, std::byte{0x68}, std::byte{0x4b}, std::byte{0xb5},
    std::byte{0xb2}, std::byte{0xca}, std::byte{0x5f}, std::byte{0x85},
    std::byte{0x9a}, std::byte{0xb0}, std::byte{0xf0}, std::byte{0xb7},
    std::byte{0x04}, std::byte{0x07}, std::byte{0x58}, std::byte{0x71},
    std::byte{0xaa},
};

const ot::UnallocatedVector<std::byte> hash_160_{
    std::byte{0x12}, std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5},
    std::byte{0x88}, std::byte{0xca}, std::byte{0x9d}, std::byte{0x57},
    std::byte{0x87}, std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb},
    std::byte{0x29}, std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6},
    std::byte{0x3c}, std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c},
};

const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> p2pk_good_{
    {
        std::byte{65},   std::byte{0x04}, std::byte{0xae}, std::byte{0x1a},
        std::byte{0x62}, std::byte{0xfe}, std::byte{0x09}, std::byte{0xc5},
        std::byte{0xf5}, std::byte{0x1b}, std::byte{0x13}, std::byte{0x90},
        std::byte{0x5f}, std::byte{0x07}, std::byte{0xf0}, std::byte{0x6b},
        std::byte{0x99}, std::byte{0xa2}, std::byte{0xf7}, std::byte{0x15},
        std::byte{0x9b}, std::byte{0x22}, std::byte{0x25}, std::byte{0xf3},
        std::byte{0x74}, std::byte{0xcd}, std::byte{0x37}, std::byte{0x8d},
        std::byte{0x71}, std::byte{0x30}, std::byte{0x2f}, std::byte{0xa2},
        std::byte{0x84}, std::byte{0x14}, std::byte{0xe7}, std::byte{0xaa},
        std::byte{0xb3}, std::byte{0x73}, std::byte{0x97}, std::byte{0xf5},
        std::byte{0x54}, std::byte{0xa7}, std::byte{0xdf}, std::byte{0x5f},
        std::byte{0x14}, std::byte{0x2c}, std::byte{0x21}, std::byte{0xc1},
        std::byte{0xb7}, std::byte{0x30}, std::byte{0x3b}, std::byte{0x8a},
        std::byte{0x06}, std::byte{0x26}, std::byte{0xf1}, std::byte{0xba},
        std::byte{0xde}, std::byte{0xd5}, std::byte{0xc7}, std::byte{0x2a},
        std::byte{0x70}, std::byte{0x4f}, std::byte{0x7e}, std::byte{0x6c},
        std::byte{0xd8}, std::byte{0x4c}, std::byte{172},
    },
    {
        std::byte{76},   std::byte{65},   std::byte{0x04}, std::byte{0xae},
        std::byte{0x1a}, std::byte{0x62}, std::byte{0xfe}, std::byte{0x09},
        std::byte{0xc5}, std::byte{0xf5}, std::byte{0x1b}, std::byte{0x13},
        std::byte{0x90}, std::byte{0x5f}, std::byte{0x07}, std::byte{0xf0},
        std::byte{0x6b}, std::byte{0x99}, std::byte{0xa2}, std::byte{0xf7},
        std::byte{0x15}, std::byte{0x9b}, std::byte{0x22}, std::byte{0x25},
        std::byte{0xf3}, std::byte{0x74}, std::byte{0xcd}, std::byte{0x37},
        std::byte{0x8d}, std::byte{0x71}, std::byte{0x30}, std::byte{0x2f},
        std::byte{0xa2}, std::byte{0x84}, std::byte{0x14}, std::byte{0xe7},
        std::byte{0xaa}, std::byte{0xb3}, std::byte{0x73}, std::byte{0x97},
        std::byte{0xf5}, std::byte{0x54}, std::byte{0xa7}, std::byte{0xdf},
        std::byte{0x5f}, std::byte{0x14}, std::byte{0x2c}, std::byte{0x21},
        std::byte{0xc1}, std::byte{0xb7}, std::byte{0x30}, std::byte{0x3b},
        std::byte{0x8a}, std::byte{0x06}, std::byte{0x26}, std::byte{0xf1},
        std::byte{0xba}, std::byte{0xde}, std::byte{0xd5}, std::byte{0xc7},
        std::byte{0x2a}, std::byte{0x70}, std::byte{0x4f}, std::byte{0x7e},
        std::byte{0x6c}, std::byte{0xd8}, std::byte{0x4c}, std::byte{172},
    },
    {
        std::byte{77},   std::byte{65},   std::byte{0},    std::byte{0x04},
        std::byte{0xae}, std::byte{0x1a}, std::byte{0x62}, std::byte{0xfe},
        std::byte{0x09}, std::byte{0xc5}, std::byte{0xf5}, std::byte{0x1b},
        std::byte{0x13}, std::byte{0x90}, std::byte{0x5f}, std::byte{0x07},
        std::byte{0xf0}, std::byte{0x6b}, std::byte{0x99}, std::byte{0xa2},
        std::byte{0xf7}, std::byte{0x15}, std::byte{0x9b}, std::byte{0x22},
        std::byte{0x25}, std::byte{0xf3}, std::byte{0x74}, std::byte{0xcd},
        std::byte{0x37}, std::byte{0x8d}, std::byte{0x71}, std::byte{0x30},
        std::byte{0x2f}, std::byte{0xa2}, std::byte{0x84}, std::byte{0x14},
        std::byte{0xe7}, std::byte{0xaa}, std::byte{0xb3}, std::byte{0x73},
        std::byte{0x97}, std::byte{0xf5}, std::byte{0x54}, std::byte{0xa7},
        std::byte{0xdf}, std::byte{0x5f}, std::byte{0x14}, std::byte{0x2c},
        std::byte{0x21}, std::byte{0xc1}, std::byte{0xb7}, std::byte{0x30},
        std::byte{0x3b}, std::byte{0x8a}, std::byte{0x06}, std::byte{0x26},
        std::byte{0xf1}, std::byte{0xba}, std::byte{0xde}, std::byte{0xd5},
        std::byte{0xc7}, std::byte{0x2a}, std::byte{0x70}, std::byte{0x4f},
        std::byte{0x7e}, std::byte{0x6c}, std::byte{0xd8}, std::byte{0x4c},
        std::byte{172},
    },
    {
        std::byte{78},   std::byte{65},   std::byte{0},    std::byte{0},
        std::byte{0},    std::byte{0x04}, std::byte{0xae}, std::byte{0x1a},
        std::byte{0x62}, std::byte{0xfe}, std::byte{0x09}, std::byte{0xc5},
        std::byte{0xf5}, std::byte{0x1b}, std::byte{0x13}, std::byte{0x90},
        std::byte{0x5f}, std::byte{0x07}, std::byte{0xf0}, std::byte{0x6b},
        std::byte{0x99}, std::byte{0xa2}, std::byte{0xf7}, std::byte{0x15},
        std::byte{0x9b}, std::byte{0x22}, std::byte{0x25}, std::byte{0xf3},
        std::byte{0x74}, std::byte{0xcd}, std::byte{0x37}, std::byte{0x8d},
        std::byte{0x71}, std::byte{0x30}, std::byte{0x2f}, std::byte{0xa2},
        std::byte{0x84}, std::byte{0x14}, std::byte{0xe7}, std::byte{0xaa},
        std::byte{0xb3}, std::byte{0x73}, std::byte{0x97}, std::byte{0xf5},
        std::byte{0x54}, std::byte{0xa7}, std::byte{0xdf}, std::byte{0x5f},
        std::byte{0x14}, std::byte{0x2c}, std::byte{0x21}, std::byte{0xc1},
        std::byte{0xb7}, std::byte{0x30}, std::byte{0x3b}, std::byte{0x8a},
        std::byte{0x06}, std::byte{0x26}, std::byte{0xf1}, std::byte{0xba},
        std::byte{0xde}, std::byte{0xd5}, std::byte{0xc7}, std::byte{0x2a},
        std::byte{0x70}, std::byte{0x4f}, std::byte{0x7e}, std::byte{0x6c},
        std::byte{0xd8}, std::byte{0x4c}, std::byte{172},
    },
    {
        std::byte{33},   std::byte{0x03}, std::byte{0xdf}, std::byte{0x51},
        std::byte{0x98}, std::byte{0x4d}, std::byte{0x6b}, std::byte{0x8b},
        std::byte{0x8b}, std::byte{0x1c}, std::byte{0xc6}, std::byte{0x93},
        std::byte{0xe2}, std::byte{0x39}, std::byte{0x49}, std::byte{0x1f},
        std::byte{0x77}, std::byte{0xa3}, std::byte{0x6c}, std::byte{0x9e},
        std::byte{0x9d}, std::byte{0xfe}, std::byte{0x4a}, std::byte{0x48},
        std::byte{0x6e}, std::byte{0x99}, std::byte{0x72}, std::byte{0xa1},
        std::byte{0x8e}, std::byte{0x03}, std::byte{0x61}, std::byte{0x0a},
        std::byte{0x0d}, std::byte{0x22}, std::byte{172},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> p2pk_bad_{
    {
        std::byte{32},   std::byte{0xdf}, std::byte{0x51}, std::byte{0x98},
        std::byte{0x4d}, std::byte{0x6b}, std::byte{0x8b}, std::byte{0x8b},
        std::byte{0x1c}, std::byte{0xc6}, std::byte{0x93}, std::byte{0xe2},
        std::byte{0x39}, std::byte{0x49}, std::byte{0x1f}, std::byte{0x77},
        std::byte{0xa3}, std::byte{0x6c}, std::byte{0x9e}, std::byte{0x9d},
        std::byte{0xfe}, std::byte{0x4a}, std::byte{0x48}, std::byte{0x6e},
        std::byte{0x99}, std::byte{0x72}, std::byte{0xa1}, std::byte{0x8e},
        std::byte{0x03}, std::byte{0x61}, std::byte{0x0a}, std::byte{0x0d},
        std::byte{0x22}, std::byte{172},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> p2pkh_good_{
    {
        std::byte{118},  std::byte{169},  std::byte{20},   std::byte{0x12},
        std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88},
        std::byte{0xca}, std::byte{0x9d}, std::byte{0x57}, std::byte{0x87},
        std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29},
        std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c},
        std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c}, std::byte{136},
        std::byte{172},
    },
    {
        std::byte{118},  std::byte{169},  std::byte{76},   std::byte{20},
        std::byte{0x12}, std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5},
        std::byte{0x88}, std::byte{0xca}, std::byte{0x9d}, std::byte{0x57},
        std::byte{0x87}, std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb},
        std::byte{0x29}, std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6},
        std::byte{0x3c}, std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c},
        std::byte{136},  std::byte{172},
    },
    {
        std::byte{118},  std::byte{169},  std::byte{77},   std::byte{20},
        std::byte{0},    std::byte{0x12}, std::byte{0xab}, std::byte{0x8d},
        std::byte{0xc5}, std::byte{0x88}, std::byte{0xca}, std::byte{0x9d},
        std::byte{0x57}, std::byte{0x87}, std::byte{0xdd}, std::byte{0xe7},
        std::byte{0xeb}, std::byte{0x29}, std::byte{0x56}, std::byte{0x9d},
        std::byte{0xa6}, std::byte{0x3c}, std::byte{0x3a}, std::byte{0x23},
        std::byte{0x8c}, std::byte{136},  std::byte{172},
    },
    {
        std::byte{118},  std::byte{169},  std::byte{78},   std::byte{20},
        std::byte{0},    std::byte{0},    std::byte{0},    std::byte{0x12},
        std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88},
        std::byte{0xca}, std::byte{0x9d}, std::byte{0x57}, std::byte{0x87},
        std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29},
        std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c},
        std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c}, std::byte{136},
        std::byte{172},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> p2pkh_bad_{
    {
        std::byte{0},    std::byte{169},  std::byte{20},   std::byte{0x12},
        std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88},
        std::byte{0xca}, std::byte{0x9d}, std::byte{0x57}, std::byte{0x87},
        std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29},
        std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c},
        std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c}, std::byte{136},
        std::byte{172},
    },
    {
        std::byte{118},  std::byte{0},    std::byte{20},   std::byte{0x12},
        std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88},
        std::byte{0xca}, std::byte{0x9d}, std::byte{0x57}, std::byte{0x87},
        std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29},
        std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c},
        std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c}, std::byte{136},
        std::byte{172},
    },
    {
        std::byte{118},  std::byte{169},  std::byte{19},   std::byte{0xab},
        std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88}, std::byte{0xca},
        std::byte{0x9d}, std::byte{0x57}, std::byte{0x87}, std::byte{0xdd},
        std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29}, std::byte{0x56},
        std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c}, std::byte{0x3a},
        std::byte{0x23}, std::byte{0x8c}, std::byte{136},  std::byte{172},
    },
    {
        std::byte{118},  std::byte{169},  std::byte{20},   std::byte{0x12},
        std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88},
        std::byte{0xca}, std::byte{0x9d}, std::byte{0x57}, std::byte{0x87},
        std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29},
        std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c},
        std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c}, std::byte{0},
        std::byte{172},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> p2sh_good_{
    {
        std::byte{169},  std::byte{20},   std::byte{0x12}, std::byte{0xab},
        std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88}, std::byte{0xca},
        std::byte{0x9d}, std::byte{0x57}, std::byte{0x87}, std::byte{0xdd},
        std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29}, std::byte{0x56},
        std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c}, std::byte{0x3a},
        std::byte{0x23}, std::byte{0x8c}, std::byte{135},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> p2sh_bad_{
    {
        std::byte{169},  std::byte{76},   std::byte{20},   std::byte{0x12},
        std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88},
        std::byte{0xca}, std::byte{0x9d}, std::byte{0x57}, std::byte{0x87},
        std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29},
        std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c},
        std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c}, std::byte{135},
    },
    {
        std::byte{169},  std::byte{77},   std::byte{20},   std::byte{0},
        std::byte{0x12}, std::byte{0xab}, std::byte{0x8d}, std::byte{0xc5},
        std::byte{0x88}, std::byte{0xca}, std::byte{0x9d}, std::byte{0x57},
        std::byte{0x87}, std::byte{0xdd}, std::byte{0xe7}, std::byte{0xeb},
        std::byte{0x29}, std::byte{0x56}, std::byte{0x9d}, std::byte{0xa6},
        std::byte{0x3c}, std::byte{0x3a}, std::byte{0x23}, std::byte{0x8c},
        std::byte{135},
    },
    {
        std::byte{169},  std::byte{78},   std::byte{20},   std::byte{0},
        std::byte{0},    std::byte{0},    std::byte{0x12}, std::byte{0xab},
        std::byte{0x8d}, std::byte{0xc5}, std::byte{0x88}, std::byte{0xca},
        std::byte{0x9d}, std::byte{0x57}, std::byte{0x87}, std::byte{0xdd},
        std::byte{0xe7}, std::byte{0xeb}, std::byte{0x29}, std::byte{0x56},
        std::byte{0x9d}, std::byte{0xa6}, std::byte{0x3c}, std::byte{0x3a},
        std::byte{0x23}, std::byte{0x8c}, std::byte{135},
    },

};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> data_good_{
    {
        std::byte{106},
        std::byte{5},
        std::byte{'h'},
        std::byte{'e'},
        std::byte{'l'},
        std::byte{'l'},
        std::byte{'o'},
        std::byte{6},
        std::byte{'w'},
        std::byte{'o'},
        std::byte{'r'},
        std::byte{'l'},
        std::byte{'d'},
        std::byte{'!'},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> data_bad_{
    {
        std::byte{106},
        std::byte{11},
        std::byte{'h'},
        std::byte{'e'},
        std::byte{'l'},
        std::byte{'l'},
        std::byte{'o'},
        std::byte{' '},
        std::byte{'w'},
        std::byte{'o'},
        std::byte{'r'},
        std::byte{'l'},
        std::byte{'d'},
        std::byte{172},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> multisig_good_{
    {
        std::byte{82},   std::byte{65},   std::byte{0x04}, std::byte{0xae},
        std::byte{0x1a}, std::byte{0x62}, std::byte{0xfe}, std::byte{0x09},
        std::byte{0xc5}, std::byte{0xf5}, std::byte{0x1b}, std::byte{0x13},
        std::byte{0x90}, std::byte{0x5f}, std::byte{0x07}, std::byte{0xf0},
        std::byte{0x6b}, std::byte{0x99}, std::byte{0xa2}, std::byte{0xf7},
        std::byte{0x15}, std::byte{0x9b}, std::byte{0x22}, std::byte{0x25},
        std::byte{0xf3}, std::byte{0x74}, std::byte{0xcd}, std::byte{0x37},
        std::byte{0x8d}, std::byte{0x71}, std::byte{0x30}, std::byte{0x2f},
        std::byte{0xa2}, std::byte{0x84}, std::byte{0x14}, std::byte{0xe7},
        std::byte{0xaa}, std::byte{0xb3}, std::byte{0x73}, std::byte{0x97},
        std::byte{0xf5}, std::byte{0x54}, std::byte{0xa7}, std::byte{0xdf},
        std::byte{0x5f}, std::byte{0x14}, std::byte{0x2c}, std::byte{0x21},
        std::byte{0xc1}, std::byte{0xb7}, std::byte{0x30}, std::byte{0x3b},
        std::byte{0x8a}, std::byte{0x06}, std::byte{0x26}, std::byte{0xf1},
        std::byte{0xba}, std::byte{0xde}, std::byte{0xd5}, std::byte{0xc7},
        std::byte{0x2a}, std::byte{0x70}, std::byte{0x4f}, std::byte{0x7e},
        std::byte{0x6c}, std::byte{0xd8}, std::byte{0x4c}, std::byte{33},
        std::byte{0x03}, std::byte{0xdf}, std::byte{0x51}, std::byte{0x98},
        std::byte{0x4d}, std::byte{0x6b}, std::byte{0x8b}, std::byte{0x8b},
        std::byte{0x1c}, std::byte{0xc6}, std::byte{0x93}, std::byte{0xe2},
        std::byte{0x39}, std::byte{0x49}, std::byte{0x1f}, std::byte{0x77},
        std::byte{0xa3}, std::byte{0x6c}, std::byte{0x9e}, std::byte{0x9d},
        std::byte{0xfe}, std::byte{0x4a}, std::byte{0x48}, std::byte{0x6e},
        std::byte{0x99}, std::byte{0x72}, std::byte{0xa1}, std::byte{0x8e},
        std::byte{0x03}, std::byte{0x61}, std::byte{0x0a}, std::byte{0x0d},
        std::byte{0x22}, std::byte{33},   std::byte{0x02}, std::byte{0x4f},
        std::byte{0x35}, std::byte{0x5b}, std::byte{0xdc}, std::byte{0xb7},
        std::byte{0xcc}, std::byte{0x0a}, std::byte{0xf7}, std::byte{0x28},
        std::byte{0xef}, std::byte{0x3c}, std::byte{0xce}, std::byte{0xb9},
        std::byte{0x61}, std::byte{0x5d}, std::byte{0x90}, std::byte{0x68},
        std::byte{0x4b}, std::byte{0xb5}, std::byte{0xb2}, std::byte{0xca},
        std::byte{0x5f}, std::byte{0x85}, std::byte{0x9a}, std::byte{0xb0},
        std::byte{0xf0}, std::byte{0xb7}, std::byte{0x04}, std::byte{0x07},
        std::byte{0x58}, std::byte{0x71}, std::byte{0xaa}, std::byte{83},
        std::byte{174},
    },
    {
        std::byte{82},   std::byte{76},   std::byte{65},   std::byte{0x04},
        std::byte{0xae}, std::byte{0x1a}, std::byte{0x62}, std::byte{0xfe},
        std::byte{0x09}, std::byte{0xc5}, std::byte{0xf5}, std::byte{0x1b},
        std::byte{0x13}, std::byte{0x90}, std::byte{0x5f}, std::byte{0x07},
        std::byte{0xf0}, std::byte{0x6b}, std::byte{0x99}, std::byte{0xa2},
        std::byte{0xf7}, std::byte{0x15}, std::byte{0x9b}, std::byte{0x22},
        std::byte{0x25}, std::byte{0xf3}, std::byte{0x74}, std::byte{0xcd},
        std::byte{0x37}, std::byte{0x8d}, std::byte{0x71}, std::byte{0x30},
        std::byte{0x2f}, std::byte{0xa2}, std::byte{0x84}, std::byte{0x14},
        std::byte{0xe7}, std::byte{0xaa}, std::byte{0xb3}, std::byte{0x73},
        std::byte{0x97}, std::byte{0xf5}, std::byte{0x54}, std::byte{0xa7},
        std::byte{0xdf}, std::byte{0x5f}, std::byte{0x14}, std::byte{0x2c},
        std::byte{0x21}, std::byte{0xc1}, std::byte{0xb7}, std::byte{0x30},
        std::byte{0x3b}, std::byte{0x8a}, std::byte{0x06}, std::byte{0x26},
        std::byte{0xf1}, std::byte{0xba}, std::byte{0xde}, std::byte{0xd5},
        std::byte{0xc7}, std::byte{0x2a}, std::byte{0x70}, std::byte{0x4f},
        std::byte{0x7e}, std::byte{0x6c}, std::byte{0xd8}, std::byte{0x4c},
        std::byte{77},   std::byte{33},   std::byte{0},    std::byte{0x03},
        std::byte{0xdf}, std::byte{0x51}, std::byte{0x98}, std::byte{0x4d},
        std::byte{0x6b}, std::byte{0x8b}, std::byte{0x8b}, std::byte{0x1c},
        std::byte{0xc6}, std::byte{0x93}, std::byte{0xe2}, std::byte{0x39},
        std::byte{0x49}, std::byte{0x1f}, std::byte{0x77}, std::byte{0xa3},
        std::byte{0x6c}, std::byte{0x9e}, std::byte{0x9d}, std::byte{0xfe},
        std::byte{0x4a}, std::byte{0x48}, std::byte{0x6e}, std::byte{0x99},
        std::byte{0x72}, std::byte{0xa1}, std::byte{0x8e}, std::byte{0x03},
        std::byte{0x61}, std::byte{0x0a}, std::byte{0x0d}, std::byte{0x22},
        std::byte{78},   std::byte{33},   std::byte{0},    std::byte{0},
        std::byte{0},    std::byte{0x02}, std::byte{0x4f}, std::byte{0x35},
        std::byte{0x5b}, std::byte{0xdc}, std::byte{0xb7}, std::byte{0xcc},
        std::byte{0x0a}, std::byte{0xf7}, std::byte{0x28}, std::byte{0xef},
        std::byte{0x3c}, std::byte{0xce}, std::byte{0xb9}, std::byte{0x61},
        std::byte{0x5d}, std::byte{0x90}, std::byte{0x68}, std::byte{0x4b},
        std::byte{0xb5}, std::byte{0xb2}, std::byte{0xca}, std::byte{0x5f},
        std::byte{0x85}, std::byte{0x9a}, std::byte{0xb0}, std::byte{0xf0},
        std::byte{0xb7}, std::byte{0x04}, std::byte{0x07}, std::byte{0x58},
        std::byte{0x71}, std::byte{0xaa}, std::byte{83},   std::byte{174},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> multisig_bad_{
    {
        std::byte{172},  std::byte{82},   std::byte{65},   std::byte{0x04},
        std::byte{0xae}, std::byte{0x1a}, std::byte{0x62}, std::byte{0xfe},
        std::byte{0x09}, std::byte{0xc5}, std::byte{0xf5}, std::byte{0x1b},
        std::byte{0x13}, std::byte{0x90}, std::byte{0x5f}, std::byte{0x07},
        std::byte{0xf0}, std::byte{0x6b}, std::byte{0x99}, std::byte{0xa2},
        std::byte{0xf7}, std::byte{0x15}, std::byte{0x9b}, std::byte{0x22},
        std::byte{0x25}, std::byte{0xf3}, std::byte{0x74}, std::byte{0xcd},
        std::byte{0x37}, std::byte{0x8d}, std::byte{0x71}, std::byte{0x30},
        std::byte{0x2f}, std::byte{0xa2}, std::byte{0x84}, std::byte{0x14},
        std::byte{0xe7}, std::byte{0xaa}, std::byte{0xb3}, std::byte{0x73},
        std::byte{0x97}, std::byte{0xf5}, std::byte{0x54}, std::byte{0xa7},
        std::byte{0xdf}, std::byte{0x5f}, std::byte{0x14}, std::byte{0x2c},
        std::byte{0x21}, std::byte{0xc1}, std::byte{0xb7}, std::byte{0x30},
        std::byte{0x3b}, std::byte{0x8a}, std::byte{0x06}, std::byte{0x26},
        std::byte{0xf1}, std::byte{0xba}, std::byte{0xde}, std::byte{0xd5},
        std::byte{0xc7}, std::byte{0x2a}, std::byte{0x70}, std::byte{0x4f},
        std::byte{0x7e}, std::byte{0x6c}, std::byte{0xd8}, std::byte{0x4c},
        std::byte{33},   std::byte{0x03}, std::byte{0xdf}, std::byte{0x51},
        std::byte{0x98}, std::byte{0x4d}, std::byte{0x6b}, std::byte{0x8b},
        std::byte{0x8b}, std::byte{0x1c}, std::byte{0xc6}, std::byte{0x93},
        std::byte{0xe2}, std::byte{0x39}, std::byte{0x49}, std::byte{0x1f},
        std::byte{0x77}, std::byte{0xa3}, std::byte{0x6c}, std::byte{0x9e},
        std::byte{0x9d}, std::byte{0xfe}, std::byte{0x4a}, std::byte{0x48},
        std::byte{0x6e}, std::byte{0x99}, std::byte{0x72}, std::byte{0xa1},
        std::byte{0x8e}, std::byte{0x03}, std::byte{0x61}, std::byte{0x0a},
        std::byte{0x0d}, std::byte{0x22}, std::byte{33},   std::byte{0x02},
        std::byte{0x4f}, std::byte{0x35}, std::byte{0x5b}, std::byte{0xdc},
        std::byte{0xb7}, std::byte{0xcc}, std::byte{0x0a}, std::byte{0xf7},
        std::byte{0x28}, std::byte{0xef}, std::byte{0x3c}, std::byte{0xce},
        std::byte{0xb9}, std::byte{0x61}, std::byte{0x5d}, std::byte{0x90},
        std::byte{0x68}, std::byte{0x4b}, std::byte{0xb5}, std::byte{0xb2},
        std::byte{0xca}, std::byte{0x5f}, std::byte{0x85}, std::byte{0x9a},
        std::byte{0xb0}, std::byte{0xf0}, std::byte{0xb7}, std::byte{0x04},
        std::byte{0x07}, std::byte{0x58}, std::byte{0x71}, std::byte{0xaa},
        std::byte{83},   std::byte{174},
    },
};
const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>>
    multisig_malformed_{
        {
            std::byte{82},   std::byte{65},   std::byte{0x04}, std::byte{0xae},
            std::byte{0x1a}, std::byte{0x62}, std::byte{0xfe}, std::byte{0x09},
            std::byte{0xc5}, std::byte{0xf5}, std::byte{0x1b}, std::byte{0x13},
            std::byte{0x90}, std::byte{0x5f}, std::byte{0x07}, std::byte{0xf0},
            std::byte{0x6b}, std::byte{0x99}, std::byte{0xa2}, std::byte{0xf7},
            std::byte{0x15}, std::byte{0x9b}, std::byte{0x22}, std::byte{0x25},
            std::byte{0xf3}, std::byte{0x74}, std::byte{0xcd}, std::byte{0x37},
            std::byte{0x8d}, std::byte{0x71}, std::byte{0x30}, std::byte{0x2f},
            std::byte{0xa2}, std::byte{0x84}, std::byte{0x14}, std::byte{0xe7},
            std::byte{0xaa}, std::byte{0xb3}, std::byte{0x73}, std::byte{0x97},
            std::byte{0xf5}, std::byte{0x54}, std::byte{0xa7}, std::byte{0xdf},
            std::byte{0x5f}, std::byte{0x14}, std::byte{0x2c}, std::byte{0x21},
            std::byte{0xc1}, std::byte{0xb7}, std::byte{0x30}, std::byte{0x3b},
            std::byte{0x8a}, std::byte{0x06}, std::byte{0x26}, std::byte{0xf1},
            std::byte{0xba}, std::byte{0xde}, std::byte{0xd5}, std::byte{0xc7},
            std::byte{0x2a}, std::byte{0x70}, std::byte{0x4f}, std::byte{0x7e},
            std::byte{0x6c}, std::byte{0xd8}, std::byte{0x4c}, std::byte{33},
            std::byte{0x03}, std::byte{0xdf}, std::byte{0x51}, std::byte{0x98},
            std::byte{0x4d}, std::byte{0x6b}, std::byte{0x8b}, std::byte{0x8b},
            std::byte{0x1c}, std::byte{0xc6}, std::byte{0x93}, std::byte{0xe2},
            std::byte{0x39}, std::byte{0x49}, std::byte{0x1f}, std::byte{0x77},
            std::byte{0xa3}, std::byte{0x6c}, std::byte{0x9e}, std::byte{0x9d},
            std::byte{0xfe}, std::byte{0x4a}, std::byte{0x48}, std::byte{0x6e},
            std::byte{0x99}, std::byte{0x72}, std::byte{0xa1}, std::byte{0x8e},
            std::byte{0x03}, std::byte{0x61}, std::byte{0x0a}, std::byte{0x0d},
            std::byte{0x22}, std::byte{33},   std::byte{0x02}, std::byte{0x4f},
            std::byte{0x35}, std::byte{0x5b}, std::byte{0xdc}, std::byte{0xb7},
            std::byte{0xcc}, std::byte{0x0a}, std::byte{0xf7}, std::byte{0x28},
            std::byte{0xef}, std::byte{0x3c}, std::byte{0xce}, std::byte{0xb9},
            std::byte{0x61}, std::byte{0x5d}, std::byte{0x90}, std::byte{0x68},
            std::byte{0x4b}, std::byte{0xb5}, std::byte{0xb2}, std::byte{0xca},
            std::byte{0x5f}, std::byte{0x85}, std::byte{0x9a}, std::byte{0xb0},
            std::byte{0xf0}, std::byte{0xb7}, std::byte{0x04}, std::byte{0x07},
            std::byte{0x58}, std::byte{0x71}, std::byte{0xaa}, std::byte{81},
            std::byte{174},
        },
        {
            std::byte{84},   std::byte{65},   std::byte{0x04}, std::byte{0xae},
            std::byte{0x1a}, std::byte{0x62}, std::byte{0xfe}, std::byte{0x09},
            std::byte{0xc5}, std::byte{0xf5}, std::byte{0x1b}, std::byte{0x13},
            std::byte{0x90}, std::byte{0x5f}, std::byte{0x07}, std::byte{0xf0},
            std::byte{0x6b}, std::byte{0x99}, std::byte{0xa2}, std::byte{0xf7},
            std::byte{0x15}, std::byte{0x9b}, std::byte{0x22}, std::byte{0x25},
            std::byte{0xf3}, std::byte{0x74}, std::byte{0xcd}, std::byte{0x37},
            std::byte{0x8d}, std::byte{0x71}, std::byte{0x30}, std::byte{0x2f},
            std::byte{0xa2}, std::byte{0x84}, std::byte{0x14}, std::byte{0xe7},
            std::byte{0xaa}, std::byte{0xb3}, std::byte{0x73}, std::byte{0x97},
            std::byte{0xf5}, std::byte{0x54}, std::byte{0xa7}, std::byte{0xdf},
            std::byte{0x5f}, std::byte{0x14}, std::byte{0x2c}, std::byte{0x21},
            std::byte{0xc1}, std::byte{0xb7}, std::byte{0x30}, std::byte{0x3b},
            std::byte{0x8a}, std::byte{0x06}, std::byte{0x26}, std::byte{0xf1},
            std::byte{0xba}, std::byte{0xde}, std::byte{0xd5}, std::byte{0xc7},
            std::byte{0x2a}, std::byte{0x70}, std::byte{0x4f}, std::byte{0x7e},
            std::byte{0x6c}, std::byte{0xd8}, std::byte{0x4c}, std::byte{33},
            std::byte{0x03}, std::byte{0xdf}, std::byte{0x51}, std::byte{0x98},
            std::byte{0x4d}, std::byte{0x6b}, std::byte{0x8b}, std::byte{0x8b},
            std::byte{0x1c}, std::byte{0xc6}, std::byte{0x93}, std::byte{0xe2},
            std::byte{0x39}, std::byte{0x49}, std::byte{0x1f}, std::byte{0x77},
            std::byte{0xa3}, std::byte{0x6c}, std::byte{0x9e}, std::byte{0x9d},
            std::byte{0xfe}, std::byte{0x4a}, std::byte{0x48}, std::byte{0x6e},
            std::byte{0x99}, std::byte{0x72}, std::byte{0xa1}, std::byte{0x8e},
            std::byte{0x03}, std::byte{0x61}, std::byte{0x0a}, std::byte{0x0d},
            std::byte{0x22}, std::byte{33},   std::byte{0x02}, std::byte{0x4f},
            std::byte{0x35}, std::byte{0x5b}, std::byte{0xdc}, std::byte{0xb7},
            std::byte{0xcc}, std::byte{0x0a}, std::byte{0xf7}, std::byte{0x28},
            std::byte{0xef}, std::byte{0x3c}, std::byte{0xce}, std::byte{0xb9},
            std::byte{0x61}, std::byte{0x5d}, std::byte{0x90}, std::byte{0x68},
            std::byte{0x4b}, std::byte{0xb5}, std::byte{0xb2}, std::byte{0xca},
            std::byte{0x5f}, std::byte{0x85}, std::byte{0x9a}, std::byte{0xb0},
            std::byte{0xf0}, std::byte{0xb7}, std::byte{0x04}, std::byte{0x07},
            std::byte{0x58}, std::byte{0x71}, std::byte{0xaa}, std::byte{83},
            std::byte{174},
        },
    };

const ot::UnallocatedVector<ot::UnallocatedVector<std::byte>> input_good_{
    {
        std::byte{0x47}, std::byte{0x30}, std::byte{0x44}, std::byte{0x02},
        std::byte{0x20}, std::byte{0x25}, std::byte{0xbc}, std::byte{0xa5},
        std::byte{0xdc}, std::byte{0x0f}, std::byte{0xe4}, std::byte{0x2a},
        std::byte{0xca}, std::byte{0x5f}, std::byte{0x07}, std::byte{0xc9},
        std::byte{0xb3}, std::byte{0xfe}, std::byte{0x1b}, std::byte{0x3f},
        std::byte{0x72}, std::byte{0x11}, std::byte{0x3f}, std::byte{0xfb},
        std::byte{0xc3}, std::byte{0x52}, std::byte{0x2f}, std::byte{0x8d},
        std::byte{0x3e}, std::byte{0xbb}, std::byte{0x24}, std::byte{0x57},
        std::byte{0xf5}, std::byte{0xbd}, std::byte{0xf8}, std::byte{0xf9},
        std::byte{0xb2}, std::byte{0x02}, std::byte{0x20}, std::byte{0x30},
        std::byte{0xff}, std::byte{0x68}, std::byte{0x7c}, std::byte{0x00},
        std::byte{0xa6}, std::byte{0x3e}, std::byte{0x81}, std::byte{0x0b},
        std::byte{0x21}, std::byte{0xe4}, std::byte{0x47}, std::byte{0xd3},
        std::byte{0xa5}, std::byte{0x7b}, std::byte{0x27}, std::byte{0x49},
        std::byte{0xeb}, std::byte{0xea}, std::byte{0x55}, std::byte{0x3c},
        std::byte{0xab}, std::byte{0x76}, std::byte{0x3e}, std::byte{0xb9},
        std::byte{0xb9}, std::byte{0x9e}, std::byte{0x1b}, std::byte{0x98},
        std::byte{0x39}, std::byte{0x65}, std::byte{0x3b}, std::byte{0x01},
        std::byte{0x41}, std::byte{0x04}, std::byte{0x46}, std::byte{0x9f},
        std::byte{0x7e}, std::byte{0xb5}, std::byte{0x4b}, std::byte{0x90},
        std::byte{0xd9}, std::byte{0x01}, std::byte{0x06}, std::byte{0xb1},
        std::byte{0xa5}, std::byte{0x41}, std::byte{0x2b}, std::byte{0x41},
        std::byte{0xa2}, std::byte{0x35}, std::byte{0x16}, std::byte{0x02},
        std::byte{0x8e}, std::byte{0x81}, std::byte{0xad}, std::byte{0x35},
        std::byte{0xe0}, std::byte{0x41}, std::byte{0x8a}, std::byte{0x44},
        std::byte{0x60}, std::byte{0x70}, std::byte{0x7a}, std::byte{0xe3},
        std::byte{0x9a}, std::byte{0x4b}, std::byte{0xf0}, std::byte{0x10},
        std::byte{0x1b}, std::byte{0x63}, std::byte{0x22}, std::byte{0x60},
        std::byte{0xfb}, std::byte{0x08}, std::byte{0x97}, std::byte{0x9a},
        std::byte{0xba}, std::byte{0x0c}, std::byte{0xee}, std::byte{0xa5},
        std::byte{0x76}, std::byte{0xb5}, std::byte{0x40}, std::byte{0x0c},
        std::byte{0x7c}, std::byte{0xf3}, std::byte{0x0b}, std::byte{0x53},
        std::byte{0x9b}, std::byte{0x05}, std::byte{0x5e}, std::byte{0xc4},
        std::byte{0xc0}, std::byte{0xb9}, std::byte{0x6a}, std::byte{0xb0},
        std::byte{0x09}, std::byte{0x84},
    },
};

const ot::UnallocatedVector<std::byte> sig_1_{
    std::byte{0x30}, std::byte{0x44}, std::byte{0x02}, std::byte{0x20},
    std::byte{0x25}, std::byte{0xbc}, std::byte{0xa5}, std::byte{0xdc},
    std::byte{0x0f}, std::byte{0xe4}, std::byte{0x2a}, std::byte{0xca},
    std::byte{0x5f}, std::byte{0x07}, std::byte{0xc9}, std::byte{0xb3},
    std::byte{0xfe}, std::byte{0x1b}, std::byte{0x3f}, std::byte{0x72},
    std::byte{0x11}, std::byte{0x3f}, std::byte{0xfb}, std::byte{0xc3},
    std::byte{0x52}, std::byte{0x2f}, std::byte{0x8d}, std::byte{0x3e},
    std::byte{0xbb}, std::byte{0x24}, std::byte{0x57}, std::byte{0xf5},
    std::byte{0xbd}, std::byte{0xf8}, std::byte{0xf9}, std::byte{0xb2},
    std::byte{0x02}, std::byte{0x20}, std::byte{0x30}, std::byte{0xff},
    std::byte{0x68}, std::byte{0x7c}, std::byte{0x00}, std::byte{0xa6},
    std::byte{0x3e}, std::byte{0x81}, std::byte{0x0b}, std::byte{0x21},
    std::byte{0xe4}, std::byte{0x47}, std::byte{0xd3}, std::byte{0xa5},
    std::byte{0x7b}, std::byte{0x27}, std::byte{0x49}, std::byte{0xeb},
    std::byte{0xea}, std::byte{0x55}, std::byte{0x3c}, std::byte{0xab},
    std::byte{0x76}, std::byte{0x3e}, std::byte{0xb9}, std::byte{0xb9},
    std::byte{0x9e}, std::byte{0x1b}, std::byte{0x98}, std::byte{0x39},
    std::byte{0x65}, std::byte{0x3b}, std::byte{0x01},
};

const ot::UnallocatedVector<std::byte> key_1_{
    std::byte{0x04}, std::byte{0x46}, std::byte{0x9f}, std::byte{0x7e},
    std::byte{0xb5}, std::byte{0x4b}, std::byte{0x90}, std::byte{0xd9},
    std::byte{0x01}, std::byte{0x06}, std::byte{0xb1}, std::byte{0xa5},
    std::byte{0x41}, std::byte{0x2b}, std::byte{0x41}, std::byte{0xa2},
    std::byte{0x35}, std::byte{0x16}, std::byte{0x02}, std::byte{0x8e},
    std::byte{0x81}, std::byte{0xad}, std::byte{0x35}, std::byte{0xe0},
    std::byte{0x41}, std::byte{0x8a}, std::byte{0x44}, std::byte{0x60},
    std::byte{0x70}, std::byte{0x7a}, std::byte{0xe3}, std::byte{0x9a},
    std::byte{0x4b}, std::byte{0xf0}, std::byte{0x10}, std::byte{0x1b},
    std::byte{0x63}, std::byte{0x22}, std::byte{0x60}, std::byte{0xfb},
    std::byte{0x08}, std::byte{0x97}, std::byte{0x9a}, std::byte{0xba},
    std::byte{0x0c}, std::byte{0xee}, std::byte{0xa5}, std::byte{0x76},
    std::byte{0xb5}, std::byte{0x40}, std::byte{0x0c}, std::byte{0x7c},
    std::byte{0xf3}, std::byte{0x0b}, std::byte{0x53}, std::byte{0x9b},
    std::byte{0x05}, std::byte{0x5e}, std::byte{0xc4}, std::byte{0xc0},
    std::byte{0xb9}, std::byte{0x6a}, std::byte{0xb0}, std::byte{0x09},
    std::byte{0x84},
};

static auto byte_array(const ot::Space& in) noexcept
{
    auto out = ot::ByteArray{};
    ot::copy(ot::reader(in), out.WriteInto());

    return out;
}

constexpr auto chain_{ot::blockchain::Type::Bitcoin};
using Position = ot::blockchain::bitcoin::block::script::Position;

TEST(Test_BitcoinScript, opcodes_general)
{
    auto elements1 = b::ScriptElements{};
    auto elements2 = b::ScriptElements{};

    for (const auto& op : vector_1_) {
        auto& element = elements1.emplace_back();
        element.opcode_ = op;
    }

    for (const auto& op : vector_2_) {
        auto& element = elements2.emplace_back();
        element.opcode_ = op;
    }

    const auto script1 =
        ot::factory::BitcoinScript(chain_, elements1, Position::Output, {});
    const auto script2 =
        ot::factory::BitcoinScript(chain_, elements2, Position::Output, {});

    EXPECT_TRUE(script1.IsValid());
    EXPECT_TRUE(script2.IsValid());

    auto serialized1 = ot::Space{};
    auto serialized2 = ot::Space{};

    EXPECT_TRUE(script1.Serialize(ot::writer(serialized1)));
    EXPECT_TRUE(script2.Serialize(ot::writer(serialized2)));

    EXPECT_EQ(expected_1_.size(), serialized1.size());
    EXPECT_EQ(
        std::memcmp(
            expected_1_.data(),
            serialized1.data(),
            std::min(expected_1_.size(), serialized1.size())),
        0);

    EXPECT_EQ(expected_1_.size(), serialized2.size());
    EXPECT_EQ(
        std::memcmp(
            expected_1_.data(),
            serialized2.data(),
            std::min(expected_1_.size(), serialized2.size())),
        0);
}

TEST(Test_BitcoinScript, opcodes_push_inline)
{
    for (auto n = std::uint8_t{1}; n < 76; ++n) {
        {
            auto elements = b::ScriptElements{};
            auto serialized = ot::Space{};
            {
                auto& [opcode, invalid, bytes, data] = elements.emplace_back();
                opcode = static_cast<b::script::OP>(n);
                data = byte_array(ot::space(n));
                serialized = ot::space(data->Bytes());
                serialized.emplace(serialized.begin(), std::byte{n});
            }

            auto script1 = ot::factory::BitcoinScript(
                chain_, elements, Position::Output, {});
            auto script2 = ot::factory::BitcoinScript(
                chain_,
                ot::reader(serialized),
                Position::Output,
                false,
                false,
                {});

            EXPECT_TRUE(script1.IsValid());
            EXPECT_TRUE(script2.IsValid());

            {
                const auto e = script1.get();

                ASSERT_EQ(1, e.size());

                const auto& [opcode, invalid, bytes, data] = e[0];

                EXPECT_EQ(n, static_cast<std::uint8_t>(opcode));
                EXPECT_FALSE(bytes.has_value());
                ASSERT_TRUE(data.has_value());
                EXPECT_EQ(n, data.value().size());
            }

            {
                const auto e = script2.get();

                ASSERT_EQ(1, e.size());

                const auto& [opcode, invalid, bytes, data] = e[0];

                EXPECT_EQ(n, static_cast<std::uint8_t>(opcode));
                EXPECT_FALSE(bytes.has_value());
                ASSERT_TRUE(data.has_value());
                EXPECT_EQ(n, data.value().size());
            }
        }

        {
            auto elements = b::ScriptElements{};
            auto serialized = ot::Space{};
            {
                auto& [opcode, invalid, bytes, data] = elements.emplace_back();
                data = byte_array(ot::space(n - 1));
                serialized = ot::space(data->Bytes());
                opcode = static_cast<b::script::OP>(n);
                serialized.emplace(serialized.begin(), std::byte{n});
            }

            auto script1 = ot::factory::BitcoinScript(
                chain_, elements, Position::Output, {});
            auto script2 = ot::factory::BitcoinScript(
                chain_,
                ot::reader(serialized),
                Position::Output,
                false,
                false,
                {});

            EXPECT_FALSE(script1.IsValid());
            EXPECT_FALSE(script2.IsValid());
        }

        {
            auto elements = b::ScriptElements{};
            auto serialized = ot::Space{};
            {
                auto& [opcode, invalid, bytes, data] = elements.emplace_back();
                data = byte_array(ot::space(n + 1));
                serialized = ot::space(data->Bytes());
                opcode = static_cast<b::script::OP>(n);
                serialized.emplace(serialized.begin(), std::byte{n});
            }

            auto script1 = ot::factory::BitcoinScript(
                chain_, elements, Position::Output, {});
            auto script2 = ot::factory::BitcoinScript(
                chain_,
                ot::reader(serialized),
                Position::Output,
                false,
                false,
                {});

            EXPECT_FALSE(script1.IsValid());
            EXPECT_FALSE(script2.IsValid());
        }
    }
}

TEST(Test_BitcoinScript, opcodes_push_1)
{
    {
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{1}});
            data = byte_array(ot::space(1));
            serialized = ot::space(data->Bytes());
            opcode = PUSHDATA1;
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{76});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script1.IsValid());
        EXPECT_TRUE(script2.IsValid());

        {
            const auto e = script1.get();

            ASSERT_EQ(1, e.size());

            const auto& [opcode, invalid, bytes, data] = e[0];

            EXPECT_EQ(opcode, PUSHDATA1);
            ASSERT_TRUE(bytes.has_value());
            EXPECT_EQ(1, bytes.value().size());
            EXPECT_EQ(std::byte{1}, *bytes.value().cbegin());
            ASSERT_TRUE(data.has_value());
            EXPECT_EQ(1, data.value().size());
        }

        {
            const auto e = script2.get();

            ASSERT_EQ(1, e.size());

            const auto& [opcode, invalid, bytes, data] = e[0];

            EXPECT_EQ(opcode, PUSHDATA1);
            ASSERT_TRUE(bytes.has_value());
            EXPECT_EQ(1, bytes.value().size());
            EXPECT_EQ(std::byte{1}, *bytes.value().cbegin());
            ASSERT_TRUE(data.has_value());
            EXPECT_EQ(1, data.value().size());
        }
    }

    {  // No size byte
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            opcode = PUSHDATA1;
        }

        auto script =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});

        EXPECT_FALSE(script.IsValid());
    }

    {  // No bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{1}});
            opcode = PUSHDATA1;
            serialized.emplace(serialized.begin(), std::byte{76});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Too few bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{2}});
            data = byte_array(ot::space(1));
            serialized = ot::space(data->Bytes());
            opcode = PUSHDATA1;
            serialized.emplace(serialized.begin(), std::byte{2});
            serialized.emplace(serialized.begin(), std::byte{76});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Too many bytes
        auto elements = b::ScriptElements{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{2}});
            data = byte_array(ot::space(3));
            opcode = PUSHDATA1;
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});

        EXPECT_FALSE(script1.IsValid());
    }
}

TEST(Test_BitcoinScript, opcodes_push_2)
{
    {
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{1}, std::byte{1}});
            data = byte_array(ot::space(257));
            serialized = ot::space(data->Bytes());
            opcode = PUSHDATA2;
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{77});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script1.IsValid());
        EXPECT_TRUE(script2.IsValid());

        {
            const auto e = script1.get();

            ASSERT_EQ(1, e.size());

            const auto& [opcode, invalid, bytes, data] = e[0];

            EXPECT_EQ(opcode, PUSHDATA2);
            ASSERT_TRUE(bytes.has_value());
            EXPECT_EQ(2, bytes.value().size());
            EXPECT_EQ(std::byte{1}, bytes.value().at(0));
            EXPECT_EQ(std::byte{1}, bytes.value().at(1));
            ASSERT_TRUE(data.has_value());
            EXPECT_EQ(257, data.value().size());
        }

        {
            const auto e = script2.get();

            ASSERT_EQ(1, e.size());

            const auto& [opcode, invalid, bytes, data] = e[0];

            EXPECT_EQ(opcode, PUSHDATA2);
            ASSERT_TRUE(bytes.has_value());
            EXPECT_EQ(2, bytes.value().size());
            EXPECT_EQ(std::byte{1}, bytes.value().at(0));
            EXPECT_EQ(std::byte{1}, bytes.value().at(1));
            ASSERT_TRUE(data.has_value());
            EXPECT_EQ(257, data.value().size());
        }
    }

    {  // No size byte
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            opcode = PUSHDATA2;
            serialized.emplace(serialized.begin(), std::byte{77});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Insufficient size bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{1}});
            opcode = PUSHDATA2;
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{77});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // No bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{1}, std::byte{0}});
            opcode = PUSHDATA2;
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{77});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Too few bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{2}, std::byte{0}});
            data = byte_array(ot::space(1));
            serialized = ot::space(data->Bytes());
            opcode = PUSHDATA2;
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{2});
            serialized.emplace(serialized.begin(), std::byte{77});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Too many bytes
        auto elements = b::ScriptElements{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{std::byte{2}, std::byte{0}});
            data = byte_array(ot::space(3));
            opcode = PUSHDATA2;
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});

        EXPECT_FALSE(script1.IsValid());
    }
}

TEST(Test_BitcoinScript, opcodes_push_4)
{
    {
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{
                std::byte{1}, std::byte{1}, std::byte{1}, std::byte{1}});
            data = byte_array(ot::space(16843009));
            serialized = ot::space(data->Bytes());
            opcode = PUSHDATA4;
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{78});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script1.IsValid());
        EXPECT_TRUE(script2.IsValid());

        {
            const auto e = script1.get();

            ASSERT_EQ(1, e.size());

            const auto& [opcode, invalid, bytes, data] = e[0];

            EXPECT_EQ(opcode, PUSHDATA4);
            ASSERT_TRUE(bytes.has_value());
            EXPECT_EQ(4, bytes.value().size());
            EXPECT_EQ(std::byte{1}, bytes.value().at(0));
            EXPECT_EQ(std::byte{1}, bytes.value().at(1));
            EXPECT_EQ(std::byte{1}, bytes.value().at(2));
            EXPECT_EQ(std::byte{1}, bytes.value().at(3));
            ASSERT_TRUE(data.has_value());
            EXPECT_EQ(16843009, data.value().size());
        }

        {
            const auto e = script2.get();

            ASSERT_EQ(1, e.size());

            const auto& [opcode, invalid, bytes, data] = e[0];

            EXPECT_EQ(opcode, PUSHDATA4);
            ASSERT_TRUE(bytes.has_value());
            EXPECT_EQ(4, bytes.value().size());
            EXPECT_EQ(std::byte{1}, bytes.value().at(0));
            EXPECT_EQ(std::byte{1}, bytes.value().at(1));
            EXPECT_EQ(std::byte{1}, bytes.value().at(2));
            EXPECT_EQ(std::byte{1}, bytes.value().at(3));
            ASSERT_TRUE(data.has_value());
            EXPECT_EQ(16843009, data.value().size());
        }
    }

    {  // No size byte
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            opcode = PUSHDATA4;
            serialized.emplace(serialized.begin(), std::byte{78});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Insufficient size bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes =
                byte_array(ot::Space{std::byte{1}, std::byte{1}, std::byte{1}});
            opcode = PUSHDATA4;
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{78});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // No bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{
                std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0}});
            opcode = PUSHDATA4;
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{1});
            serialized.emplace(serialized.begin(), std::byte{78});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Too few bytes
        auto elements = b::ScriptElements{};
        auto serialized = ot::Space{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{
                std::byte{2}, std::byte{0}, std::byte{0}, std::byte{0}});
            data = byte_array(ot::space(1));
            serialized = ot::space(data->Bytes());
            opcode = PUSHDATA4;
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{0});
            serialized.emplace(serialized.begin(), std::byte{2});
            serialized.emplace(serialized.begin(), std::byte{78});
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});
        auto script2 = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_FALSE(script1.IsValid());
        EXPECT_FALSE(script2.IsValid());
    }

    {  // Too many bytes
        auto elements = b::ScriptElements{};
        {
            auto& [opcode, invalid, bytes, data] = elements.emplace_back();
            bytes = byte_array(ot::Space{
                std::byte{2}, std::byte{0}, std::byte{0}, std::byte{0}});
            data = byte_array(ot::space(3));
            opcode = PUSHDATA4;
        }

        auto script1 =
            ot::factory::BitcoinScript(chain_, elements, Position::Output, {});

        EXPECT_FALSE(script1.IsValid());
    }
}

TEST(Test_BitcoinScript, p2pk)
{
    for (const auto& serialized : p2pk_good_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});
        const auto elements = script.get();

        EXPECT_TRUE(script.IsValid());
        ASSERT_EQ(Pattern::PayToPubkey, script.Type());
        EXPECT_FALSE(script.M());
        EXPECT_FALSE(script.MultisigPubkey(0));
        EXPECT_FALSE(script.N());
        ASSERT_TRUE(script.Pubkey());

        if (65 == script.Pubkey().value().size()) {
            EXPECT_EQ(
                std::memcmp(
                    script.Pubkey().value().data(),
                    uncompressed_pubkey_1_.data(),
                    uncompressed_pubkey_1_.size()),
                0);
        } else if (33 == script.Pubkey().value().size()) {
            EXPECT_EQ(
                std::memcmp(
                    script.Pubkey().value().data(),
                    compressed_pubkey_1_.data(),
                    compressed_pubkey_1_.size()),
                0);
        } else {
            EXPECT_TRUE(false);
        }

        EXPECT_FALSE(script.PubkeyHash());
        EXPECT_EQ(Position::Output, script.Role());
        ASSERT_FALSE(script.ScriptHash());
        ASSERT_EQ(2, elements.size());
        EXPECT_FALSE(script.Value(0));

        {
            const auto& [opcode, invalid, bytes, data] = elements[0];

            ASSERT_TRUE(data);
            EXPECT_NE(0, data.value().size());
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[1];

            EXPECT_EQ(CHECKSIG, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }

    for (const auto& serialized : p2pk_bad_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script.IsValid());
        EXPECT_EQ(Pattern::Custom, script.Type());

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }
}

TEST(Test_BitcoinScript, p2pkh)
{
    for (const auto& serialized : p2pkh_good_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});
        const auto elements = script.get();

        EXPECT_TRUE(script.IsValid());
        ASSERT_EQ(Pattern::PayToPubkeyHash, script.Type());
        EXPECT_FALSE(script.M());
        EXPECT_FALSE(script.MultisigPubkey(0));
        EXPECT_FALSE(script.N());
        EXPECT_FALSE(script.Pubkey());
        ASSERT_TRUE(script.PubkeyHash());
        ASSERT_EQ(hash_160_.size(), script.PubkeyHash().value().size());
        EXPECT_EQ(
            std::memcmp(
                script.PubkeyHash().value().data(),
                hash_160_.data(),
                hash_160_.size()),
            0);
        EXPECT_EQ(Position::Output, script.Role());
        ASSERT_FALSE(script.ScriptHash());
        ASSERT_EQ(5, elements.size());
        EXPECT_FALSE(script.Value(0));

        {
            const auto& [opcode, invalid, bytes, data] = elements[0];

            EXPECT_EQ(DUP, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[1];

            EXPECT_EQ(HASH160, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[2];

            ASSERT_TRUE(data);
            EXPECT_EQ(hash_160_.size(), data.value().size());
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[3];

            EXPECT_EQ(EQUALVERIFY, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[4];

            EXPECT_EQ(CHECKSIG, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }

    for (const auto& serialized : p2pkh_bad_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script.IsValid());
        EXPECT_EQ(Pattern::Custom, script.Type());

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }
}

TEST(Test_BitcoinScript, p2sh)
{
    for (const auto& serialized : p2sh_good_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});
        const auto elements = script.get();

        EXPECT_TRUE(script.IsValid());
        ASSERT_EQ(Pattern::PayToScriptHash, script.Type());
        EXPECT_FALSE(script.M());
        EXPECT_FALSE(script.MultisigPubkey(0));
        EXPECT_FALSE(script.N());
        EXPECT_FALSE(script.Pubkey());
        EXPECT_FALSE(script.PubkeyHash());
        EXPECT_EQ(Position::Output, script.Role());
        ASSERT_TRUE(script.ScriptHash());
        ASSERT_EQ(hash_160_.size(), script.ScriptHash().value().size());
        EXPECT_EQ(
            std::memcmp(
                script.ScriptHash().value().data(),
                hash_160_.data(),
                hash_160_.size()),
            0);
        ASSERT_EQ(3, elements.size());
        EXPECT_FALSE(script.Value(0));

        {
            const auto& [opcode, invalid, bytes, data] = elements[0];

            EXPECT_EQ(HASH160, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[1];

            EXPECT_EQ(PUSHDATA_20, opcode);
            ASSERT_TRUE(data);
            EXPECT_EQ(hash_160_.size(), data.value().size());
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[2];

            EXPECT_EQ(EQUAL, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }

    for (const auto& serialized : p2sh_bad_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script.IsValid());
        EXPECT_EQ(Pattern::Custom, script.Type());

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }
}

TEST(Test_BitcoinScript, null_data)
{
    for (const auto& serialized : data_good_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});
        const auto elements = script.get();

        EXPECT_TRUE(script.IsValid());
        ASSERT_EQ(Pattern::NullData, script.Type());
        EXPECT_FALSE(script.M());
        EXPECT_FALSE(script.MultisigPubkey(0));
        EXPECT_FALSE(script.N());
        EXPECT_FALSE(script.Pubkey());
        EXPECT_FALSE(script.PubkeyHash());
        EXPECT_EQ(Position::Output, script.Role());
        EXPECT_FALSE(script.ScriptHash());
        ASSERT_EQ(3, elements.size());
        ASSERT_TRUE(script.Value(0));
        ASSERT_TRUE(script.Value(1));
        EXPECT_EQ(
            ot::UnallocatedCString{"hello"},
            ot::UnallocatedCString{script.Value(0).value()});
        EXPECT_EQ(
            ot::UnallocatedCString{"world!"},
            ot::UnallocatedCString{script.Value(1).value()});

        {
            const auto& [opcode, invalid, bytes, data] = elements[0];

            EXPECT_EQ(RETURN, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[1];

            EXPECT_EQ(PUSHDATA_5, opcode);
            ASSERT_TRUE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[2];

            EXPECT_EQ(PUSHDATA_6, opcode);
            ASSERT_TRUE(data);
        }

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }

    for (const auto& serialized : data_bad_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script.IsValid());
        EXPECT_EQ(Pattern::Custom, script.Type());

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }
}

TEST(Test_BitcoinScript, multisig)
{
    for (const auto& serialized : multisig_good_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});
        const auto elements = script.get();

        EXPECT_TRUE(script.IsValid());
        ASSERT_EQ(Pattern::PayToMultisig, script.Type());
        ASSERT_TRUE(script.M());
        EXPECT_EQ(2, script.M().value());
        ASSERT_TRUE(script.MultisigPubkey(0));
        ASSERT_EQ(
            uncompressed_pubkey_1_.size(),
            script.MultisigPubkey(0).value().size());
        EXPECT_EQ(
            std::memcmp(
                script.MultisigPubkey(0).value().data(),
                uncompressed_pubkey_1_.data(),
                uncompressed_pubkey_1_.size()),
            0);
        ASSERT_TRUE(script.MultisigPubkey(1));
        ASSERT_EQ(
            compressed_pubkey_1_.size(),
            script.MultisigPubkey(1).value().size());
        EXPECT_EQ(
            std::memcmp(
                script.MultisigPubkey(1).value().data(),
                compressed_pubkey_1_.data(),
                compressed_pubkey_1_.size()),
            0);
        ASSERT_TRUE(script.MultisigPubkey(2));
        ASSERT_EQ(
            compressed_pubkey_2_.size(),
            script.MultisigPubkey(2).value().size());
        EXPECT_EQ(
            std::memcmp(
                script.MultisigPubkey(2).value().data(),
                compressed_pubkey_2_.data(),
                compressed_pubkey_2_.size()),
            0);
        ASSERT_TRUE(script.N());
        EXPECT_EQ(3, script.N().value());
        EXPECT_FALSE(script.Pubkey());
        EXPECT_FALSE(script.PubkeyHash());
        EXPECT_EQ(Position::Output, script.Role());
        EXPECT_FALSE(script.ScriptHash());
        ASSERT_EQ(6, elements.size());
        EXPECT_FALSE(script.Value(0));

        {
            const auto& [opcode, invalid, bytes, data] = elements[0];

            EXPECT_EQ(TWO, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[1];

            ASSERT_TRUE(data);
            EXPECT_EQ(uncompressed_pubkey_1_.size(), data.value().size());
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[2];

            ASSERT_TRUE(data);
            EXPECT_EQ(compressed_pubkey_1_.size(), data.value().size());
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[3];

            ASSERT_TRUE(data);
            EXPECT_EQ(compressed_pubkey_2_.size(), data.value().size());
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[4];

            EXPECT_EQ(THREE, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        {
            const auto& [opcode, invalid, bytes, data] = elements[5];

            EXPECT_EQ(CHECKMULTISIG, opcode);
            EXPECT_FALSE(bytes);
            EXPECT_FALSE(data);
        }

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }

    for (const auto& serialized : multisig_bad_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script.IsValid());
        EXPECT_EQ(Pattern::Custom, script.Type());

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }

    for (const auto& serialized : multisig_malformed_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Output, false, false, {});

        EXPECT_TRUE(script.IsValid());
        EXPECT_EQ(Pattern::Malformed, script.Type());

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }
}

TEST(Test_BitcoinScript, input)
{
    for (const auto& serialized : input_good_) {
        const auto script = ot::factory::BitcoinScript(
            chain_, ot::reader(serialized), Position::Input, true, false, {});
        const auto elements = script.get();

        EXPECT_TRUE(script.IsValid());
        EXPECT_EQ(Pattern::Input, script.Type());
        ASSERT_EQ(2, elements.size());

        const auto& sig = elements[0];

        ASSERT_TRUE(sig.data_.has_value());
        ASSERT_EQ(sig.data_->size(), sig_1_.size());
        EXPECT_EQ(
            std::memcmp(sig.data_->data(), sig_1_.data(), sig_1_.size()), 0);

        const auto& key = elements[1];

        ASSERT_TRUE(key.data_.has_value());
        ASSERT_EQ(key.data_->size(), key_1_.size());
        EXPECT_EQ(
            std::memcmp(key.data_->data(), key_1_.data(), key_1_.size()), 0);

        auto bytes = ot::Space{};

        EXPECT_TRUE(script.Serialize(ot::writer(bytes)));
        ASSERT_EQ(bytes.size(), serialized.size());
        EXPECT_EQ(
            std::memcmp(bytes.data(), serialized.data(), serialized.size()), 0);
    }
}
}  // namespace ottest
