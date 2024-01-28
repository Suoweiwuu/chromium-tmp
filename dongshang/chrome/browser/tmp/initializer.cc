#include "content/public/browser/dongshang/initializer.h"

#include "content/public/browser/render_frame_host.h"
#include "chrome/browser/ui/browser.h"

Initializer::Initializer() {}
Initializer::~Initializer() {}

bool Initializer::initialize(std::string code) {
  //content::RenderFrameHost* const frame = browser()->tab_strip_model()->GetActiveWebContents()->GetMainFrame();
  return false;
}
