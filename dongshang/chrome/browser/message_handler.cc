#include "dongshang/chrome/browser/message_handler.h"

#include "base/base64.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "chrome/browser/printing/print_preview_dialog_controller.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tab_contents/tab_contents_iterator.h"
#include "chrome/common/extensions/api/tabs.h"
#include "components/sessions/content/session_tab_helper.h"
#include "content/browser/devtools/protocol/page.h"
#include "content/browser/renderer_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/render_view_host.h"
#include "dongshang/chrome/browser/websocket_server.h"
#include "net/cookies/cookie_util.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "third_party/blink/public/common/widget/device_emulation_params.h"
#include "ui/gfx/skbitmap_operations.h"

namespace {

content::WebContents* GetActiveWebContents() {
  content::WebContents* web_contents = nullptr;
  Browser* browser = chrome::FindLastActive();
  if (!browser) {
    LOG(WARNING) << "GetActiveWebContents FindLastActive is null";
    return nullptr;
  }
  web_contents = browser->tab_strip_model()->GetActiveWebContents();
  return web_contents;
}

content::RenderFrameHost* FindRenderFrameHostByID(
    content::WebContents* web_contents,
    int id) {
  content::RenderFrameHost* rfh = nullptr;

  web_contents->GetMainFrame()->ForEachRenderFrameHost(base::BindRepeating(
      [](content::WebContents* web_contents, int id,
         content::RenderFrameHost** rfh,
         content::RenderFrameHost* render_frame_host) {
        if (render_frame_host->GetFrameTreeNodeId() == id) {
          *rfh = render_frame_host;
          return content::RenderFrameHost::FrameIterationAction::kStop;
        }
        return content::RenderFrameHost::FrameIterationAction::kContinue;
      },
      web_contents, id, &rfh));

  return rfh;
}

void UpdateDeviceEmulationStateForHost(
    const blink::DeviceEmulationParams& params,
    content::RenderWidgetHostImpl* render_widget_host,
    bool device_emulation_enabled) {
  auto& frame_widget = render_widget_host->GetAssociatedFrameWidget();
  if (!frame_widget) {
    LOG(WARNING) << "UpdateDeviceEmulationStateForHost frame_widget is null";
    return;
  }

  if (device_emulation_enabled) {
    frame_widget->EnableDeviceEmulation(params);
  } else {
    frame_widget->DisableDeviceEmulation();
  }
}

void SetDeviceEmulationParams(const blink::DeviceEmulationParams& params,
                              content::RenderFrameHostImpl* host,
                              bool device_emulation_enabled) {
  if (!host)
    return;
  // Device emulation only happens on the main frame.
  if (!host->is_main_frame())
    return;

  UpdateDeviceEmulationStateForHost(params, host->GetRenderWidgetHost(),
                                    device_emulation_enabled);
  content::WebContentsImpl* web_contents =
      static_cast<content::WebContentsImpl*>(
          content::WebContents::FromRenderFrameHost(host));

  // Update portals inside this page.
  for (auto* web_contents : web_contents->GetWebContentsAndAllInner()) {
    if (web_contents->IsPortal()) {
      UpdateDeviceEmulationStateForHost(
          params, web_contents->GetMainFrame()->GetRenderWidgetHost(),
          device_emulation_enabled);
    }
  }
}

}  // namespace

MessageHandler::MessageHandler() {}

MessageHandler::~MessageHandler() {}

void MessageHandler::StartWebSocketServer() {
  LOG(WARNING) << "MessageHandler::StartWebSocketServer";
  websocket_server_.reset(new WebSocketServer(this));
  websocket_server_->Init();
}

void MessageHandler::StopWebSocketServer() {
  if (websocket_server_) {
    LOG(WARNING) << "MessageHandler::StopWebSocketServer";
    websocket_server_.reset();
  }
}

void MessageHandler::OnConnect(int connection_id) {
  LOG(WARNING) << "MessageHandler::OnConnect " << connection_id;
}

void Test() {
  auto& all_tabs = AllTabContentses();
  for (content::WebContents* web_contents : all_tabs) {
    LOG(WARNING) << web_contents->GetURL().spec();
  }
}

void MessageHandler::OnWebSocketMessage(int connection_id, std::string data) {
  absl::optional<base::Value> value = base::JSONReader::Read(data);
  if (!value) {
    LOG(WARNING) << "MessageHandler::OnWebSocketMessage value is null"
                 << connection_id;
    return;
  }

  const base::Value::Dict* dict = value->GetIfDict();
  if (!dict) {
    LOG(WARNING) << "MessageHandler::OnWebSocketMessage value not dict"
                 << connection_id;
    return;
  }

  const std::string* command_name = dict->FindString("commandName");
  if (!command_name) {
    LOG(WARNING) << "MessageHandler::OnWebSocketMessage without commandName"
                 << connection_id;
    return;
  }

  if (*command_name == std::string("GetActiveTabId")) {
    GetActiveTabId(connection_id, dict);
  } else if (*command_name == std::string("GetWindowIdForTabId")) {
    GetWindowIdForTabId(connection_id, dict);
  } else if (*command_name == std::string("GetFrameIndex")) {
    GetFrameIndex(connection_id, dict);
  } else if (*command_name == std::string("GetHtmlValue")) {
    GetHtmlValue(connection_id, dict);
  } else if (*command_name == std::string("CaptureHtmlElement")) {
    CaptureHtmlElement(connection_id, dict);
  } else if (*command_name == std::string("CloseTab")) {
    CloseTab(connection_id, dict);
  } else if (*command_name == std::string("CookieOperator")) {
    CookieOperator(connection_id, dict);
  } else if (*command_name == std::string("Test")) {
    Test();
  } else {
    LOG(ERROR) << "unknown command name:" << command_name->c_str();
  }
}

void MessageHandler::OnClose(int connection_id) {
  LOG(WARNING) << "MessageHandler::OnClose " << connection_id;
}

void MessageHandler::GetActiveTabId(int connection_id,
                                    const base::Value::Dict* dict) {
  if (!dict) {
    LOG(WARNING) << "MessageHandler::GetActiveTabId dict is null "
                 << connection_id;
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    LOG(WARNING) << "MessageHandler::GetActiveTabId without requestId "
                 << connection_id;
    return;
  }

  absl::optional<int> window_id = dict->FindInt("windowId");
  if (!window_id) {
    LOG(WARNING) << "MessageHandler::GetActiveTabId without windowId "
                 << connection_id;
    return;
  }

  int active_tab_id = 0;
  int active_window_id = window_id.value();
  content::WebContents* web_contents = nullptr;
  if (window_id == -1) {
    web_contents = GetActiveWebContents();
  } else {
    for (auto* browser : *BrowserList::GetInstance()) {
      if (browser->session_id().id() == window_id.value()) {
        web_contents = browser->tab_strip_model()->GetActiveWebContents();
      }
    }
  }

  // for print preview
  bool find_print_preview = false;
  printing::PrintPreviewDialogController* dialog_controller =
      printing::PrintPreviewDialogController::GetInstance();
  if (dialog_controller && web_contents) {
    content::WebContents* print_web_contents =
        dialog_controller->GetPrintPreviewForContents(web_contents);
    if (print_web_contents) {
      web_contents = print_web_contents;
      find_print_preview = true;
      LOG(WARNING) << "MessageHandler::GetActiveTabId find print preview "
                   << connection_id;
    }
  }

  if (!web_contents) {
    LOG(WARNING) << "MessageHandler::GetActiveTabId web_contents is null "
                 << connection_id;
    return;
  }

  std::string url;
  std::u16string title;
  extensions::api::tabs::TabStatus status =
      extensions::api::tabs::TabStatus::TAB_STATUS_NONE;

  if (find_print_preview) {
    active_tab_id = -2;
  } else {
    active_tab_id = sessions::SessionTabHelper::IdForTab(web_contents).id();
  }

  url = web_contents->GetURL().spec();
  title = web_contents->GetTitle();
  status = extensions::ExtensionTabUtil::GetLoadingStatus(web_contents);

  base::Value::Dict respond_info;
  respond_info.Set("returnId", *request_id);
  respond_info.Set("retCode", true);
  respond_info.SetByDottedPath("tabInfo.windowId", active_window_id);
  respond_info.SetByDottedPath("tabInfo.id", active_tab_id);
  respond_info.SetByDottedPath("tabInfo.url", url);
  respond_info.SetByDottedPath("tabInfo.title", title);
  respond_info.SetByDottedPath("tabInfo.status",
                               extensions::api::tabs::ToString(status));

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::GetWindowIdForTabId(int connection_id,
                                         const base::Value::Dict* dict) {
  if (!dict) {
    LOG(WARNING) << "MessageHandler::GetWindowIdForTabId dict is null "
                 << connection_id;
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    LOG(WARNING) << "MessageHandler::GetWindowIdForTabId without requestId "
                 << connection_id;
    return;
  }

  absl::optional<int> tab_id = dict->FindInt("tabId");
  if (!tab_id) {
    LOG(WARNING) << "MessageHandler::GetWindowIdForTabId without tabId "
                 << connection_id;
    return;
  }

  auto& all_tabs = AllTabContentses();
  auto tab_id_matches = [tab_id](content::WebContents* web_contents) {
    return sessions::SessionTabHelper::IdForTab(web_contents).id() ==
           tab_id.value();
  };
  auto it = std::find_if(all_tabs.begin(), all_tabs.end(), tab_id_matches);
  if (it == all_tabs.end()) {
    LOG(WARNING) << "MessageHandler::GetWindowIdForTabId not find tabs "
                 << connection_id;
    return;
  }

  int window_id =
      sessions::SessionTabHelper::FromWebContents(*it)->window_id().id();

  base::Value::Dict respond_info;
  respond_info.Set("returnId", *request_id);
  respond_info.Set("retCode", true);
  respond_info.Set("windowId", window_id);

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::GetFrameIndex(int connection_id,
                                   const base::Value::Dict* dict) {
  if (!dict) {
    LOG(WARNING) << "MessageHandler::GetFrameIndex dict is null "
                 << connection_id;
    return;
  }
  content::WebContents* web_contents = GetActiveWebContents();
  if (!web_contents) {
    LOG(WARNING) << "MessageHandler::GetFrameIndex web_contents is null "
                 << connection_id;
    return;
  }

  // for print preview
  printing::PrintPreviewDialogController* dialog_controller =
      printing::PrintPreviewDialogController::GetInstance();
  if (dialog_controller && web_contents) {
    content::WebContents* print_web_contents =
        dialog_controller->GetPrintPreviewForContents(web_contents);
    if (print_web_contents) {
      web_contents = print_web_contents;
      LOG(WARNING) << "MessageHandler::GetActiveTabId find print preview "
                   << connection_id;
    }
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    LOG(WARNING) << "MessageHandler::GetFrameIndex without requestId "
                 << connection_id;
    return;
  }

  absl::optional<int> frame_id = dict->FindInt("frameId");
  if (!frame_id) {
    LOG(WARNING) << "MessageHandler::GetFrameIndex without frameId "
                 << connection_id;
    return;
  }

  content::RenderFrameHost* render_frame_host =
      FindRenderFrameHostByID(web_contents, frame_id.value());
  if (!render_frame_host) {
    LOG(WARNING) << "MessageHandler::GetFrameIndex render_frame_host is null "
                 << connection_id;
    return;
  }

  content::RenderFrameHost* parent_render_frame_host =
      render_frame_host->GetParent();
  if (!parent_render_frame_host) {
    LOG(WARNING)
        << "MessageHandler::GetFrameIndex parent_render_frame_host is null "
        << connection_id;
    return;
  }

  int frame_index = -1;
  web_contents->GetMainFrame()->ForEachRenderFrameHost(base::BindRepeating(
      [](content::RenderFrameHost* parent_render_frame_host, int id,
         int& frame_index, content::RenderFrameHost* render_frame_host) {
        if (render_frame_host->GetParent()) {
          if (render_frame_host->GetParent() == parent_render_frame_host) {
            frame_index++;
            if (render_frame_host->GetFrameTreeNodeId() == id) {
              return content::RenderFrameHost::FrameIterationAction::kStop;
            }
          }
        }

        if (render_frame_host->GetParent()) {
          LOG(ERROR) << "this=" << render_frame_host
                     << " id=" << render_frame_host->GetFrameTreeNodeId()
                     << " parent=" << render_frame_host->GetParent()
                     << " parent_id="
                     << render_frame_host->GetParent()->GetFrameTreeNodeId();
        }

        return content::RenderFrameHost::FrameIterationAction::kContinue;
      },
      parent_render_frame_host, frame_id.value(), std::ref(frame_index)));

  base::Value::Dict respond_info;
  respond_info.Set("returnId", *request_id);
  respond_info.Set("retCode", true);
  respond_info.Set("index", base::NumberToString(frame_index));

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::GetHtmlValue(int connection_id,
                                  const base::Value::Dict* dict) {
  if (!dict) {
    LOG(WARNING) << "MessageHandler::GetHtmlValue dict is null "
                 << connection_id;
    return;
  }
  content::WebContents* web_contents = GetActiveWebContents();
  if (!web_contents) {
    LOG(WARNING) << "MessageHandler::GetHtmlValue web_contents is null "
                 << connection_id;
    return;
  }

  // for print preview
  printing::PrintPreviewDialogController* dialog_controller =
      printing::PrintPreviewDialogController::GetInstance();
  if (dialog_controller && web_contents) {
    content::WebContents* print_web_contents =
        dialog_controller->GetPrintPreviewForContents(web_contents);
    if (print_web_contents) {
      web_contents = print_web_contents;
      LOG(WARNING) << "MessageHandler::GetActiveTabId find print preview "
                   << connection_id;
    }
  }

  content::RenderFrameHost* render_frame_host = web_contents->GetMainFrame();
  if (!render_frame_host) {
    LOG(WARNING) << "MessageHandler::GetHtmlValue render_frame_host is null "
                 << connection_id;
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    LOG(WARNING) << "MessageHandler::GetHtmlValue without requestId "
                 << connection_id;
    return;
  }

  const std::string* element_id = dict->FindString("elementId");
  if (!element_id) {
    LOG(WARNING) << "MessageHandler::GetHtmlValue without elementId "
                 << connection_id;
    return;
  }

  std::u16string javascript(
      base::StringPrintf(u"document.getElementById(\"%ls\").innerHTML",
                         base::ASCIIToUTF16(*element_id).c_str()));

  render_frame_host->AllowInjectingJavaScript();

  render_frame_host->ExecuteJavaScript(
      javascript,
      base::BindOnce(&MessageHandler::OnGetInnerHtml,
                     weak_factory_.GetWeakPtr(), connection_id, *request_id));
}

void MessageHandler::CaptureHtmlElement(int connection_id,
                                        const base::Value::Dict* dict) {
  if (!dict) {
    LOG(WARNING) << "MessageHandler::CaptureHtmlElement dict is null "
                 << connection_id;
    return;
  }
  content::WebContents* web_contents = GetActiveWebContents();
  if (!web_contents) {
    LOG(WARNING) << "MessageHandler::CaptureHtmlElement web_contents is null "
                 << connection_id;
    return;
  }

  // for print preview
  printing::PrintPreviewDialogController* dialog_controller =
      printing::PrintPreviewDialogController::GetInstance();
  if (dialog_controller && web_contents) {
    content::WebContents* print_web_contents =
        dialog_controller->GetPrintPreviewForContents(web_contents);
    if (print_web_contents) {
      web_contents = print_web_contents;
      LOG(WARNING) << "MessageHandler::GetActiveTabId find print preview "
                   << connection_id;
    }
  }

  content::RenderFrameHost* render_frame_host = web_contents->GetMainFrame();
  if (!render_frame_host) {
    LOG(WARNING)
        << "MessageHandler::CaptureHtmlElement render_frame_host is null "
        << connection_id;
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    LOG(WARNING) << "MessageHandler::CaptureHtmlElement without requestId "
                 << connection_id;
    return;
  }

  const std::string* element_id = dict->FindString("elementId");
  if (!element_id) {
    LOG(WARNING) << "MessageHandler::CaptureHtmlElement without elementId "
                 << connection_id;
    return;
  }

  std::u16string javascript(
      base::StringPrintf(uR"(
{
let rect = document.getElementById("%ls").getBoundingClientRect();
let r = {
"left":rect.left,
"top":rect.top,
"right":rect.right,
"bottom":rect.bottom
};
r;
}

)",
                         base::ASCIIToUTF16(*element_id).c_str()));

  render_frame_host->AllowInjectingJavaScript();

  render_frame_host->ExecuteJavaScript(
      javascript,
      base::BindOnce(&MessageHandler::OnGetElementRect,
                     weak_factory_.GetWeakPtr(), connection_id, *request_id));
}

void MessageHandler::CloseTab(int connection_id,
                              const base::Value::Dict* dict) {
  if (!dict) {
    SendErrorMessage(connection_id, "", "CloseTab dict is null");
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    SendErrorMessage(connection_id, "", "CloseTab missing requestId");
    return;
  }

  absl::optional<int> window_id = dict->FindInt("windowId");
  if (!window_id) {
    SendErrorMessage(connection_id, *request_id, "CloseTab missing windowId");
    return;
  }

  absl::optional<int> tab_id = dict->FindInt("tabId");
  if (!tab_id) {
    SendErrorMessage(connection_id, *request_id, "CloseTab missing tabId");
    return;
  }

  auto& all_tabs = AllTabContentses();
  auto tab_id_matches = [tab_id](content::WebContents* web_contents) {
    return sessions::SessionTabHelper::IdForTab(web_contents).id() ==
           tab_id.value();
  };
  auto it = std::find_if(all_tabs.begin(), all_tabs.end(), tab_id_matches);
  if (it == all_tabs.end()) {
    SendErrorMessage(connection_id, *request_id, "CloseTab not find tab");
    return;
  }

  it->Close();

  base::Value::Dict respond_info;
  respond_info.Set("returnId", *request_id);
  respond_info.Set("retCode", true);

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::CookieOperator(int connection_id,
                                    const base::Value::Dict* dict) {
  if (!dict) {
    SendErrorMessage(connection_id, "", "CookieOperator dict is null");
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    SendErrorMessage(connection_id, "", "CookieOperator missing requestId");
    return;
  }

  absl::optional<int> window_id = dict->FindInt("windowId");
  if (!window_id) {
    SendErrorMessage(connection_id, *request_id,
                     "CookieOperator missing windowId");
    return;
  }

  absl::optional<int> tab_id = dict->FindInt("tabId");
  if (!tab_id) {
    SendErrorMessage(connection_id, *request_id,
                     "CookieOperator missing tabId");
    return;
  }

  const std::string* cmd = dict->FindString("cmd");
  if (!cmd) {
    SendErrorMessage(connection_id, *request_id, "CookieOperator missing cmd");
    return;
  }

  Profile* profile = ProfileManager::GetLastUsedProfile();
  if (!profile) {
    SendErrorMessage(connection_id, *request_id,
                     "CookieOperator profile is null");
    return;
  }

  content::StoragePartition* partition = profile->GetDefaultStoragePartition();
  if (!partition) {
    SendErrorMessage(connection_id, *request_id,
                     "CookieOperator partition is null");
    return;
  }

  network::mojom::CookieManager* cookie_manager =
      partition->GetCookieManagerForBrowserProcess();
  if (!cookie_manager) {
    SendErrorMessage(connection_id, *request_id,
                     "CookieOperator cookie_manager is null");
    return;
  }

  GURL cookie_url;
  const std::string* url = dict->FindString("url");
  if (url) {
    if (GURL(*url).is_valid()) {
      cookie_url = GURL(*url);
    } else {
      LOG(WARNING) << "MessageHandler::CookieOperator url is invalid"
                   << connection_id;
    }
  } else {
    auto& all_tabs = AllTabContentses();
    auto tab_id_matches = [tab_id](content::WebContents* web_contents) {
      return sessions::SessionTabHelper::IdForTab(web_contents).id() ==
             tab_id.value();
    };
    auto it = std::find_if(all_tabs.begin(), all_tabs.end(), tab_id_matches);
    if (it == all_tabs.end()) {
      SendErrorMessage(connection_id, *request_id,
                       "CookieOperator not find tab");
      return;
    }

    cookie_url = it->GetVisibleURL();
  }

  if (!cookie_url.is_valid()) {
    SendErrorMessage(connection_id, *request_id,
                     "CookieOperator url is invalid");
    return;
  }

  if (*cmd == std::string("GetAll")) {
    cookie_manager->GetCookieList(
        cookie_url, net::CookieOptions::MakeAllInclusive(),
        net::CookiePartitionKeyCollection::Todo(),
        base::BindOnce(&MessageHandler::OnGetAll, weak_factory_.GetWeakPtr(),
                       connection_id, *request_id));
    return;
  } else if (*cmd == std::string("DeleteAll")) {
    network::mojom::CookieDeletionFilterPtr filter(
        network::mojom::CookieDeletionFilter::New());
    filter->url = cookie_url;
    cookie_manager->DeleteCookies(
        std::move(filter),
        base::BindOnce(&MessageHandler::OnDeleteCookies,
                       weak_factory_.GetWeakPtr(), connection_id, *request_id));
    return;
  } else if (*cmd == std::string("DeleteOne")) {
    const std::string* name = dict->FindString("name");
    if (!name) {
      LOG(WARNING) << "MessageHandler::CookieOperator without name "
                   << connection_id;
      SendErrorMessage(connection_id, *request_id,
                       "CookieOperator DeleteOne missing name");
      return;
    }
    network::mojom::CookieDeletionFilterPtr filter(
        network::mojom::CookieDeletionFilter::New());
    filter->url = cookie_url;
    filter->cookie_name = *name;
    cookie_manager->DeleteCookies(
        std::move(filter),
        base::BindOnce(&MessageHandler::OnDeleteCookies,
                       weak_factory_.GetWeakPtr(), connection_id, *request_id));
    return;
  } else if (*cmd == std::string("AddOrUpdateOne")) {
    const std::string* name = dict->FindString("name");
    if (!name) {
      SendErrorMessage(connection_id, *request_id,
                       "CookieOperator AddOrUpdateOne missing name");
      return;
    }

    const std::string* value = dict->FindString("value");
    if (!value) {
      SendErrorMessage(connection_id, *request_id,
                       "CookieOperator AddOrUpdateOne missing value");
      return;
    }
    std::string cookie_domain;
    net::cookie_util::GetCookieDomainWithString(cookie_url, cookie_url.host(),
                                                &cookie_domain);
    std::unique_ptr<net::CanonicalCookie> cookie =
        ::net::CanonicalCookie::CreateSanitizedCookie(
            cookie_url, *name, *value, cookie_domain, cookie_url.path(),
            base::Time::Now(), base::Time::Now() + base::Days(365),
            base::Time::Now(), false, false, net::CookieSameSite::UNSPECIFIED,
            net::CookiePriority::COOKIE_PRIORITY_MEDIUM, false,
            absl::optional<net::CookiePartitionKey>());

    cookie_manager->SetCanonicalCookie(
        *cookie, cookie_url, net::CookieOptions::MakeAllInclusive(),
        base::BindOnce(&MessageHandler::OnSetCanonicalCookie,
                       weak_factory_.GetWeakPtr(), connection_id, *request_id));
    return;
  }

  base::Value::Dict respond_info;
  respond_info.Set("returnId", *request_id);
  respond_info.Set("retCode", false);
  base::Value::List cookie_list;
  respond_info.Set("cookieList", std::move(cookie_list));

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::OnGetAll(
    int connection_id,
    const std::string& request_id,
    const std::vector<::net::CookieWithAccessResult>& cookies,
    const std::vector<::net::CookieWithAccessResult>& excluded_cookies) {
  base::Value::Dict respond_info;
  respond_info.Set("returnId", request_id);
  respond_info.Set("retCode", true);
  base::Value::List cookie_list;
  for (auto c : cookies) {
    base::Value::Dict cv;
    cv.Set("name", c.cookie.Name());
    cv.Set("value", c.cookie.Value());
    cv.Set("domain", c.cookie.Domain());
    cv.Set("path", c.cookie.Path());
    cv.Set("creation_time",
           base::StringPrintf("%I64d", c.cookie.CreationDate().ToJavaTime()));
    cv.Set("expiration_time",
           base::StringPrintf("%I64d", c.cookie.ExpiryDate().ToJavaTime()));
    cv.Set("secure", c.cookie.IsSecure());
    cv.Set("http_only", c.cookie.IsHttpOnly());
    cookie_list.Append(std::move(cv));
  }

  for (auto c : excluded_cookies) {
    base::Value::Dict cv;
    cv.Set("name", c.cookie.Name());
    cv.Set("value", c.cookie.Value());
    cv.Set("domain", c.cookie.Domain());
    cv.Set("path", c.cookie.Path());
    cv.Set("creation_time",
           base::StringPrintf("%I64d", c.cookie.CreationDate().ToJavaTime()));
    cv.Set("expiration_time",
           base::StringPrintf("%I64d", c.cookie.ExpiryDate().ToJavaTime()));
    cv.Set("secure", c.cookie.IsSecure());
    cv.Set("http_only", c.cookie.IsHttpOnly());
    cookie_list.Append(std::move(cv));
  }
  respond_info.Set("cookieList", std::move(cookie_list));

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::OnDeleteCookies(int connection_id,
                                     const std::string& request_id,
                                     uint32_t num_deleted) {
  base::Value::Dict respond_info;
  respond_info.Set("returnId", request_id);
  respond_info.Set("retCode", true);
  respond_info.Set("num_deleted", (int)num_deleted);
  base::Value::List cookie_list;
  respond_info.Set("cookieList", std::move(cookie_list));

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::OnSetCanonicalCookie(int connection_id,
                                          const std::string& request_id,
                                          net::CookieAccessResult result) {
  base::Value::Dict respond_info;
  respond_info.Set("returnId", request_id);
  respond_info.Set("retCode", true);
  base::Value::List cookie_list;
  respond_info.Set("cookieList", std::move(cookie_list));

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::SendMessage(int connection_id,
                                 const base::Value::Dict* dict) {
  std::string json_string;
  if (!base::JSONWriter::Write(*dict, &json_string)) {
    LOG(WARNING) << "MessageHandler::SendMessage JSONWriter failed "
                 << connection_id;
    return;
  }

  if (websocket_server_) {
    websocket_server_->SendOverWebSocket(connection_id, json_string);
  }
}

void MessageHandler::SendErrorMessage(int connection_id,
                                      const std::string& request_id,
                                      const std::string& error) {
  base::Value::Dict respond_info;
  respond_info.Set("returnId", request_id);
  respond_info.Set("retCode", false);
  respond_info.Set("error", error);

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::OnGetInnerHtml(int connection_id,
                                    std::string request_id,
                                    base::Value result) {
  std::string* value = result.GetIfString();
  if (!value) {
    LOG(WARNING) << "MessageHandler::OnGetInnerHtml result is null "
                 << connection_id << " " << request_id;
    return;
  }

  base::Value::Dict respond_info;
  respond_info.Set("returnId", request_id);
  respond_info.Set("retCode", true);
  respond_info.Set("innerHtml", *value);

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::OnGetElementRect(int connection_id,
                                      std::string request_id,
                                      base::Value result) {
  const base::Value::Dict* dict = result.GetIfDict();
  if (!dict) {
    LOG(WARNING) << "MessageHandler::OnGetElementRect dict is null "
                 << connection_id << " " << request_id;
    return;
  }

  absl::optional<double> left = dict->FindDouble("left");
  if (!left) {
    LOG(WARNING) << "MessageHandler::OnGetElementRect left is null "
                 << connection_id << " " << request_id;
    return;
  }

  absl::optional<double> top = dict->FindDouble("top");
  if (!top) {
    LOG(WARNING) << "MessageHandler::OnGetElementRect top is null "
                 << connection_id << " " << request_id;
    return;
  }

  absl::optional<double> right = dict->FindDouble("right");
  if (!right) {
    LOG(WARNING) << "MessageHandler::OnGetElementRect right is null "
                 << connection_id << " " << request_id;
    return;
  }

  absl::optional<double> bottom = dict->FindDouble("bottom");
  if (!bottom) {
    LOG(WARNING) << "MessageHandler::OnGetElementRect bottom is null "
                 << connection_id << " " << request_id;
    return;
  }

  content::WebContents* web_contents = GetActiveWebContents();
  if (!web_contents) {
    LOG(WARNING) << "MessageHandler::OnGetElementRect web_contents is null "
                 << connection_id << " " << request_id;
    return;
  }

  // for print preview
  printing::PrintPreviewDialogController* dialog_controller =
      printing::PrintPreviewDialogController::GetInstance();
  if (dialog_controller && web_contents) {
    content::WebContents* print_web_contents =
        dialog_controller->GetPrintPreviewForContents(web_contents);
    if (print_web_contents) {
      web_contents = print_web_contents;
      LOG(WARNING) << "MessageHandler::GetActiveTabId find print preview "
                   << connection_id;
    }
  }

  content::RenderFrameHostImpl* render_frame_host =
      (content::RenderFrameHostImpl*)web_contents->GetMainFrame();
  if (!render_frame_host || !render_frame_host->GetRenderWidgetHost() ||
      !render_frame_host->GetRenderWidgetHost()->GetView()) {
    LOG(WARNING)
        << "MessageHandler::OnGetElementRect render_frame_host is null "
        << connection_id << " " << request_id;
    return;
  }

  content::RenderWidgetHostImpl* widget_host =
      render_frame_host->GetRenderWidgetHost();

  std::unique_ptr<content::protocol::Page::Viewport> clip =
      content::protocol::Page::Viewport::Create()
          .SetX(left.value())
          .SetY(top.value())
          .SetHeight(bottom.value() - top.value())
          .SetWidth(right.value() - left.value())
          .SetScale(1.0)
          .Build();

  blink::DeviceEmulationParams original_params = blink::DeviceEmulationParams();
  blink::DeviceEmulationParams modified_params = original_params;
  gfx::Size original_view_size = widget_host->GetView()->GetViewBounds().size();
  double dpfactor = 1;
  float widget_host_device_scale_factor = widget_host->GetDeviceScaleFactor();

  modified_params.view_size = original_view_size;
  modified_params.screen_size = gfx::Size();
  modified_params.device_scale_factor = 0;
  modified_params.scale = 1;
  modified_params.viewport_offset.SetPoint(clip->GetX(), clip->GetY());
  modified_params.viewport_scale = dpfactor;
  modified_params.viewport_offset.Scale(widget_host_device_scale_factor);

  absl::optional<blink::web_pref::WebPreferences> maybe_original_web_prefs;
  blink::web_pref::WebPreferences original_web_prefs =
      render_frame_host->render_view_host()
          ->GetDelegate()
          ->GetOrCreateWebPreferences();
  maybe_original_web_prefs = original_web_prefs;

  blink::web_pref::WebPreferences modified_web_prefs = original_web_prefs;
  // Hiding scrollbar is needed to avoid scrollbar artefacts on the
  // screenshot. Details: https://crbug.com/1003629.
  modified_web_prefs.hide_scrollbars = true;
  modified_web_prefs.record_whole_document = true;
  render_frame_host->render_view_host()->GetDelegate()->SetWebPreferences(
      modified_web_prefs);

  {
    // TODO(crbug.com/1141835): Remove the bug is fixed.
    // Walkaround for the bug. Emulated `view_size` has to be set twice,
    // otherwise the scrollbar will be on the screenshot present.
    blink::DeviceEmulationParams tmp_params = modified_params;
    tmp_params.view_size = gfx::Size(1, 1);
    SetDeviceEmulationParams(tmp_params, render_frame_host, true);
  }

  // We use DeviceEmulationParams to either emulate, set viewport or both.
  SetDeviceEmulationParams(modified_params, render_frame_host, true);

  {
    double scale = dpfactor * clip->GetScale();
    widget_host->GetView()->SetSize(
        gfx::Size(base::ClampRound(clip->GetWidth() * scale),
                  base::ClampRound(clip->GetHeight() * scale)));
  }

  gfx::Size requested_image_size = gfx::Size();
  requested_image_size = gfx::Size(clip->GetWidth(), clip->GetHeight());

  {
    double scale = widget_host_device_scale_factor * dpfactor;
    scale *= clip->GetScale();
    requested_image_size = gfx::ScaleToRoundedSize(requested_image_size, scale);
  }

  widget_host->GetSnapshotFromBrowser(
      base::BindOnce(&MessageHandler::ScreenshotCaptured,
                     weak_factory_.GetWeakPtr(), connection_id, request_id,
                     original_view_size, requested_image_size, original_params,
                     maybe_original_web_prefs),
      true);
}

void SaveImageDataToFile(scoped_refptr<base::RefCountedMemory> data) {
  base::FilePath file_path(L"D:\\a.png");
  base::WriteFile(file_path, (const char*)data->data(), data->size());
}

void MessageHandler::ScreenshotCaptured(
    int connection_id,
    std::string request_id,
    const gfx::Size& original_view_size,
    const gfx::Size& requested_image_size,
    const blink::DeviceEmulationParams& original_params,
    const absl::optional<blink::web_pref::WebPreferences>& original_web_prefs,
    const gfx::Image& image) {
  content::WebContents* web_contents = GetActiveWebContents();
  if (!web_contents) {
    LOG(WARNING) << "MessageHandler::ScreenshotCaptured web_contents is null "
                 << connection_id << " " << request_id;
    return;
  }

  // for print preview
  printing::PrintPreviewDialogController* dialog_controller =
      printing::PrintPreviewDialogController::GetInstance();
  if (dialog_controller && web_contents) {
    content::WebContents* print_web_contents =
        dialog_controller->GetPrintPreviewForContents(web_contents);
    if (print_web_contents) {
      web_contents = print_web_contents;
      LOG(WARNING) << "MessageHandler::GetActiveTabId find print preview "
                   << connection_id;
    }
  }

  content::RenderFrameHostImpl* render_frame_host =
      (content::RenderFrameHostImpl*)web_contents->GetMainFrame();

  content::RenderWidgetHostImpl* widget_host =
      render_frame_host->GetRenderWidgetHost();
  widget_host->GetView()->SetSize(original_view_size);
  SetDeviceEmulationParams(original_params, render_frame_host, false);

  render_frame_host->render_view_host()->GetDelegate()->SetWebPreferences(
      original_web_prefs.value());

  if (image.IsEmpty()) {
    LOG(WARNING) << "MessageHandler::ScreenshotCaptured image is null "
                 << connection_id << " " << request_id;
    return;
  }

  const SkBitmap* bitmap = image.ToSkBitmap();
  SkBitmap cropped = SkBitmapOperations::CreateTiledBitmap(
      *bitmap, 0, 0, requested_image_size.width(),
      requested_image_size.height());
  gfx::Image croppedImage = gfx::Image::CreateFrom1xBitmap(cropped);
  scoped_refptr<base::RefCountedMemory> data = image.As1xPNGBytes();

  base::span<const uint8_t> input(data->data(), data->data() + data->size());
  std::string base64_imagedata =
      std::string("data:image/png;base64,") + base::Base64Encode(input);

  base::Value::Dict respond_info;
  respond_info.Set("returnId", request_id);
  respond_info.Set("retCode", true);
  respond_info.Set("imageBase64", base64_imagedata);

  SendMessage(connection_id, &respond_info);

  // base::ThreadPool::PostTask(
  //    FROM_HERE, {base::TaskPriority::BEST_EFFORT, base::MayBlock()},
  //    base::BindOnce(&SaveImageDataToFile, data));
}