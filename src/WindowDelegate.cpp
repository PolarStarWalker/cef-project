#include "WindowDelegate.hpp"

#include "include/views/cef_window.h"

WindowDelegate::WindowDelegate(CefRefPtr<CefBrowserView> browser_view,
                               cef_runtime_style_t runtime_style)
    : browser_view_(browser_view), runtime_style_(runtime_style) {}

void WindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window) {
  // Add the browser view and show the window.
  window->AddChildView(browser_view_);
  window->Show();

  // Give keyboard focus to the browser view.
  browser_view_->RequestFocus();
}

void WindowDelegate::OnWindowDestroyed(CefRefPtr<CefWindow> window) {
  browser_view_ = nullptr;
}

bool WindowDelegate::CanClose(CefRefPtr<CefWindow> window) {
  // Allow the window to close if the browser says it's OK.
  auto browser = browser_view_->GetBrowser();
  if (browser)
    return browser->GetHost()->TryCloseBrowser();
  return true;
}

CefSize WindowDelegate::GetPreferredSize(CefRefPtr<CefView> view) {
  // Preferred window size.
  return {800, 600};
}

CefSize WindowDelegate::GetMinimumSize(CefRefPtr<CefView> view) {
  // Minimum window size.
  return {200, 100};
}

cef_runtime_style_t WindowDelegate::GetWindowRuntimeStyle() {
  return runtime_style_;
}
