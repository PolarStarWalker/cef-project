// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_RENDERER_PERFORMANCE_TEST_H_
#define CEF_TESTS_CEFCLIENT_RENDERER_PERFORMANCE_TEST_H_
#pragma once

#include "shared/renderer/client_app_renderer.h"

namespace client::performance_test {

// Create the renderer delegate. Called from client_app_delegates_renderer.cc.
void CreateDelegates(ClientAppRenderer::DelegateSet& delegates);

}  // namespace client::performance_test

#endif  // CEF_TESTS_CEFCLIENT_RENDERER_PERFORMANCE_TEST_H_
