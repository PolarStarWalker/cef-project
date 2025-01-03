// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/main_context_impl.h"

#include <algorithm>
#include <memory>

#include "cefclient/browser/test_runner.h"
#include "include/cef_parser.h"
#include "shared/browser/client_app_browser.h"
#include "shared/common/client_switches.h"
#include "shared/common/string_util.h"

namespace client {

namespace {

// The default URL to load in a browser window.
constexpr std::string_view kDefaultUrl = "https://polaruv.tech";

// Returns the ARGB value for |color|.
cef_color_t ParseColor(const std::string& color) {
  const std::string& colorToLower = AsciiStrToLower(color);
  if (colorToLower == "black") {
    return CefColorSetARGB(255, 0, 0, 0);
  } else if (colorToLower == "blue") {
    return CefColorSetARGB(255, 0, 0, 255);
  } else if (colorToLower == "green") {
    return CefColorSetARGB(255, 0, 255, 0);
  } else if (colorToLower == "red") {
    return CefColorSetARGB(255, 255, 0, 0);
  } else if (colorToLower == "white") {
    return CefColorSetARGB(255, 255, 255, 255);
  }

  // Use the default color.
  return 0;
}

}  // namespace

MainContextImpl::MainContextImpl(bool terminate_when_all_windows_closed)
    : terminate_when_all_windows_closed_(terminate_when_all_windows_closed) {
  // Whether windowless (off-screen) rendering will be used.
  use_windowless_rendering_ = true;

  shared_texture_enabled_ = true;

  external_begin_frame_enabled_ = false;
  windowless_frame_rate_ = 60;

  // Whether the Views framework will be used.
  use_views_ = false;

  if (use_windowless_rendering_ && use_views_) {
    LOG(ERROR) << "Windowless rendering is not supported by the Views framework.";
    use_views_ = false;
  }

  // Whether Alloy style will be used.
  use_alloy_style_ = true;

  if (use_windowless_rendering_ && !use_alloy_style_) {
    LOG(WARNING) << "Windowless rendering requires Alloy style.";
    use_alloy_style_ = true;
  }

  // Whether to use a native parent window.
  static constexpr bool use_chrome_native_parent = false;

#if defined(OS_MAC)
  if (use_chrome_native_parent && !use_alloy_style_) {
    // MacOS does not support Chrome style with native parent. See issue #3294.
    LOG(WARNING) << "Native parent on MacOS requires Alloy style";
    use_alloy_style_ = true;
  }
#endif

  if (!use_views_ && !use_chrome_native_parent && !use_windowless_rendering_) {
    LOG(WARNING) << "Chrome runtime defaults to the Views framework.";
    use_views_ = true;
  }

  background_color_ = CefColorSetARGB(255, 255, 255, 255);
  browser_background_color_ = background_color_;

  // Log the current configuration.
  LOG(WARNING) << "Using " << (use_alloy_style_ ? "Alloy" : "Chrome") << " style; " << (use_views_ ? "Views" : "Native")
               << "-hosted window; " << (use_windowless_rendering_ ? "Windowless" : "Windowed")
               << " rendering (not a warning)";
}

MainContextImpl::~MainContextImpl() {
  // The context must either not have been initialized, or it must have also
  // been shut down.
  DCHECK(!initialized_ || shutdown_);
}

std::string MainContextImpl::GetConsoleLogPath() {
  return GetAppWorkingDirectory() + "console.log";
}

std::string MainContextImpl::GetMainURL() {
  return std::string{kDefaultUrl};
}

cef_color_t MainContextImpl::GetBackgroundColor() {
  return background_color_;
}

bool MainContextImpl::UseViewsGlobal() {
  return use_views_;
}

bool MainContextImpl::UseAlloyStyleGlobal() {
  return use_alloy_style_;
}

bool MainContextImpl::TouchEventsEnabled() {
  return false;
}

bool MainContextImpl::UseDefaultPopup() {
  return false;
}

CefSettings MainContextImpl::PopulateSettings() {
  CefSettings settings = ClientAppBrowser::PopulateSettings();

  if (use_windowless_rendering_) {
    settings.windowless_rendering_enabled = true;
  }

  if (browser_background_color_ != 0) {
    settings.background_color = browser_background_color_;
  }

  return settings;
}

void MainContextImpl::PopulateBrowserSettings(CefBrowserSettings* settings) {
  settings->windowless_frame_rate = windowless_frame_rate_;

  if (browser_background_color_ != 0) {
    settings->background_color = browser_background_color_;
  }

}

void MainContextImpl::PopulateOsrSettings(OsrRendererSettings* settings) {
  settings->show_update_rect = false;

  settings->shared_texture_enabled = shared_texture_enabled_;
  settings->external_begin_frame_enabled = external_begin_frame_enabled_;
  settings->begin_frame_rate = windowless_frame_rate_;

  if (browser_background_color_ != 0) {
    settings->background_color = browser_background_color_;
  }
}

RootWindowManager* MainContextImpl::GetRootWindowManager() {
  DCHECK(InValidState());
  return root_window_manager_.get();
}

bool MainContextImpl::Initialize(const CefMainArgs& args,
                                 const CefSettings& settings,
                                 CefRefPtr<CefApp> application,
                                 void* windows_sandbox_info) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!initialized_);
  DCHECK(!shutdown_);

  if (!CefInitialize(args, settings, application, windows_sandbox_info)) {
    return false;
  }

  // Need to create the RootWindowManager after calling CefInitialize because
  // TempWindowX11 uses cef_get_xdisplay().
  root_window_manager_ = std::make_unique<RootWindowManager>(terminate_when_all_windows_closed_);

  initialized_ = true;

  return true;
}

void MainContextImpl::Shutdown() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(initialized_);
  DCHECK(!shutdown_);

  root_window_manager_.reset();

  CefShutdown();

  shutdown_ = true;
}

}  // namespace client
