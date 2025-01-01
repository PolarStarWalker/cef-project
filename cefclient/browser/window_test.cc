// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/window_test.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "include/base/cef_callback.h"
#include "include/views/cef_browser_view.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "cefclient/browser/main_context.h"
#include "cefclient/browser/test_runner.h"
#include "cefclient/browser/window_test_runner.h"
#include "cefclient/browser/window_test_runner_views.h"

#if defined(OS_WIN)
#include "cefclient/browser/window_test_runner_win.h"
#elif defined(OS_LINUX)
#include "cefclient/browser/window_test_runner_gtk.h"
#elif defined(OS_MAC)
#include "cefclient/browser/window_test_runner_mac.h"
#endif

namespace client::window_test {

namespace {

const char kTestUrlPath[] = "/window";
const char kMessagePositionName[] = "WindowTest.Position";
const char kMessageMinimizeName[] = "WindowTest.Minimize";
const char kMessageMaximizeName[] = "WindowTest.Maximize";
const char kMessageRestoreName[] = "WindowTest.Restore";
const char kMessageFullscreenName[] = "WindowTest.Fullscreen";
const char kMessageTitlebarHeightName[] = "WindowTest.TitlebarHeight";

// Create the appropriate platform test runner object.
std::unique_ptr<WindowTestRunner> CreateWindowTestRunner(
    CefRefPtr<CefBrowser> browser) {
  if (CefBrowserView::GetForBrowser(browser)) {
    // Browser is Views-hosted.
    return std::make_unique<WindowTestRunnerViews>();
  }

#if defined(OS_WIN)
  return std::make_unique<WindowTestRunnerWin>();
#elif defined(OS_LINUX)
  return std::make_unique<WindowTestRunnerGtk>();
#elif defined(OS_MAC)
  return std::make_unique<WindowTestRunnerMac>();
#else
#error "No implementation available for your platform."
#endif
}

// Parse the comma-delimited list of integer values.
std::vector<int> ParsePosition(const std::string& message_name) {
  std::vector<int> vec;
  const std::string& vals = message_name.substr(sizeof(kMessagePositionName));
  std::stringstream ss(vals);
  int i;
  while (ss >> i) {
    vec.push_back(i);
    if (ss.peek() == ',') {
      ss.ignore();
    }
  }

  return vec;
}

std::optional<float> ParseHeight(const std::string& message) {
  if (message.size() > sizeof(kMessageTitlebarHeightName)) {
    const std::string& val = message.substr(sizeof(kMessageTitlebarHeightName));
    return std::stof(val);
  } else {
    return std::nullopt;
  }
}

// Handle messages in the browser process.
class Handler : public CefMessageRouterBrowserSide::Handler {
 public:
  Handler() = default;

  // Called due to cefBroadcast execution in window.html.
  bool OnQuery(CefRefPtr<CefBrowser> browser,
               CefRefPtr<CefFrame> frame,
               int64_t query_id,
               const CefString& request,
               bool persistent,
               CefRefPtr<Callback> callback) override {
    // Only handle messages from the test URL.
    const std::string& url = frame->GetURL();
    if (!test_runner::IsTestURL(url, kTestUrlPath)) {
      return false;
    }

    auto runner = CreateWindowTestRunner(browser);

    const std::string& message_name = request;
    if (message_name.find(kMessagePositionName) == 0) {
      const auto vec = ParsePosition(message_name);
      if (vec.size() == 4) {
        runner->SetPos(browser, vec[0], vec[1], vec[2], vec[3]);
      }
    } else if (message_name == kMessageMinimizeName) {
      runner->Minimize(browser);
    } else if (message_name == kMessageMaximizeName) {
      runner->Maximize(browser);
    } else if (message_name == kMessageRestoreName) {
      runner->Restore(browser);
    } else if (message_name == kMessageFullscreenName) {
      runner->Fullscreen(browser);
    } else if (message_name.find(kMessageTitlebarHeightName) == 0) {
      const auto height = ParseHeight(message_name);
      runner->SetTitleBarHeight(browser, height);
    } else {
      NOTREACHED();
    }

    callback->Success("");
    return true;
  }
};

}  // namespace

void CreateMessageHandlers(test_runner::MessageHandlerSet& handlers) {
  handlers.insert(new Handler());
}

}  // namespace client::window_test
