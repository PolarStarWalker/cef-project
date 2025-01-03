// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <windows.h>

#include <filesystem>
#include <memory>

#include "cefclient/browser/main_context_impl.h"
#include "cefclient/browser/main_message_loop_multithreaded_win.h"
#include "cefclient/browser/resource.h"
#include "cefclient/browser/root_window_manager.h"
#include "cefclient/browser/test_runner.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "shared/browser/client_app_browser.h"
#include "shared/browser/main_message_loop_external_pump.h"
#include "shared/browser/main_message_loop_std.h"
#include "shared/common/client_app_other.h"
#include "shared/common/client_switches.h"
#include "shared/renderer/client_app_renderer.h"

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
#undef CEF_USE_SANDBOX
#endif /* CEF_USE_SANDBOX */

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library may not link successfully with all VS
// versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

namespace client {
namespace {

std::wstring GetOptions() {
  static constexpr std::wstring_view cmd_options =
      L"--external-message-pump --off-screen-rendering-enabled --shared-texture-enabled --enable-gpu";
  static constexpr std::wstring_view filename = L"cefclient.exe";
  auto str = (std::filesystem::current_path() / filename.data()).wstring();
  str += L" ";
  str += cmd_options.data();
  return str;
}

int RunMain(HINSTANCE hInstance, int nCmdShow) {
  CefMainArgs main_args(hInstance);

  void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Parse command-line arguments.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  const auto str = GetOptions();
  command_line->InitFromString(str);

  // Create a ClientApp of the correct type.
  CefRefPtr<CefApp> app{new ClientAppBrowser()};

  // Execute the secondary process, if any.
  const auto exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0) {
    return exit_code;
  }

  // Create the main context object.
  auto context = std::make_unique<MainContextImpl>(command_line, true);

  // Populate the settings based on command line arguments.
  auto settings = context->PopulateSettings();

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  // Set the ID for the ICON resource that will be loaded from the main
  // executable and used when creating default Chrome windows such as DevTools
  // and Task Manager. Only used with the Chrome runtime.
  settings.chrome_app_icon_id = IDR_MAINFRAME;

  // Create the main message loop object.
  auto message_loop = MainMessageLoopExternalPump::Create();

  // Initialize the CEF browser process. May return false if initialization
  // fails or if early exit is desired (for example, due to process singleton
  // relaunch behavior).
  if (!context->Initialize(main_args, settings, app, sandbox_info)) {
    return CefGetExitCode();
  }

  // Register scheme handlers.
  test_runner::RegisterSchemeHandlers();

  auto window_config = std::make_unique<RootWindowConfig>(command_line);
  window_config->always_on_top = false;
  window_config->with_osr = true;

  // Create the first window.
  context->GetRootWindowManager()->CreateRootWindow(std::move(window_config));

  // Run the message loop. This will block until Quit() is called by the
  // RootWindowManager after all windows have been destroyed.
  const auto result = message_loop->Run();

  // Shut down CEF.
  context->Shutdown();

  // Release objects in reverse order of creation.
  message_loop.reset();
  context.reset();

  return result;
}

}  // namespace
}  // namespace client

// Program entry point function.
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  return client::RunMain(hInstance, nCmdShow);
}
