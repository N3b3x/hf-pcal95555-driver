
#===============================================================================
# PCAL95555 Driver - Build Settings
# Shared variables for target name, includes, sources, and dependencies.
# This file is the SINGLE SOURCE OF TRUTH for the driver version.
#===============================================================================

include_guard(GLOBAL)

# Target name
set(HF_PCAL95555_TARGET_NAME "hf_pcal95555")

#===============================================================================
# Versioning (single source of truth)
#===============================================================================
set(HF_PCAL95555_VERSION_MAJOR 1)
set(HF_PCAL95555_VERSION_MINOR 0)
set(HF_PCAL95555_VERSION_PATCH 0)
set(HF_PCAL95555_VERSION "${HF_PCAL95555_VERSION_MAJOR}.${HF_PCAL95555_VERSION_MINOR}.${HF_PCAL95555_VERSION_PATCH}")

#===============================================================================
# Generate version header from template (into build directory)
#===============================================================================
# The generated header is placed in CMAKE_CURRENT_BINARY_DIR to keep the
# source tree clean.  This eliminates the need for a .gitignore entry and
# allows parallel builds with different version stamps.
set(HF_PCAL95555_VERSION_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/../inc/pcal95555_version.h.in")
set(HF_PCAL95555_VERSION_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/hf_pcal95555_generated")
set(HF_PCAL95555_VERSION_HEADER     "${HF_PCAL95555_VERSION_HEADER_DIR}/pcal95555_version.h")

# Ensure the output directory exists before configure_file and before any
# consumer (e.g. ESP-IDF idf_component_register) validates include paths.
file(MAKE_DIRECTORY "${HF_PCAL95555_VERSION_HEADER_DIR}")

if(EXISTS "${HF_PCAL95555_VERSION_TEMPLATE}")
    configure_file(
        "${HF_PCAL95555_VERSION_TEMPLATE}"
        "${HF_PCAL95555_VERSION_HEADER}"
        @ONLY
    )
    message(STATUS "PCAL95555 driver v${HF_PCAL95555_VERSION} — generated pcal95555_version.h in ${HF_PCAL95555_VERSION_HEADER_DIR}")
else()
    message(WARNING "pcal95555_version.h.in not found at ${HF_PCAL95555_VERSION_TEMPLATE}")
endif()

#===============================================================================
# Public include directories
#===============================================================================
# Two include directories:
#   1. Source tree inc/ — hand-written headers (pcal95555.hpp, etc.)
#   2. Build tree generated dir — configure_file outputs (pcal95555_version.h)
set(HF_PCAL95555_PUBLIC_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../inc"
    "${HF_PCAL95555_VERSION_HEADER_DIR}"
)

#===============================================================================
# Source files (empty for header-only)
#===============================================================================
set(HF_PCAL95555_SOURCE_FILES)

#===============================================================================
# ESP-IDF component dependencies
#===============================================================================
set(HF_PCAL95555_IDF_REQUIRES driver)
