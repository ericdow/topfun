# build the geometry library
set(SOURCES
  BoundingBox.cpp
  BoundingFrustum.cpp
)

add_library(geometry STATIC ${SOURCES})
