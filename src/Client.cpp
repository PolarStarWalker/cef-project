#include "Client.hpp"

#include "ClientManager.hpp"
#include "resource.h"

#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_byte_read_handler.h"
#include "include/wrapper/cef_resource_manager.h"
#include "include/wrapper/cef_stream_resource_handler.h"

namespace {

const char kTestOrigin[] = "https://example.com/";

int GetResourceId(const std::string& resource_path) {
  if (resource_path == "logo.png")
    return IDS_LOGO_PNG;
  if (resource_path == "scheme_handler.html")
    return IDS_SCHEME_HANDLER_HTML;
  return 0;
}

// Retrieve the contents of a BINARY resource from the current executable.
bool LoadBinaryResource(int binaryId, DWORD& dwSize, LPBYTE& pBytes) {
  HINSTANCE hInst = GetModuleHandle(nullptr);
  HRSRC hRes =
      FindResource(hInst, MAKEINTRESOURCE(binaryId), MAKEINTRESOURCE(256));
  if (hRes) {
    HGLOBAL hGlob = LoadResource(hInst, hRes);
    if (hGlob) {
      dwSize = SizeofResource(hInst, hRes);
      pBytes = (LPBYTE)LockResource(hGlob);
      if (dwSize > 0 && pBytes)
        return true;
    }
  }

  return false;
}

CefRefPtr<CefStreamReader> GetResourceReader(const std::string& resource_path) {
  int resource_id = GetResourceId(resource_path);
  if (resource_id == 0)
    return nullptr;

  DWORD dwSize;
  LPBYTE pBytes;

  if (LoadBinaryResource(resource_id, dwSize, pBytes)) {
    return CefStreamReader::CreateForHandler(
        new CefByteReadHandler(pBytes, dwSize, nullptr));
  }

  NOTREACHED();  // The resource should be found.
  return nullptr;
}

void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                         const CefString& title) {
  CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
  SetWindowText(hwnd, std::wstring(title).c_str());
}

// Provider implementation for loading BINARY resources from the current
// executable.
class BinaryResourceProvider : public CefResourceManager::Provider {
 public:
  explicit BinaryResourceProvider(const std::string& root_url)
      : root_url_(root_url) {
    DCHECK(!root_url.empty());
  }

  bool OnRequest(scoped_refptr<CefResourceManager::Request> request) override {
    CEF_REQUIRE_IO_THREAD();

    const std::string& url = request->url();
    if (url.find(root_url_) != 0L) {
      // Not handled by this provider.
      return false;
    }

    CefRefPtr<CefResourceHandler> handler;

    const std::string& relative_path = url.substr(root_url_.length());
    if (!relative_path.empty()) {
      CefRefPtr<CefStreamReader> stream =
          GetResourceReader(relative_path.data());
      if (stream.get()) {
        handler = new CefStreamResourceHandler(
            request->mime_type_resolver().Run(url), stream);
      }
    }

    request->Continue(handler);
    return true;
  }

 private:
  std::string root_url_;

  DISALLOW_COPY_AND_ASSIGN(BinaryResourceProvider);
};

CefResourceManager::Provider* CreateBinaryResourceProvider(
    const std::string& url_path) {
  return new BinaryResourceProvider(url_path);
}

std::string DumpRequestContents(CefRefPtr<CefRequest> request) {
  std::stringstream ss;

  ss << "URL: " << std::string(request->GetURL());
  ss << "\nMethod: " << std::string(request->GetMethod());

  CefRequest::HeaderMap headerMap;
  request->GetHeaderMap(headerMap);
  if (headerMap.size() > 0) {
    ss << "\nHeaders:";
    CefRequest::HeaderMap::const_iterator it = headerMap.begin();
    for (; it != headerMap.end(); ++it) {
      ss << "\n\t" << std::string((*it).first) << ": "
         << std::string((*it).second);
    }
  }

  CefRefPtr<CefPostData> postData = request->GetPostData();
  if (postData.get()) {
    CefPostData::ElementVector elements;
    postData->GetElements(elements);
    if (elements.size() > 0) {
      ss << "\nPost Data:";
      CefRefPtr<CefPostDataElement> element;
      CefPostData::ElementVector::const_iterator it = elements.begin();
      for (; it != elements.end(); ++it) {
        element = (*it);
        if (element->GetType() == PDE_TYPE_BYTES) {
          // the element is composed of bytes
          ss << "\n\tBytes: ";
          if (element->GetBytesCount() == 0) {
            ss << "(empty)";
          } else {
            // retrieve the data.
            size_t size = element->GetBytesCount();
            char* bytes = new char[size];
            element->GetBytes(size, bytes);
            ss << std::string(bytes, size);
            delete[] bytes;
          }
        } else if (element->GetType() == PDE_TYPE_FILE) {
          ss << "\n\tFile: " << std::string(element->GetFile());
        }
      }
    }
  }

  return ss.str();
}

void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<CefBrowserView> browser_view =
      CefBrowserView::GetForBrowser(browser);
  if (browser_view) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefWindow> window = browser_view->GetWindow();
    if (window)
      window->SetTitle(title);
  } else {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}

void OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  ClientManager::GetInstance()->OnAfterCreated(browser);
}

bool DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  ClientManager::GetInstance()->DoClose(browser);

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  ClientManager::GetInstance()->OnBeforeClose(browser);
}

// Demonstrate a custom Provider implementation by dumping the request contents.
class RequestDumpResourceProvider : public CefResourceManager::Provider {
 public:
  explicit RequestDumpResourceProvider(const std::string& url) : url_(url) {
    DCHECK(!url.empty());
  }

  bool OnRequest(scoped_refptr<CefResourceManager::Request> request) override {
    CEF_REQUIRE_IO_THREAD();

    const std::string& url = request->url();
    if (url != url_) {
      // Not handled by this provider.
      return false;
    }

    const std::string& dump = DumpRequestContents(request->request());
    std::string str =
        "<html><body bgcolor=\"white\"><pre>" + dump + "</pre></body></html>";
    CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForData(
        static_cast<void*>(const_cast<char*>(str.c_str())), str.size());
    DCHECK(stream.get());
    request->Continue(new CefStreamResourceHandler("text/html", stream));
    return true;
  }

 private:
  std::string url_;

  DISALLOW_COPY_AND_ASSIGN(RequestDumpResourceProvider);
};

// Add example Providers to the CefResourceManager.
void SetupResourceManager(CefRefPtr<CefResourceManager> resource_manager) {
  if (!CefCurrentlyOn(TID_IO)) {
    // Execute on the browser IO thread.
    CefPostTask(TID_IO, base::BindOnce(SetupResourceManager, resource_manager));
    return;
  }

  const std::string& test_origin = kTestOrigin;

  // Add the Provider for dumping request contents.
  resource_manager->AddProvider(
      new RequestDumpResourceProvider(test_origin + "request.html"), 0,
      std::string());

// Add the Provider for bundled resource files.
#if defined(OS_WIN)
  // Read BINARY resources from the executable.
  resource_manager->AddProvider(CreateBinaryResourceProvider(test_origin), 100,
                                std::string());
#elif defined(OS_POSIX)
  // Read individual resource files from a directory on disk.
  std::string resource_dir;
  if (shared::GetResourceDir(resource_dir)) {
    resource_manager->AddDirectoryProvider(test_origin, resource_dir, 100,
                                           std::string());
  }
#endif
}

}  // namespace

Client::Client() {
  resource_manager_ = new CefResourceManager();
  SetupResourceManager(resource_manager_);
}

void Client::OnTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) {
  ::OnTitleChange(browser, title);
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  ::OnAfterCreated(browser);
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
  return ::DoClose(browser);
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  return ::OnBeforeClose(browser);
}

CefRefPtr<CefResourceRequestHandler> Client::GetResourceRequestHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool is_navigation,
    bool is_download,
    const CefString& request_initiator,
    bool& disable_default_handling) {
  CEF_REQUIRE_IO_THREAD();
  return this;
}

cef_return_value_t Client::OnBeforeResourceLoad(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    CefRefPtr<CefCallback> callback) {
  CEF_REQUIRE_IO_THREAD();

  return resource_manager_->OnBeforeResourceLoad(browser, frame, request,
                                                 callback);
}

CefRefPtr<CefResourceHandler> Client::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) {
  CEF_REQUIRE_IO_THREAD();

  return resource_manager_->GetResourceHandler(browser, frame, request);
}
