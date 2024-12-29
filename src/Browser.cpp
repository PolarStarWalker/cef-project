#include "Browser.hpp"

namespace {
bool IsViewsEnabled() {
  static const bool enabled = []() {
    // Views is enabled by default, unless `--use-native` is specified.
    return !CefCommandLine::GetGlobalCommandLine()->HasSwitch("use-native");
  }();
  return enabled;
}

bool IsAlloyStyleEnabled() {
  static const bool enabled = []() {
    // Chrome style is enabled by default, unless `--use-alloy-style` is
    // specified.
    return CefCommandLine::GetGlobalCommandLine()->HasSwitch("use-alloy-style");
  }();
  return enabled;
}

constexpr auto kStartupURL = "https://youtu.be/0NQPMTJ9rh0?si=Q3xRpxGRSlVMVKdf";

}  // namespace

void CreateBrowser(CefRefPtr<CefClient> client,
                   const CefString& startup_url,
                   const CefBrowserSettings& settings) {
  CEF_REQUIRE_UI_THREAD();

  const auto runtime_style = IsAlloyStyleEnabled() ? CEF_RUNTIME_STYLE_ALLOY
                                                   : CEF_RUNTIME_STYLE_DEFAULT;

  // Create the browser using the Views framework if "--use-views"  or
  // "--enable-chrome-runtime" is specified via the command-line. Otherwise,
  // create the browser using the native platform framework.
  if (IsViewsEnabled()) {
    // Create the BrowserView.
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        client, startup_url, settings, nullptr, nullptr,
        new BrowserViewDelegate(runtime_style));

    // Create the Window. It will show itself after creation.
    CefWindow::CreateTopLevelWindow(
        new WindowDelegate(browser_view, runtime_style));
  } else {
    // Information used when creating the native window.
    CefWindowInfo window_info;

#if defined(OS_WIN)
    // On Windows we need to specify certain flags that will be passed to
    // CreateWindowEx().
    window_info.SetAsPopup(nullptr, "examples");
#endif

    // Alloy style will create a basic native window. Chrome style will create a
    // fully styled Chrome UI window.
    window_info.runtime_style = runtime_style;

    // Create the browser window.
    CefBrowserHost::CreateBrowser(window_info, client, startup_url, settings,
                                  nullptr, nullptr);
  }
}

// CefApp methods:
CefRefPtr<CefBrowserProcessHandler> BrowserApp::GetBrowserProcessHandler() {
  return this;
}

void BrowserApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // Command-line flags can be modified in this callback.
  // |process_type| is empty for the browser process.
  if (process_type.empty()) {
#if defined(OS_MACOSX)
    // Disable the macOS keychain prompt. Cookies will not be encrypted.
    command_line->AppendSwitch("use-mock-keychain");
#endif
  }
}

// CefBrowserProcessHandler methods:
void BrowserApp::OnContextInitialized() {
  // Create the browser window.
  CreateBrowser(new Client(), kStartupURL, CefBrowserSettings());
}
