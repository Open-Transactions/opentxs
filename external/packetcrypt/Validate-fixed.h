// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <packetcrypt/PacketCrypt.h>

int Validate_checkBlock_fixed(
    const PacketCrypt_HeaderAndProof_t* hap,
    uint32_t hapLen,
    uint32_t blockHeight,
    const PacketCrypt_Coinbase_t* coinbaseCommitment,
    const uint8_t blockHashes[],
    PacketCrypt_ValidateCtx_t* vctx);
