// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "shared/browser/client_app_browser.h"

#include "include/base/cef_logging.h"
#include "include/cef_cookie.h"
#include "shared/browser/main_message_loop_external_pump.h"
#include "shared/common/client_switches.h"

namespace client {

ClientAppBrowser::ClientAppBrowser() {
  CreateDelegates(delegates_);
}

// static
CefSettings ClientAppBrowser::PopulateSettings() {
  CefSettings settings;

#if (defined(OS_WIN) || defined(OS_LINUX))
  settings.multi_threaded_message_loop = false;
#endif

    settings.external_message_pump = true;

  std::vector<std::string> cookieable_schemes;
  RegisterCookieableSchemes(cookieable_schemes);
  if (!cookieable_schemes.empty()) {
    std::string list_str;
    for (const auto& scheme : cookieable_schemes) {
      if (!list_str.empty()) {
        list_str += ",";
      }
      list_str += scheme;
    }
    CefString(&settings.cookieable_schemes_list) = list_str;
  }
  return settings;
}

void ClientAppBrowser::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // Pass additional command-line flags to the browser process.
  if (process_type.empty()) {
    // Pass additional command-line flags when off-screen rendering is enabled.
    if (command_line->HasSwitch(switches::kOffScreenRenderingEnabled) &&
        !command_line->HasSwitch(switches::kSharedTextureEnabled)) {
      // Use software rendering and compositing (disable GPU) for increased FPS
      // and decreased CPU usage. This will also disable WebGL so remove these
      // switches if you need that capability.
      // See https://github.com/chromiumembedded/cef/issues/1257 for details.
      if (!command_line->HasSwitch(switches::kEnableGPU)) {
        command_line->AppendSwitch("disable-gpu");
        command_line->AppendSwitch("disable-gpu-compositing");
      }
    }

    if (command_line->HasSwitch(switches::kUseViews) &&
        !command_line->HasSwitch("top-chrome-md")) {
      // Use non-material mode on all platforms by default. Among other things
      // this causes menu buttons to show hover state. See usage of
      // MaterialDesignController::IsModeMaterial() in Chromium code.
      command_line->AppendSwitchWithValue("top-chrome-md", "non-material");
    }

#if defined(OS_MAC)
    // Disable the toolchain prompt on macOS.
    command_line->AppendSwitch("use-mock-keychain");
#endif

    DelegateSet::iterator it = delegates_.begin();
    for (; it != delegates_.end(); ++it) {
      (*it)->OnBeforeCommandLineProcessing(this, command_line);
    }
  }
}

void ClientAppBrowser::OnRegisterCustomPreferences(
    cef_preferences_type_t type,
    CefRawPtr<CefPreferenceRegistrar> registrar) {
  for (auto& delegate : delegates_) {
    delegate->OnRegisterCustomPreferences(this, type, registrar);
  }
}

void ClientAppBrowser::OnContextInitialized() {
  for (auto& delegate : delegates_) {
    delegate->OnContextInitialized(this);
  }
}

bool ClientAppBrowser::OnAlreadyRunningAppRelaunch(
    CefRefPtr<CefCommandLine> command_line,
    const CefString& current_directory) {
  for (auto& delegate : delegates_) {
    if (delegate->OnAlreadyRunningAppRelaunch(this, command_line,
                                              current_directory)) {
      return true;
    }
  }
  return false;
}

void ClientAppBrowser::OnScheduleMessagePumpWork(int64_t delay) {
  // Only used when `--external-message-pump` is passed via the command-line.
  MainMessageLoopExternalPump* message_pump =
      MainMessageLoopExternalPump::Get();
  if (message_pump) {
    message_pump->OnScheduleMessagePumpWork(delay);
  }
}

CefRefPtr<CefClient> ClientAppBrowser::GetDefaultClient() {
  for (auto& delegate : delegates_) {
    if (auto client = delegate->GetDefaultClient(this)) {
      return client;
    }
  }
  return nullptr;
}

}  // namespace client
