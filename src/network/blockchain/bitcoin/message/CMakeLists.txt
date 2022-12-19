# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(addr)
  add_subdirectory(addr2)
  add_subdirectory(base)
  add_subdirectory(block)
  add_subdirectory(cfcheckpt)
  add_subdirectory(cfheaders)
  add_subdirectory(cfilter)
  add_subdirectory(getaddr)
  add_subdirectory(getblocks)
  add_subdirectory(getcfcheckpt)
  add_subdirectory(getcfheaders)
  add_subdirectory(getcfilters)
  add_subdirectory(getdata)
  add_subdirectory(getheaders)
  add_subdirectory(headers)
  add_subdirectory(inv)
  add_subdirectory(mempool)
  add_subdirectory(notfound)
  add_subdirectory(ping)
  add_subdirectory(pong)
  add_subdirectory(reject)
  add_subdirectory(sendaddr2)
  add_subdirectory(tx)
  add_subdirectory(verack)
  add_subdirectory(version)

  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/bitcoin/message/Factory.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/bitcoin/message/Types.hpp"
      "Common.cpp"
  )
endif()
