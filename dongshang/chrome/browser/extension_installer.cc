#include "dongshang/chrome/browser/extension_installer.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/common/chrome_paths.h"

namespace {
const char kExtensionFileName[] = "chrome_extension_1.0.5.crx";
}

void InstallDefaultExtension() {
  base::FilePath exe_dir;
  base::PathService::Get(chrome::DIR_DEFAULT_APPS, &exe_dir);
  base::FilePath extension_dir = exe_dir.AppendASCII(kExtensionFileName);
  if (!base::PathExists(extension_dir)) {
    return;
  }

  Profile* profile = ProfileManager::GetActiveUserProfile();
  if (!profile) {
    return;
  }

  extensions::ExtensionService* service =
      extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!service) {
    DCHECK(false);
    return;
  }
  scoped_refptr<extensions::CrxInstaller> installer =
      extensions::CrxInstaller::CreateSilent(service);
  installer->set_is_gallery_install(true);
  installer->set_allow_silent_install(true);
  installer->set_install_immediately(true);
  installer->set_off_store_install_allow_reason(
      extensions::CrxInstaller::OffStoreInstallAllowedBecausePref);
  installer->InstallCrx(extension_dir);
}