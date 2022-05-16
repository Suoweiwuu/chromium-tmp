#include "dongshang/chrome/browser/message_handler.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tab_contents/tab_contents_iterator.h"
#include "chrome/common/extensions/api/tabs.h"
#include "components/sessions/content/session_tab_helper.h"
#include "dongshang/chrome/browser/websocket_server.h"

MessageHandler::MessageHandler() {}

MessageHandler::~MessageHandler() {}

void MessageHandler::StartWebSocketServer() {
  websocket_server_.reset(new WebSocketServer(this));
  websocket_server_->Init();
}

void MessageHandler::StopWebSocketServer() {
  if (websocket_server_) {
    websocket_server_.reset();
  }
}

void MessageHandler::OnConnect(int connection_id) {
  int ia = 0;
  ia++;
}

void MessageHandler::OnWebSocketMessage(int connection_id, std::string data) {
  absl::optional<base::Value> value = base::JSONReader::Read(data);
  if (!value) {
    return;
  }

  const base::Value::Dict* dict = value->GetIfDict();
  if (!dict) {
    return;
  }

  const std::string* command_name = dict->FindString("commandName");
  if (!command_name) {
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
  } else {
    LOG(ERROR) << "unknown command name:" << command_name->c_str();
  }
}

void MessageHandler::OnClose(int connection_id) {
  int ia = 0;
  ia++;
}

void MessageHandler::GetActiveTabId(int connection_id,
                                    const base::Value::Dict* dict) {
  if (!dict) {
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    return;
  }

  absl::optional<int> window_id = dict->FindInt("windowId");
  if (!window_id) {
    return;
  }

  int active_tab_id = 0;
  int active_window_id = window_id.value();
  content::WebContents* web_contents = nullptr;
  if (window_id == -1) {
    Browser* browser = chrome::FindLastActive();
    if (browser) {
      active_window_id = browser->session_id().id();
      web_contents = browser->tab_strip_model()->GetActiveWebContents();
    }
  } else {
    for (auto* browser : *BrowserList::GetInstance()) {
      if (browser->session_id().id() == window_id.value()) {
        web_contents = browser->tab_strip_model()->GetActiveWebContents();
      }
    }
  }

  std::string url;
  std::u16string title;
  extensions::api::tabs::TabStatus status =
      extensions::api::tabs::TabStatus::TAB_STATUS_NONE;
  if (web_contents) {
    active_tab_id = sessions::SessionTabHelper::IdForTab(web_contents).id();
    url = web_contents->GetURL().spec();
    title = web_contents->GetTitle();
    status = extensions::ExtensionTabUtil::GetLoadingStatus(web_contents);
  }

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
    return;
  }

  const std::string* request_id = dict->FindString("requestId");
  if (!request_id) {
    return;
  }

  absl::optional<int> tab_id = dict->FindInt("tabId");
  if (!tab_id) {
    return;
  }

  auto& all_tabs = AllTabContentses();
  auto tab_id_matches = [tab_id](content::WebContents* web_contents) {
    return sessions::SessionTabHelper::IdForTab(web_contents).id() ==
           tab_id.value();
  };
  auto it = std::find_if(all_tabs.begin(), all_tabs.end(), tab_id_matches);
  if (it == all_tabs.end()) {
    return;
  }

  int window_id =
      sessions::SessionTabHelper::FromWebContents(*it)->window_id().id();

  base::Value::Dict respond_info;
  respond_info.Set("returnId", *request_id);
  respond_info.Set("retCode", true);
  respond_info.Set("tabInfo.windowId", window_id);

  SendMessage(connection_id, &respond_info);
}

void MessageHandler::GetFrameIndex(int connection_id,
                                   const base::Value::Dict* dict) {}

void MessageHandler::GetHtmlValue(int connection_id,
                                  const base::Value::Dict* dict) {}

void MessageHandler::SendMessage(int connection_id,
                                 const base::Value::Dict* dict) {
  std::string json_string;
  if (!base::JSONWriter::Write(*dict, &json_string)) {
    return;
  }

  if (websocket_server_) {
    websocket_server_->SendOverWebSocket(connection_id, json_string);
  }
}
