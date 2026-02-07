cmake_minimum_required(VERSION 3.21)

project(shared_resources VERSION 0.1.0 LANGUAGES CXX)

option(ENABLE_TESTING_SHARED_RESOURCES "Enable testing" OFF)
option(BUILD_SHARED_RESOURCES_DOCS "Build documentation" OFF)

if (ENABLE_TESTING_SHARED_RESOURCES)
    enable_testing()
    add_subdirectory(tests)
endif()

if (BUILD_SHARED_RESOURCES_DOCS)
    add_subdirectory(docs)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(shared_resources INTERFACE)
target_include_directories(shared_resources
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# C++ standard
target_compile_features(shared_resources INTERFACE cxx_std_20)

#Export + Install rules
install(TARGETS shared_resources
    EXPORT shared_resourcesTargets
)

# Install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT shared_resourcesTargets
    FILE shared_resourcesTargets.cmake
    NAMESPACE shared_resources::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/shared_resources
)

# Generate config files
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/shared_resourcesConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/shared_resourcesConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/shared_resources
)

# Generate version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/shared_resourcesConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install config and version files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/shared_resourcesConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/shared_resourcesConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/shared_resources
)

# Export build-tree package config
export(EXPORT shared_resourcesTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/shared_resourcesTargets.cmake"
    NAMESPACE shared_resources::
)

#Add the build tree as a package registry entry
set(SHARED_RESOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}")
