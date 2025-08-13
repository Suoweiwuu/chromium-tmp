#include "download_file_util.h"
#include "custom_network_context_client.h"
#include "custom_cert_verifier_params_factory.h"

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/task/thread_pool.h"
#include "base/memory/ref_counted.h"
#include "base/run_loop.h"
#include "base/threading/thread.h"


#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#if BUILDFLAG(IS_WIN)
#include "chrome/updater/win/net/network.h"
#elif BUILDFLAG(IS_MAC)
#include "chrome/updater/mac/net/network.h"
#elif BUILDFLAG(IS_LINUX)
#include "chrome/updater/linux/net/network.h"
#endif

#include "mojo/public/c/system/types.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "mojo/public/cpp/system/data_pipe.h"


#include "net/url_request/url_request.h"

#include "services/network/network_service.h"
#include "services/network/public/cpp/features.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader_stream_consumer.h"
#include "services/network/public/cpp/simple_url_loader_throttle.h"
#include "services/network/public/cpp/url_loader_completion_status.h"
#include "services/network/public/mojom/data_pipe_getter.mojom.h"
#include "services/network/public/mojom/network_service.mojom.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
//#include "services/network/test/fake_test_cert_verifier_params_factory.h"
//#include "services/network/test/test_network_context_client.h"
#include "services/network/public/mojom/data_pipe_getter.mojom.h"
#include "services/network/public/mojom/network_service.mojom.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "services/network/public/cpp/simple_url_loader.h"

#include <thread>
#include <chrono>

DownloadFileUtil::DownloadFileUtil() {}
DownloadFileUtil::~DownloadFileUtil() {}

void DownloadFileUtil::DoDownloadFile(const GURL& url,
                                  const base::FilePath& output_path) {
  network_service_ = network::NetworkService::CreateForTesting();

  network::mojom::NetworkContextParamsPtr context_params =
      network::mojom::NetworkContextParams::New();

  context_params->cert_verifier_params =
      network::CustomCertVerifierParamsFactory::GetCertVerifierParams();

  network_service_->CreateNetworkContext(
      network_context_.BindNewPipeAndPassReceiver(), std::move(context_params));

  mojo::PendingReceiver<network::mojom::URLLoaderNetworkServiceObserver>
      default_observer_receiver;
  network::mojom::NetworkServiceParamsPtr network_service_params =
      network::mojom::NetworkServiceParams::New();
  network_service_params->default_observer =
      default_observer_receiver.InitWithNewPipeAndPassRemote();
  network_service_->SetParams(std::move(network_service_params));

  network::mojom::URLLoaderFactoryParamsPtr params =
      network::mojom::URLLoaderFactoryParams::New();
  params->process_id = network::mojom::kBrowserProcessId;  // 0, 合法
  params->is_corb_enabled = false;
  params->is_trusted = true;
  url::Origin origin = url::Origin::Create(url);
  params->isolation_info = net::IsolationInfo::CreateForInternalRequest(origin);
  network_context_->CreateURLLoaderFactory(
      url_loader_factory_.BindNewPipeAndPassReceiver(), std::move(params));

  std::unique_ptr<network::ResourceRequest> resource_request =
      std::make_unique<network::ResourceRequest>();
  resource_request->url = url;
  resource_request->method = "GET";
  resource_request->enable_upload_progress = true;

  simple_url_loader_ = network::SimpleURLLoader::Create(
      std::move(resource_request), net::DefineNetworkTrafficAnnotation(
                                       "downloader", "Standalone downloader"));

  simple_url_loader_->DownloadToFile(
      url_loader_factory_.get(),
      base::BindOnce(&DownloadFileUtil::DwonloadCompletedCallback,
                     base::Unretained(this)),
      output_path);
}


void DownloadFileUtil::DownloadFile(const GURL& url,
    const base::FilePath& output_path) {
  {
    base::Thread::Options options;
    options.message_pump_type = base::MessagePumpType::IO;  // 关键：IO消息泵
    io_thread_.StartWithOptions(std::move(options));
  }

  // 2. 把任务投递到 IO 线程执行
  io_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&DownloadFileUtil::DoDownloadFile,
                                base::Unretained(this), url, output_path));
}



void DownloadFileUtil::ResponseStartedCallback(int response_code,
                                             int64_t content_length) {
  LOG(INFO) << "ResponseStartedCallback response_code: " << response_code
            << "content_length: " << content_length;
}


void DownloadFileUtil::ProcessCallback(int64_t current) {

}


void DownloadFileUtil::CompletedCallback(int net_error, int64_t content_size) {
  LOG(INFO) << "download finished";
}

void DownloadFileUtil::DwonloadCompletedCallback(base::FilePath path) {
  LOG(INFO) << "download finished";
}