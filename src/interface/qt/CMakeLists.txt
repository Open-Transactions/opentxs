# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_QT_EXPORT)
  add_subdirectory(displayscale)
  add_subdirectory(seedvalidator)
  add_subdirectory(identitymanager)
endif()

add_subdirectory(qml)

if(OT_QT_EXPORT)
  libopentxs_add_public_header_qt("AccountActivity.hpp")
  libopentxs_add_public_header_qt("AccountList.hpp")
  libopentxs_add_public_header_qt("AccountSummary.hpp")
  libopentxs_add_public_header_qt("AccountTree.hpp")
  libopentxs_add_public_header_qt("ActivitySummary.hpp")
  libopentxs_add_public_header_qt("AmountValidator.hpp")
  libopentxs_add_public_header_qt("BlankModel.hpp")
  libopentxs_add_public_header_qt("BlockchainAccountStatus.hpp")
  libopentxs_add_public_header_qt("BlockchainSelection.hpp")
  libopentxs_add_public_header_qt("BlockchainStatistics.hpp")
  libopentxs_add_public_header_qt("Contact.hpp")
  libopentxs_add_public_header_qt("ContactActivity.hpp")
  libopentxs_add_public_header_qt("ContactActivityFilterable.hpp")
  libopentxs_add_public_header_qt("ContactList.hpp")
  libopentxs_add_public_header_qt("DestinationValidator.hpp")
  libopentxs_add_public_header_qt("MessagableList.hpp")
  libopentxs_add_public_header_qt("Model.hpp")
  libopentxs_add_public_header_qt("NymList.hpp")
  libopentxs_add_public_header_qt("PayableList.hpp")
  libopentxs_add_public_header_qt("Profile.hpp")
  libopentxs_add_public_header_qt("SeedList.hpp")
  libopentxs_add_public_header_qt("SeedTree.hpp")
  libopentxs_add_public_header_qt("UnitList.hpp")

  list(
    APPEND
    OPENTXS_QT_PRIVATE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/DraftValidator.hpp"
  )
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/interface/qt/Factory.hpp"
      "AccountActivity.cpp"
      "AccountList.cpp"
      "AccountSummary.cpp"
      "AccountTree.cpp"
      "ActivitySummary.cpp"
      "AmountValidator.cpp"
      "AmountValidator.hpp"
      "BlockchainAccountStatus.cpp"
      "BlockchainDestinationValidator.cpp"
      "BlockchainSelection.cpp"
      "BlockchainStatistics.cpp"
      "Contact.cpp"
      "ContactActivity.cpp"
      "ContactActivityFilterable.cpp"
      "ContactList.cpp"
      "CustodialDestinationValidator.cpp"
      "DestinationValidator.cpp"
      "DestinationValidator.hpp"
      "DraftValidator.cpp"
      "MessagableList.cpp"
      "Model.cpp"
      "NymList.cpp"
      "PayableList.cpp"
      "Profile.cpp"
      "SeedList.cpp"
      "SeedTree.cpp"
      "SendMonitor.cpp"
      "SendMonitor.hpp"
      "UnitList.cpp"
      ${MOC_SRCS}
  )

  if(OT_BLOCKCHAIN_EXPORT)
    target_sources(opentxs-common PRIVATE "Blockchain.cpp")
  endif()

  target_link_libraries(opentxs-moc PRIVATE Qt::Core Qt::Gui)
  libopentxs_link_internal(Qt::Core)
  libopentxs_link_internal(Qt::Gui)
  libopentxs_link_external(Qt::Core)
  libopentxs_link_external(Qt::Gui)
else()
  target_sources(opentxs-common PRIVATE "no-Qt.cpp")
endif()

set(OPENTXS_QT_PUBLIC_HEADERS
    ${OPENTXS_QT_PUBLIC_HEADERS}
    PARENT_SCOPE
)
set(OPENTXS_QT_PRIVATE_HEADERS
    ${OPENTXS_QT_PRIVATE_HEADERS}
    PARENT_SCOPE
)
