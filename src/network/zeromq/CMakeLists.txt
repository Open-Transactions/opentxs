# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-network-zeromq)

add_subdirectory(zap)

set(cxx-sources
  curve/Client.cpp
  curve/Server.cpp
  socket/Bidirectional.cpp
  socket/Dealer.cpp
  socket/Pair.cpp
  socket/Publish.cpp
  socket/Pull.cpp
  socket/Push.cpp
  socket/Reply.cpp
  socket/Request.cpp
  socket/Router.cpp
  socket/Sender.cpp
  socket/Socket.cpp
  socket/Subscribe.cpp
  Context.cpp
  Frame.cpp
  FrameIterator.cpp
  FrameSection.cpp
  ListenCallback.cpp
  ListenCallbackSwig.cpp
  Message.cpp
  PairEventCallback.cpp
  PairEventCallbackSwig.cpp
  PairEventListener.cpp
  Proxy.cpp
  ReplyCallback.cpp
)

file(GLOB cxx-install-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/network/zeromq/*.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  curve/Client.hpp
  curve/Server.hpp
  socket/Bidirectional.hpp
  socket/Dealer.hpp
  socket/Pair.hpp
  socket/Publish.hpp
  socket/Pull.hpp
  socket/Push.hpp
  socket/Receiver.hpp
  socket/Receiver.tpp
  socket/Reply.hpp
  socket/Request.hpp
  socket/Router.hpp
  socket/Send.hpp
  socket/Sender.hpp
  socket/Socket.hpp
  socket/Subscribe.hpp
  Context.hpp
  Frame.hpp
  ListenCallback.hpp
  ListenCallbackSwig.hpp
  PairEventCallback.hpp
  PairEventCallbackSwig.hpp
  PairEventListener.hpp
  Proxy.hpp
  ReplyCallback.hpp
)

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/network/zeromq
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
target_include_directories(${MODULE_NAME} PRIVATE ${OT_ZEROMQ_HEADERS})
