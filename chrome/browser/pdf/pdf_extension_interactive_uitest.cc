// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/run_loop.h"
#include "base/test/with_feature_override.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/browser/pdf/pdf_extension_test_util.h"
#include "chrome/browser/renderer_context_menu/render_view_context_menu_browsertest_util.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
#include "chrome/test/base/interactive_test_utils.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/browser_plugin_guest_manager.h"
#include "content/public/browser/focused_node_details.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/focus_changed_observer.h"
#include "content/public/test/test_utils.h"
#include "extensions/browser/api/extensions_api_client.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "pdf/pdf_features.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/mojom/input/focus_type.mojom-shared.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "url/gurl.h"

#if defined(TOOLKIT_VIEWS) && defined(USE_AURA)
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/touch_selection_controller_client_manager.h"
#include "ui/events/base_event_utils.h"
#include "ui/events/event.h"
#include "ui/events/gesture_event_details.h"
#include "ui/events/types/event_type.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/test/geometry_util.h"
#include "ui/gfx/selection_bound.h"
#include "ui/touch_selection/touch_selection_controller.h"
#include "ui/views/touchui/touch_selection_menu_views.h"
#include "ui/views/widget/any_widget_observer.h"
#include "ui/views/widget/widget.h"
#endif  // defined(TOOLKIT_VIEWS) && defined(USE_AURA)

namespace {

using ::pdf_extension_test_util::ConvertPageCoordToScreenCoord;
using ::pdf_extension_test_util::EnsurePDFHasLoaded;
using ::pdf_extension_test_util::SetInputFocusOnPlugin;

class PDFExtensionInteractiveUITest : public base::test::WithFeatureOverride,
                                      public extensions::ExtensionApiTest {
 public:
  PDFExtensionInteractiveUITest()
      : WithFeatureOverride(chrome_pdf::features::kPdfUnseasoned) {}
  ~PDFExtensionInteractiveUITest() override = default;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    content::IsolateAllSitesForTesting(command_line);
  }

  void SetUpOnMainThread() override {
    extensions::ExtensionApiTest::SetUpOnMainThread();
    host_resolver()->AddRule("*", "127.0.0.1");
    ASSERT_TRUE(embedded_test_server()->InitializeAndListen());
    content::SetupCrossSiteRedirector(embedded_test_server());
    embedded_test_server()->StartAcceptingConnections();
  }

  void TearDownOnMainThread() override {
    ASSERT_TRUE(embedded_test_server()->ShutdownAndWaitUntilComplete());
    extensions::ExtensionApiTest::TearDownOnMainThread();
  }

  content::WebContents* LoadPdfGetGuestContents(const GURL& url) {
    ui_test_utils::NavigateToURLWithDisposition(
        browser(), url, WindowOpenDisposition::NEW_FOREGROUND_TAB,
        ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP);
    if (!EnsurePDFHasLoaded(GetActiveWebContents()))
      return nullptr;

    content::WebContents* contents = GetActiveWebContents();
    content::BrowserPluginGuestManager* guest_manager =
        contents->GetBrowserContext()->GetGuestManager();
    return guest_manager->GetFullPageGuest(contents);
  }

  content::WebContents* GetActiveWebContents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  content::FocusedNodeDetails TabAndWait(content::WebContents* guest_contents,
                                         bool forward) {
    content::FocusChangedObserver focus_observer(guest_contents);
    if (!ui_test_utils::SendKeyPressSync(browser(), ui::VKEY_TAB,
                                         /*control=*/false,
                                         /*shift=*/!forward,
                                         /*alt=*/false,
                                         /*command=*/false)) {
      ADD_FAILURE() << "Failed to send key press";
      return {};
    }
    return focus_observer.Wait();
  }
};

class TabChangedWaiter : public TabStripModelObserver {
 public:
  explicit TabChangedWaiter(Browser* browser) {
    browser->tab_strip_model()->AddObserver(this);
  }
  TabChangedWaiter(const TabChangedWaiter&) = delete;
  TabChangedWaiter& operator=(const TabChangedWaiter&) = delete;
  ~TabChangedWaiter() override = default;

  void Wait() { run_loop_.Run(); }

  // TabStripModelObserver:
  void OnTabStripModelChanged(
      TabStripModel* tab_strip_model,
      const TabStripModelChange& change,
      const TabStripSelectionChange& selection) override {
    if (change.type() == TabStripModelChange::kSelectionOnly)
      run_loop_.Quit();
  }

 private:
  base::RunLoop run_loop_;
};

}  // namespace

// For crbug.com/1038918
IN_PROC_BROWSER_TEST_P(PDFExtensionInteractiveUITest,
                       CtrlPageUpDownSwitchesTabs) {
  content::WebContents* guest_contents =
      LoadPdfGetGuestContents(embedded_test_server()->GetURL("/pdf/test.pdf"));

  auto* tab_strip_model = browser()->tab_strip_model();
  ASSERT_EQ(2, tab_strip_model->count());
  EXPECT_EQ(1, tab_strip_model->active_index());

  SetInputFocusOnPlugin(guest_contents);

  {
    TabChangedWaiter tab_changed_waiter(browser());
    ASSERT_TRUE(ui_test_utils::SendKeyPressSync(browser(), ui::VKEY_NEXT,
                                                /*control=*/true,
                                                /*shift=*/false,
                                                /*alt=*/false,
                                                /*command=*/false));
    tab_changed_waiter.Wait();
  }
  ASSERT_EQ(2, tab_strip_model->count());
  EXPECT_EQ(0, tab_strip_model->active_index());

  {
    TabChangedWaiter tab_changed_waiter(browser());
    ASSERT_TRUE(ui_test_utils::SendKeyPressSync(browser(), ui::VKEY_PRIOR,
                                                /*control=*/true,
                                                /*shift=*/false,
                                                /*alt=*/false,
                                                /*command=*/false));
    tab_changed_waiter.Wait();
  }
  ASSERT_EQ(2, tab_strip_model->count());
  EXPECT_EQ(1, tab_strip_model->active_index());
}

IN_PROC_BROWSER_TEST_P(PDFExtensionInteractiveUITest, FocusForwardTraversal) {
  content::WebContents* guest_contents = LoadPdfGetGuestContents(
      embedded_test_server()->GetURL("/pdf/test.pdf#toolbar=0"));

  // Tab in.
  content::FocusedNodeDetails details =
      TabAndWait(guest_contents, /*forward=*/true);
  EXPECT_EQ(blink::mojom::FocusType::kForward, details.focus_type);

  // Tab out.
  details = TabAndWait(guest_contents, /*forward=*/true);
  EXPECT_EQ(blink::mojom::FocusType::kNone, details.focus_type);
}

IN_PROC_BROWSER_TEST_P(PDFExtensionInteractiveUITest, FocusReverseTraversal) {
  content::WebContents* guest_contents = LoadPdfGetGuestContents(
      embedded_test_server()->GetURL("/pdf/test.pdf#toolbar=0"));

  // Tab in.
  content::FocusedNodeDetails details =
      TabAndWait(guest_contents, /*forward=*/false);
  EXPECT_EQ(blink::mojom::FocusType::kBackward, details.focus_type);

  // Tab out.
  details = TabAndWait(guest_contents, /*forward=*/false);
  EXPECT_EQ(blink::mojom::FocusType::kNone, details.focus_type);
}

#if defined(TOOLKIT_VIEWS) && defined(USE_AURA)
namespace {

views::Widget* TouchSelectText(content::WebContents* contents,
                               const gfx::Point& screen_pos) {
  views::NamedWidgetShownWaiter waiter(views::test::AnyWidgetTestPasskey{},
                                       "TouchSelectionMenuViews");
  content::SimulateTouchEventAt(contents, ui::ET_TOUCH_PRESSED, screen_pos);

  bool success = false;
  if (!content::ExecuteScriptAndExtractBool(
          contents,
          "window.addEventListener('message', function(event) {"
          "  if (event.data.type == 'touchSelectionOccurred')"
          "    window.domAutomationController.send(true);"
          "});",
          &success)) {
    ADD_FAILURE() << "Failed to add message event listener";
    return nullptr;
  }

  if (!success) {
    ADD_FAILURE() << "Failed to receive message";
    return nullptr;
  }

  content::SimulateTouchEventAt(contents, ui::ET_TOUCH_RELEASED, screen_pos);
  return waiter.WaitIfNeededAndGet();
}

}  // namespace

// On text selection, a touch selection menu should pop up. On clicking ellipsis
// icon on the menu, the context menu should open up.
IN_PROC_BROWSER_TEST_P(PDFExtensionInteractiveUITest,
                       ContextMenuOpensFromTouchSelectionMenu) {
  const GURL url = embedded_test_server()->GetURL("/pdf/text_large.pdf");
  content::WebContents* const guest_contents = LoadPdfGetGuestContents(url);
  ASSERT_TRUE(guest_contents);

  gfx::Point screen_pos =
      ConvertPageCoordToScreenCoord(guest_contents, {12, 12});
  views::Widget* widget = TouchSelectText(GetActiveWebContents(), screen_pos);
  ASSERT_TRUE(widget);
  views::View* menu = widget->GetContentsView();
  ASSERT_TRUE(menu);
  views::View* ellipsis_button = menu->GetViewByID(
      views::TouchSelectionMenuViews::ButtonViewId::kEllipsisButton);
  ASSERT_TRUE(ellipsis_button);
  ContextMenuWaiter context_menu_observer;
  ui::GestureEvent tap(0, 0, 0, ui::EventTimeForNow(),
                       ui::GestureEventDetails(ui::ET_GESTURE_TAP));
  ellipsis_button->OnGestureEvent(&tap);
  context_menu_observer.WaitForMenuOpenAndClose();

  // Verify that the expected context menu items are present.
  //
  // Note that the assertion below doesn't use exact matching via
  // testing::ElementsAre, because some platforms may include unexpected extra
  // elements (e.g. an extra separator and IDC=100 has been observed on some Mac
  // bots).
  EXPECT_THAT(
      context_menu_observer.GetCapturedCommandIds(),
      testing::IsSupersetOf(
          {IDC_CONTENT_CONTEXT_COPY, IDC_CONTENT_CONTEXT_SEARCHWEBFOR,
           IDC_PRINT, IDC_CONTENT_CONTEXT_ROTATECW,
           IDC_CONTENT_CONTEXT_ROTATECCW, IDC_CONTENT_CONTEXT_INSPECTELEMENT}));
}

IN_PROC_BROWSER_TEST_P(PDFExtensionInteractiveUITest, TouchSelectionBounds) {
  // Use test.pdf here because it has embedded font metrics. With a fixed zoom,
  // coordinates should be consistent across platforms.
  const GURL url = embedded_test_server()->GetURL("/pdf/test.pdf#zoom=100");
  content::WebContents* const guest_contents = LoadPdfGetGuestContents(url);
  ASSERT_TRUE(guest_contents);

  views::Widget* widget = TouchSelectText(GetActiveWebContents(), {473, 166});
  ASSERT_TRUE(widget);

  auto* touch_selection_controller =
      guest_contents->GetRenderWidgetHostView()
          ->GetTouchSelectionControllerClientManager()
          ->GetTouchSelectionController();

  gfx::SelectionBound start_bound = touch_selection_controller->start();
  EXPECT_EQ(gfx::SelectionBound::LEFT, start_bound.type());
  EXPECT_POINTF_NEAR(gfx::PointF(454.0f, 161.0f), start_bound.edge_start(),
                     1.0f);
  EXPECT_POINTF_NEAR(gfx::PointF(454.0f, 171.0f), start_bound.edge_end(), 1.0f);

  gfx::SelectionBound end_bound = touch_selection_controller->end();
  EXPECT_EQ(gfx::SelectionBound::RIGHT, end_bound.type());
  EXPECT_POINTF_NEAR(gfx::PointF(492.0f, 161.0f), end_bound.edge_start(), 1.0f);
  EXPECT_POINTF_NEAR(gfx::PointF(492.0f, 171.0f), end_bound.edge_end(), 1.0f);
}
#endif  // defined(TOOLKIT_VIEWS) && defined(USE_AURA)

INSTANTIATE_FEATURE_OVERRIDE_TEST_SUITE(PDFExtensionInteractiveUITest);
