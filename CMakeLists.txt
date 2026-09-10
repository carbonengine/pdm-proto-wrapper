cmake_minimum_required(VERSION 3.19)
project(platform_pdm_proto_wrapper VERSION 2.0.0)

find_package(carbon-pdm CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)

OPTION(BUILD_SHARED_LIBS "build using shared libraries" OFF)

add_library(platform_pdm_proto_wrapper
        generated/eve_public/app/platform.pb.cc
        generated/eve_launcher/pdm.pb.cc
        include/pdm/protobuf.h
        src/protobuf_template.h
        src/protobuf_public.cpp
        src/protobuf_launcher.cpp
        src/semver.h
        src/semver.cpp
)

find_program(PROTOC_EXE protoc)

add_custom_command(OUTPUT
        generated/eve_launcher/pdm.pb.cc
        generated/eve_launcher/pdm.pb.h
        generated/eve_public/app/platform.pb.cc
        generated/eve_public/app/platform.pb.h
        COMMAND ${PROTOC_EXE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protos/eve-proto/4d8a22b2b9eadfbec8fad6e44986feab33dd4951/eve_launcher --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/generated/eve_launcher pdm.proto
        COMMAND ${PROTOC_EXE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protos/eve-proto/4d8a22b2b9eadfbec8fad6e44986feab33dd4951/eve_public/app --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/generated/eve_public/app platform.proto
        COMMENT "Generating protos for pdm.proto and platform.proto from eve-proto ref 4d8a22b2b9eadfbec8fad6e44986feab33dd4951"
)

target_link_libraries(platform_pdm_proto_wrapper PUBLIC pdm protobuf::libprotobuf)
get_target_property(PDM_INTERFACE_INCLUDE_DIRECTORIES pdm INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(platform_pdm_proto_wrapper
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
)

option(USE_WITH_PROTOBUF_DLL "Is this library going to be used with protobuf as DLL?" OFF)

if(USE_WITH_PROTOBUF_DLL)
    target_compile_definitions(platform_pdm_proto_wrapper PUBLIC PROTOBUF_USE_DLLS=1)
endif()

set_property(TARGET platform_pdm_proto_wrapper PROPERTY CXX_STANDARD 17)
set_property(TARGET platform_pdm_proto_wrapper PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET platform_pdm_proto_wrapper PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET platform_pdm_proto_wrapper PROPERTY VISIBILITY_INLINES_HIDDEN YES)


# Install rules
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTING "Build with tests" ON)

    if(BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()

    set(PDM_WITH_PROTO_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}pdm_with_proto${CMAKE_STATIC_LIBRARY_SUFFIX}")

    if (NOT BUILD_SHARED_LIBS)
        option(COMBINE_STATIC_LIBS "Combine static libs into one lib if protobuf is static" OFF)

        if(COMBINE_STATIC_LIBS)

            if (WIN32)
                # CMAKE_AR should actually be the path to lib.exe on Windows.
                # However, as this cache var is marked as INTERNAL we don't want to rely on that always being true
                # Therefore, we use is as a HINT input to find_program.
                cmake_path(GET CMAKE_AR PARENT_PATH LIB_PATH_HINT)
                find_program(LIB_TOOL_EXE NAMES lib lib.exe REQUIRED HINTS ${LIB_PATH_HINT})

                #requires lib.exe to be available msvc
                add_custom_command(TARGET platform_pdm_proto_wrapper POST_BUILD
                        COMMAND ${LIB_TOOL_EXE} /OUT:$<TARGET_FILE_DIR:platform_pdm_proto_wrapper>/${PDM_WITH_PROTO_NAME}
                        $<TARGET_FILE:platform_pdm_proto_wrapper>
                        $<TARGET_FILE:pdm>
                        $<TARGET_FILE:protobuf::libprotobuf>
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                )
            elseif (APPLE)
                find_program(LIBTOOL_TOOL_EXE libtool REQUIRED)
                add_custom_command(TARGET platform_pdm_proto_wrapper POST_BUILD
                        COMMAND ${LIBTOOL_TOOL_EXE} -static -o $<TARGET_FILE_DIR:platform_pdm_proto_wrapper>/${PDM_WITH_PROTO_NAME}
                        $<TARGET_FILE:platform_pdm_proto_wrapper>
                        $<TARGET_FILE:pdm>
                        $<TARGET_FILE:protobuf::libprotobuf>
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                )
            else()
                message(FATAL_ERROR "Post build step to combine libs doesn't support current platform")
            endif()
        endif()
    endif ()

    option(INSTALL_ARCHIVE "Generate an archive of all build files on install (including static dependencies), This is useful for some legacy usage that exists on CI" OFF)
    if (INSTALL_ARCHIVE)

        set(INSTALL_LIBS
                $<TARGET_FILE:platform_pdm_proto_wrapper>
                $<TARGET_FILE:pdm>
                $<TARGET_FILE:protobuf::libprotobuf>
        )

        get_target_property(PDM_INCLUDE_DIRS pdm INTERFACE_INCLUDE_DIRECTORIES)
        get_target_property(PROTO_INCLUDE_DIRS protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)

        file(GLOB PDM_INCLUDE_FILES "${PDM_INCLUDE_DIRS}/*.h")
        file(GLOB PROTO_INCLUDE_FILES "${PROTO_INCLUDE_DIRS}/*.h")

        if (COMBINE_STATIC_LIBS)
            list(APPEND install_libs $<TARGET_FILE_DIR:platform_pdm_proto_wrapper>/${PDM_WITH_PROTO_NAME})
        endif ()

        install(
                FILES ${INSTALL_LIBS}
                DESTINATION lib/
        )
        install(DIRECTORY include/ DESTINATION include)
        install(FILES ${PDM_INCLUDE_FILES} DESTINATION include)
        install(FILES ${PROTO_INCLUDE_FILES} DESTINATION include)

    else ()
        install(
                TARGETS platform_pdm_proto_wrapper
                EXPORT platform_pdm_proto_wrapperTargets
                CONFIGURATIONS Release
                ARCHIVE DESTINATION lib/
                LIBRARY DESTINATION lib/
        )
        # Install rule for available public headers
        install(DIRECTORY include/ DESTINATION include)

        install(
                EXPORT platform_pdm_proto_wrapperTargets
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-pdmprotowrapper
                FILE carbon-pdmprotowrapper.cmake
        )

        install(
                FILES carbon-pdmprotowrapperConfig.cmake
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-pdmprotowrapper
        )
    endif()
endif()
