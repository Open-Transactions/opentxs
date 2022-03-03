// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/node/FilterOracle.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"

namespace opentxs::blockchain::node::implementation
{
const FilterOracle::CheckpointMap FilterOracle::filter_checkpoints_{
    {blockchain::Type::Bitcoin,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c2"
                "02"}},
              {filter::Type::ES,
               {"fad52acc389a391c1d6d94e8984fe77323fbda24fb31299b88635d7bee0278"
                "e8"}},
          }},
     }},
    {blockchain::Type::Bitcoin_testnet3,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb7794558"
                "21"}},
              {filter::Type::ES,
               {"995cfe5d055c9158c5a388b71fb2ddbe292c9ca2d30dca91359d8cbbe4603e"
                "02"}},
          }},
     }},
    {blockchain::Type::BitcoinCash,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c2"
                "02"}},
              {filter::Type::ES,
               {"fad52acc389a391c1d6d94e8984fe77323fbda24fb31299b88635d7bee0278"
                "e8"}},
          }},
     }},
    {blockchain::Type::BitcoinCash_testnet3,
     {
         {0,
          {
              {filter::Type::Basic_BCHVariant,
               {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb7794558"
                "21"}},
              {filter::Type::ES,
               {"995cfe5d055c9158c5a388b71fb2ddbe292c9ca2d30dca91359d8cbbe4603e"
                "02"}},
          }},
     }},
    {blockchain::Type::Litecoin,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"8aa75530308cf8247a151c37c24e7aaa281ae3b5cecedb581aacb3a0d07c24"
                "51"}},
              {filter::Type::ES,
               {"23b8dae37cf04c8a278bd50bcbcf23a03051ea902f67c4760eb35be96d4283"
                "20"}},
          }},
     }},
    {blockchain::Type::Litecoin_testnet4,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"02d023da9d271b849f717089aad7e03a515dac982c9fb2cfd952e2ce1c6187"
                "92"}},
              {filter::Type::ES,
               {"ad242bb97aaf6a8f973dc2054d5356a4fcc87f575b29bbb3e0d953cfaedff8"
                "c6"}},
          }},
     }},
    {blockchain::Type::PKT,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd3"
                "6c"}},
              {filter::Type::ES,
               {"155e1700eff3f9019ba1716316295a8753ec44d2a7730eee1c1c73e2b511e1"
                "34"}},
          }},
     }},
    {blockchain::Type::PKT_testnet,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"526b0656def40fcb65ef87a75337001fae57a1d17dc17e103fb536cfddedd3"
                "6c"}},
              {filter::Type::ES,
               {"155e1700eff3f9019ba1716316295a8753ec44d2a7730eee1c1c73e2b511e1"
                "34"}},
          }},
     }},
    {blockchain::Type::UnitTest,
     {
         {0,
          {
              {filter::Type::Basic_BIP158,
               {"2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e"
                "48"}},
              {filter::Type::Basic_BCHVariant,
               {"2b5adc66021d5c775f630efd91518cf6ce3e9f525bbf54d9f0d709451e305e"
                "48"}},
              {filter::Type::ES,
               {"5e0aa302450f931bc2e4fab27632231a06964277ea8dfcdd93c19149a24fe7"
                "88"}},
          }},
     }},
};
}  // namespace opentxs::blockchain::node::implementation
