#include "include/cef_app.h"
#include "include/cef_sandbox_win.h"
#include "include/wrapper/cef_helpers.h"

#include "Browser.hpp"

namespace {

CefRefPtr<CefCommandLine> CreateCommandLine(const CefMainArgs& main_args) {
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
#if defined(OS_WIN)
  command_line->InitFromString(::GetCommandLineW());
#else
  command_line->InitFromArgv(main_args.argc, main_args.argv);
#endif
  return command_line;
}

CefSettings MakeSettings() {
  CefSettings settings{};
  //settings.multi_threaded_message_loop = true;

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif
  return settings;
}

} // namespace

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine,
                      int nCmdShow) {
  void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  // Create a temporary CommandLine object.
  CefRefPtr<CefCommandLine> command_line = CreateCommandLine(main_args);

  // Create a CefApp of the correct process type.
  CefRefPtr<CefApp> app(new BrowserApp);

  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  const auto exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Create the singleton manager instance.
  ClientManager manager;

  // Specify CEF global settings here.
  const auto settings = MakeSettings();

  // Initialize the CEF browser process. The first browser instance will be
  // created in CefBrowserProcessHandler::OnContextInitialized() after CEF has
  // been initialized. May return false if initialization fails or if early exit
  // is desired (for example, due to process singleton relaunch behavior).
  if (!CefInitialize(main_args, settings, app, sandbox_info)) {
    return 1;
  }

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}