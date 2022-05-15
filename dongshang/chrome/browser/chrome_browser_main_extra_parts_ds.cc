#include "dongshang/chrome/browser/chrome_browser_main_extra_parts_ds.h"

#include "chrome/browser/browser_process.h"
#include "dongshang/chrome/browser/ds_browser_biz.h"

ChromeBrowserMainExtraPartsDs::ChromeBrowserMainExtraPartsDs() = default;

ChromeBrowserMainExtraPartsDs::~ChromeBrowserMainExtraPartsDs() {}

void ChromeBrowserMainExtraPartsDs::PreMainMessageLoopRun() {
  g_browser_process->ds_browser_biz()->PreMainMessageLoopRun();
}

void ChromeBrowserMainExtraPartsDs::PostBrowserStart() {
  g_browser_process->ds_browser_biz()->PostBrowserStart();
}