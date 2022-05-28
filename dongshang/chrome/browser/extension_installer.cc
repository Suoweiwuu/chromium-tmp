#include "dongshang/chrome/browser/extension_installer.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/common/chrome_paths.h"

namespace {
const char kExtensionPath[] = "ds-extension";
}  // namespace

void InstallDefaultExtension() {
  base::FilePath extension_dir;
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (!cmd->HasSwitch(kExtensionPath)) {
    LOG(WARNING) << "without --ds-extension";
    return;
  }
  extension_dir = cmd->GetSwitchValuePath(kExtensionPath);
  if (!base::PathExists(extension_dir)) {
    LOG(WARNING) << "--ds-extension extension path not exist";
    return;
  }

  Profile* profile = ProfileManager::GetActiveUserProfile();
  if (!profile) {
    LOG(WARNING) << "InstallDefaultExtension profile is null";
    return;
  }

  extensions::ExtensionService* service =
      extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!service) {
    LOG(WARNING) << "InstallDefaultExtension ExtensionService is null";
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