// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/renderer/client_renderer.h"
#include "cefclient/renderer/ipc_performance_test.h"
#include "cefclient/renderer/performance_test.h"
#include "shared/renderer/client_app_renderer.h"

namespace client {

// static
void ClientAppRenderer::CreateDelegates(DelegateSet& delegates) {
  renderer::CreateDelegates(delegates);
  performance_test::CreateDelegates(delegates);
  ipc_performance_test::CreateDelegates(delegates);
}

}  // namespace client
