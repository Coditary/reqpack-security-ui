# Resolves reqpack-security-core: explicit REQPACK_SECURITY_CORE_DIR, monorepo sibling, or FetchContent.

set(REQPACK_SECURITY_CORE_REPO
    "https://github.com/Coditary/reqpack-security-core.git"
    CACHE STRING "reqpack-security-core Git repository"
)

set(REQPACK_SECURITY_CORE_TAG
    "main"
    CACHE STRING "Git branch, tag, or commit for reqpack-security-core"
)

set(REQPACK_SECURITY_CORE_DIR
    ""
    CACHE PATH "Local reqpack-security-core checkout (skips Git fetch when set)"
)

set(_reqpack_security_core_local "${CMAKE_CURRENT_SOURCE_DIR}/../reqpack-security-core")

if(REQPACK_SECURITY_CORE_DIR AND EXISTS "${REQPACK_SECURITY_CORE_DIR}/CMakeLists.txt")
    set(REQPACK_SECURITY_CORE_SOURCE_DIR "${REQPACK_SECURITY_CORE_DIR}")
    message(STATUS "reqpack-security-ui: using REQPACK_SECURITY_CORE_DIR at ${REQPACK_SECURITY_CORE_SOURCE_DIR}")
elseif(EXISTS "${_reqpack_security_core_local}/CMakeLists.txt")
    set(REQPACK_SECURITY_CORE_SOURCE_DIR "${_reqpack_security_core_local}")
    message(STATUS "reqpack-security-ui: using sibling reqpack-security-core at ${REQPACK_SECURITY_CORE_SOURCE_DIR}")
else()
    message(STATUS "reqpack-security-ui: fetching reqpack-security-core from ${REQPACK_SECURITY_CORE_REPO} @ ${REQPACK_SECURITY_CORE_TAG}")
    FetchContent_Declare(
        reqpack_security_core
        GIT_REPOSITORY ${REQPACK_SECURITY_CORE_REPO}
        GIT_TAG ${REQPACK_SECURITY_CORE_TAG}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(reqpack_security_core)
    set(REQPACK_SECURITY_CORE_SOURCE_DIR "${reqpack_security_core_SOURCE_DIR}")
endif()

if(NOT TARGET reqpack_security_core)
    add_subdirectory(
        "${REQPACK_SECURITY_CORE_SOURCE_DIR}"
        "${CMAKE_BINARY_DIR}/_deps/reqpack-security-core-build"
        EXCLUDE_FROM_ALL
    )
endif()
