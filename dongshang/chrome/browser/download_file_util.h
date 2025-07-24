#ifndef DONG_SHANG_CHROME_DOWNLOAD_FILE_UTIL_H_
#define DONG_SHANG_CHROME_DOWNLOAD_FILE_UTIL_H_

#include "url/gurl.h"
#include <string>
#include <vector>
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/network/test/test_network_context_client.h"
#include "services/network/public/mojom/network_service.mojom.h"
#include "services/network/network_service.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "base/run_loop.h"

namespace base {
class FilePath;
}  // namespace base

class DownloadFileUtil {
 public:
  DownloadFileUtil();
  ~DownloadFileUtil();

  void DownloadFile(const GURL& url, const base::FilePath& output_path);

  void DoDownloadFile(const GURL& url, const base::FilePath& output_path);

  void ResponseStartedCallback(int response_code, int64_t content_length);

  void ProcessCallback(int64_t current);

  void CompletedCallback(int net_error, int64_t content_size);

  void DwonloadCompletedCallback(base::FilePath path);

 private:
  std::unique_ptr<network::mojom::NetworkService> network_service_;
  std::unique_ptr<network::mojom::NetworkContextClient> network_context_client_;
  mojo::Remote<network::mojom::NetworkContext> network_context_;
  mojo::Remote<network::mojom::URLLoaderFactory> url_loader_factory_;
  std::unique_ptr<network::SimpleURLLoader> simple_url_loader_;

};

#endif  // DONG_SHANG_CHROME_UTIL_DOWNLOAD_FILE_UTIL_H_
