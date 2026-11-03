set(BASE_TRIPLET_FILE ${CMAKE_CURRENT_LIST_DIR}/../vendor/github.com/carbonengine/vcpkg-registry/triplets/x64-linux-internal.cmake)

include(${BASE_TRIPLET_FILE})

list(APPEND VCPKG_HASH_ADDITIONAL_FILES ${BASE_TRIPLET_FILE})
