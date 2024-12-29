#pragma once

#include "include/views/cef_browser_view.h"
#include "include/views/cef_window_delegate.h"

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class WindowDelegate : public CefWindowDelegate {
 public:
  WindowDelegate(CefRefPtr<CefBrowserView> browser_view,
                 cef_runtime_style_t runtime_style);

  void OnWindowCreated(CefRefPtr<CefWindow> window) override;
  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override;

  bool CanClose(CefRefPtr<CefWindow> window) override;

  CefSize GetPreferredSize(CefRefPtr<CefView> view) override;

  CefSize GetMinimumSize(CefRefPtr<CefView> view) override;

  cef_runtime_style_t GetWindowRuntimeStyle() override;

 private:
  CefRefPtr<CefBrowserView> browser_view_;
  const cef_runtime_style_t runtime_style_;

  IMPLEMENT_REFCOUNTING(WindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(WindowDelegate);
};
