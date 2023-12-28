# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(
  libopentxs_add_enum_header
  install_path
  enum_name
)
  include(libopentxs-add-public-header)
  include(libopentxs-add-public-moc-header)

  if(OT_QT_EXPORT)
    set(MAYBE_INCLUDE_QOBJECT "#include <QMetaObject>\n")
    set(MAYBE_OPEN_NAMESPACE
        "{\nQ_NAMESPACE_EXPORT(OPENTXS_EXPORT)\nQ_CLASSINFO(\"RegisterEnumClassesUnscoped\", \"true\")\n"
    )
    set(MAYBE_EXPORT_ENUM "};\nQ_ENUM_NS(${enum_name})\n")
  else()
    set(MAYBE_INCLUDE_QOBJECT "")
    set(MAYBE_OPEN_NAMESPACE "{\n")
    set(MAYBE_EXPORT_ENUM "};\n")
  endif()

  configure_file(
    "${opentxs_SOURCE_DIR}/include/opentxs/${install_path}/${enum_name}.hpp.in"
    "${opentxs_BINARY_DIR}/include/opentxs/${install_path}/${enum_name}.hpp"
    @ONLY
  )

  libopentxs_add_public_header("${install_path}" "${enum_name}.hpp")

  if(OT_QT_EXPORT)
    libopentxs_add_public_moc_header(
      "${opentxs_BINARY_DIR}/include/opentxs/${install_path}/${enum_name}.hpp"
    )
  endif()
endmacro()
