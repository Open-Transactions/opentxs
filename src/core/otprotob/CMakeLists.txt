# Copyright (c) Monetas AG, 2014

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (NOT WIN32)
  set(CMAKE_CXX_FLAGS "-Wno-shadow -Wno-error")
endif()

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER Generics.proto Bitcoin.proto Markets.proto Moneychanger.proto)

if (WIN32)
  # suppress std::_Copy_impl being unsafe warnings 
  add_definitions("/wd4996")
endif()

include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIR})

add_library(otprotob
  STATIC
  ${PROTO_SRC}
  ${PROTO_HEADER}
)

