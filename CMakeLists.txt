cmake_minimum_required(VERSION 3.15)

project(reqpack-security-ui
    VERSION 0.2.0
    DESCRIPTION "ReqPack Security CLI (rqps) and optional HTTP API (rqps-ui)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RQPS_BUILD_CLI "Build rqps command-line tool" ON)
option(RQPS_BUILD_HTTP "Build rqps_http (REST server + remote ping); required for rqps-ui" ON)
option(RQPS_BUILD_UI "Build rqps-ui HTTP server binary" ON)
option(RQPS_BUILD_TESTS "Build rqps unit tests (Catch2)" ON)

include(FetchContent)
include(cmake/FetchSecurityCore.cmake)

set(_rqp_security_config_h "${REQPACK_SECURITY_CORE_SOURCE_DIR}/include/rqp/security/security_config.h")
if(EXISTS "${_rqp_security_config_h}")
    file(READ "${_rqp_security_config_h}" _rqp_security_config_header)
    if(_rqp_security_config_header MATCHES "auditFailOnFindings")
        set(RQPS_HAVE_AUDIT_FAIL_ON_FINDINGS 1)
    endif()
    if(_rqp_security_config_header MATCHES "std::string preset")
        set(RQPS_HAVE_SECURITY_PRESET 1)
    endif()
endif()

if(EXISTS "${REQPACK_SECURITY_CORE_SOURCE_DIR}/include/rqp/security/exchange.h")
    message(STATUS "reqpack-security-ui: exchange pipeline available in security-core")
    set(RQPS_HAVE_EXCHANGE 1)
endif()

# Local scan SDK: application core + adapters — no HTTP, no terminal UI.
add_library(rqps_core STATIC
    src/adapters/security_core/core_scanner_adapter.cpp
    src/adapters/system/os_process_runner.cpp
    src/adapters/system/path_guard.cpp
    src/application/audit_workflow.cpp
    src/application/doctor_workflow.cpp
    src/application/policy_evaluator.cpp
    src/application/scan_workflow.cpp
    src/application/service_registry.cpp
    src/db_service.cpp
    src/inventory_json.cpp
    src/local_status.cpp
    src/rqps_config.cpp
    src/sbom_service.cpp
    src/scan_service.cpp
    src/syft_discover_service.cpp
)

add_library(ReqPack::rqps_core ALIAS rqps_core)

target_include_directories(rqps_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(rqps_core
    PUBLIC
        reqpack_security_core
)

if(RQPS_HAVE_AUDIT_FAIL_ON_FINDINGS)
    target_compile_definitions(rqps_core PUBLIC RQPS_UI_HAVE_AUDIT_FAIL_ON_FINDINGS=1)
endif()
if(RQPS_HAVE_SECURITY_PRESET)
    target_compile_definitions(rqps_core PUBLIC RQPS_UI_HAVE_SECURITY_PRESET=1)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rqps_core PRIVATE -Wall -Wextra)
endif()

if(RQPS_BUILD_HTTP)
    include(FetchContent)
    FetchContent_Declare(
        cpp-httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.15.3
    )
    FetchContent_MakeAvailable(cpp-httplib)

    add_library(rqps_http STATIC
        src/http/http_error_mapping.cpp
        src/http/http_util.cpp
        src/http/routes/discover_routes.cpp
        src/http/routes/health_routes.cpp
        src/http/routes/register_routes.cpp
        src/http/routes/scan_routes.cpp
        src/http/routes/status_routes.cpp
        src/http/serve_runner.cpp
        src/remote_config.cpp
    )

    add_library(ReqPack::rqps_http ALIAS rqps_http)

    target_link_libraries(rqps_http
        PUBLIC
            rqps_core
            httplib::httplib
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rqps_http PRIVATE -Wall -Wextra)
    endif()
endif()

if(RQPS_BUILD_CLI)
    add_executable(rqps
        src/cli/cli_context.cpp
        src/cli/commands/db_command.cpp
        src/cli/commands/discover_command.cpp
        src/cli/commands/doctor_command.cpp
        src/cli/commands/misc_commands.cpp
        src/cli/commands/sbom_command.cpp
        src/cli/commands/scan_command.cpp
        src/cli/commands/test_command.cpp
        src/cli/output_renderers.cpp
        src/cli/scan_output.cpp
        src/cli_util.cpp
        src/rqps_cli.cpp
        src/rqps_main.cpp
    )

    target_compile_definitions(rqps PRIVATE RQPS_VERSION="${PROJECT_VERSION}")
    target_link_libraries(rqps PRIVATE rqps_core)

    if(RQPS_BUILD_HTTP)
        target_link_libraries(rqps PRIVATE rqps_http)
        target_compile_definitions(rqps PRIVATE RQPS_HAVE_HTTP=1)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rqps PRIVATE -Wall -Wextra)
    endif()

    install(TARGETS rqps RUNTIME DESTINATION bin)
endif()

if(RQPS_BUILD_UI)
    if(NOT RQPS_BUILD_HTTP)
        message(FATAL_ERROR "rqps-ui requires RQPS_BUILD_HTTP=ON")
    endif()

    add_executable(rqps-ui
        src/rqps_ui_main.cpp
    )

    target_link_libraries(rqps-ui PRIVATE rqps_http)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rqps-ui PRIVATE -Wall -Wextra)
    endif()

    install(TARGETS rqps-ui RUNTIME DESTINATION bin)
    install(FILES configs/rqps-ui.example.json DESTINATION share/rqps RENAME config.example.json)
endif()

if(RQPS_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.10
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(rqps_unit_tests
        tests/cpp/test_main.cpp
        tests/unit/cli_util_tests.cpp
        tests/unit/inventory_json_tests.cpp
        tests/unit/json_util_tests.cpp
        tests/unit/path_guard_tests.cpp
        tests/unit/domain_error_tests.cpp
        tests/unit/rqps_config_tests.cpp
        tests/unit/syft_discover_tests.cpp
        src/cli_util.cpp
    )

    target_include_directories(rqps_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/support
        ${REQPACK_SECURITY_CORE_SOURCE_DIR}/tests/cpp
        ${REQPACK_SECURITY_CORE_SOURCE_DIR}/src
        ${REQPACK_SECURITY_CORE_SOURCE_DIR}/src/adapters
    )

    target_link_libraries(rqps_unit_tests PRIVATE
        Catch2::Catch2
        rqps_core
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rqps_unit_tests PRIVATE -Wall -Wextra)
    endif()

    include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
    catch_discover_tests(rqps_unit_tests TEST_PREFIX "rqps::")
endif()
