#pragma once
#include <list>

#include "include/cef_browser.h"
#include "include/internal/cef_ptr.h"

class ClientManager {
public:
  ClientManager();
  ~ClientManager();

  // Returns the singleton instance of this object.
  static ClientManager* GetInstance();

  // Called from CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser);
  void DoClose(CefRefPtr<CefBrowser> browser);
  void OnBeforeClose(CefRefPtr<CefBrowser> browser);

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  // Returns true if the last browser instance is closing.
  bool IsClosing() const;

private:
  base::ThreadChecker thread_checker_;

  bool is_closing_;

  // List of existing browsers.
  using BrowserList = std::list<CefRefPtr<CefBrowser>>;
  BrowserList browser_list_;
};