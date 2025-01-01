// Copyright (c) 2024 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_TASK_MANAGER_TEST_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_TASK_MANAGER_TEST_H_
#pragma once

#include "cefclient/browser/test_runner.h"

namespace client::task_manager_test {

void CreateMessageHandlers(test_runner::MessageHandlerSet& handlers);

}  // namespace client::task_manager_test

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_TASK_MANAGER_TEST_H_
