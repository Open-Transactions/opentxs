// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <string.h>

#include "Validate-fixed.h"
#include "packetcrypt/Validate.h"

int Validate_checkBlock_fixed(
    const PacketCrypt_HeaderAndProof_t* hap,
    uint32_t hapLen,
    uint32_t blockHeight,
    const PacketCrypt_Coinbase_t* coinbaseCommitment,
    const uint8_t blockHashes[],
    PacketCrypt_ValidateCtx_t* vctx)
{
    uint32_t shareTarget = 0;
    uint8_t workHashOut[32] = {0};

    return Validate_checkBlock(
        hap,
        hapLen,
        blockHeight,
        shareTarget,
        coinbaseCommitment,
        blockHashes,
        workHashOut,
        vctx);
}
