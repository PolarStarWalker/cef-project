add_executable(puv-client WIN32)

if(OS_MACOSX OR OS_WINDOWS)
    # Place the target in the "examples" folder in Visual Studio and Xcode.
    set_property(TARGET puv-client PROPERTY FOLDER "demo")
endif()

if(OS_LINUX OR OS_WINDOWS)
    # Logical target used to link the libcef library on Linux and Windows.
    # On macOS the CEF framework is loaded dynamically at startup.
    ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
endif()

if(USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
endif()

# Set the configuration-specific binary output directory.
if(GEN_NINJA OR GEN_MAKEFILES)
    # Force Ninja and Make to create a subdirectory named after the configuration.
    set(EXAMPLE_TARGET_OUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
    set(EXAMPLE_TARGET_OUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
endif()

SET_EXECUTABLE_TARGET_PROPERTIES(puv-client)

target_compile_features(puv-client PRIVATE cxx_std_23)
target_include_directories(puv-client PRIVATE "${CEF_ROOT}")

# Output all binaries to the configuration-specific build directory.
set_target_properties(puv-client PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${EXAMPLE_TARGET_OUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${EXAMPLE_TARGET_OUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${EXAMPLE_TARGET_OUT_DIR}")

if(OS_WINDOWS)
    # Shared Windows resources (version information, icons).
    set(SHARED_RESOURCES_SRCS
            win/big.ico
            win/resource.h
            win/small.ico
    )

    # Manifest files used for executables.
    set(SHARED_EXE_MANIFEST win/shared.exe.manifest)
    set(SHARED_COMPATIBILITY_MANIFEST win/compatibility.manifest)
endif()

if(OS_WINDOWS)
    # Add the custom Windows manifest files to the executable.
    add_custom_command(
            TARGET puv-client
            POST_BUILD
            COMMAND "mt.exe" -nologo
            -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/${SHARED_EXE_MANIFEST}\"
            \"${CMAKE_CURRENT_SOURCE_DIR}/${SHARED_COMPATIBILITY_MANIFEST}\"
            -outputresource:"${EXAMPLE_TARGET_OUT_DIR}/puv-client.exe"\;\#1
            COMMENT "Adding manifest..."
    )
endif()

target_sources(puv-client PRIVATE
        main.cpp
        Client.cpp
        Client.hpp
        ClientManager.cpp
        ClientManager.hpp
        Browser.cpp
        Browser.hpp
        WindowDelegate.cpp
        WindowDelegate.hpp
        resource.rc "${SHARED_RESOURCES_SRCS}")
add_dependencies(puv-client libcef_dll_wrapper)
target_link_libraries(puv-client PRIVATE libcef_lib libcef_dll_wrapper "${CEF_STANDARD_LIBS}" d3d11.lib glu32.lib imm32.lib opengl32.lib delayimp)

if(USE_SANDBOX)
    target_link_libraries( puv-client PRIVATE cef_sandbox_lib "${CEF_SANDBOX_STANDARD_LIBS}")
endif()


if(OS_WINDOWS)
    # Copy binary and resource files to the target output directory.
    COPY_FILES(puv-client "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${EXAMPLE_TARGET_OUT_DIR}")
    COPY_FILES(puv-client "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${EXAMPLE_TARGET_OUT_DIR}")
endif()
