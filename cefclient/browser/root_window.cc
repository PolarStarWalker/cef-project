// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/browser/root_window.h"

#include "include/base/cef_callback_helpers.h"
#include "cefclient/browser/main_context.h"
#include "cefclient/browser/root_window_manager.h"
#include "shared/common/client_switches.h"

namespace client {

RootWindowConfig::RootWindowConfig()
    : use_views(MainContext::Get()->UseViewsGlobal()),
      use_alloy_style(MainContext::Get()->UseAlloyStyleGlobal()),
      with_controls(true),
      url(MainContext::Get()->GetMainURL()) {}

RootWindow::RootWindow(bool use_alloy_style)
    : use_alloy_style_(use_alloy_style) {}

RootWindow::~RootWindow() = default;

// static
scoped_refptr<RootWindow> RootWindow::GetForBrowser(int browser_id) {
  return MainContext::Get()->GetRootWindowManager()->GetWindowForBrowser(
      browser_id);
}

}  // namespace client
