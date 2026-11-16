//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "finalizer.h"
#include "perf_access.h"
#include "profiler.h"
#include "utils.h"

static Profiler* profiler = nullptr;

void Finalize();

extern "C" PTI_EXPORT
void Usage() {
  std::cout <<
    "Usage: ./oneprof[.exe] [options] <application> <args>" <<
    std::endl;
  std::cout << "Options:" << std::endl;
  std::cout <<
    "--raw-metrics [-m]               " <<
    "Collect raw metric stream for the device" <<
    std::endl;
  std::cout <<
    "--kernel-intervals [-i]          " <<
    "Collect raw kernel intervals for the device" <<
    std::endl;
  std::cout <<
    "--kernel-metrics [-k]            " <<
    "Collect over-time metrics for each kernel instance" <<
    std::endl;
  std::cout <<
    "--aggregation [-a]               " <<
    "Collect aggregated metrics for each kernel instance in time-based mode" <<
    std::endl;
  std::cout <<
    "--kernel-query [-q]              " <<
    "Collect aggregated metrics for each kernel instance in query-based mode" <<
    std::endl;
  std::cout <<
    "--stall-sampling [-e]            " <<
    "Collect EU stall sampling: stall reasons for each kernel and its hottest instructions (Level Zero and OpenCL kernels)" <<
    std::endl;
  std::cout <<
    "--hot-instructions <N>           " <<
    "Hottest instructions listed per kernel with -e (default is 10, 0 lists all)" <<
    std::endl;
  std::cout <<
    "--device [-d] <ID>               " <<
    "Target device for profiling (default is 0)" <<
    std::endl;
  std::cout <<
    "--group [-g] <NAME>              " <<
    "Target metric group to collect (default is ComputeBasic)" <<
    std::endl;
  std::cout <<
    "--sampling-interval [-s] <VALUE> " <<
    "Sampling interval for metrics collection in us (default is 1000 us)" <<
    std::endl;
  std::cout <<
    "--output [-o] <FILENAME>         " <<
    "Print console logs into the file" <<
    std::endl;
  std::cout <<
    "--raw-data-path [-p] <DIRECTORY> " <<
    "Path to store raw metic data into (default is process folder)" <<
    std::endl;
  std::cout <<
    "--finalize [-f] <FILENAME>       " <<
    "Print output from collected result file" <<
    std::endl;
  std::cout <<
    "--no-finalize                    " <<
    "Do not finalize and do not report collection results" <<
    std::endl;
  std::cout <<
    "--device-list                    " <<
    "Print list of available devices" <<
    std::endl;
  std::cout <<
    "--metric-list                    " <<
    "Print list of available metrics" <<
    std::endl;
  std::cout <<
    "--version                        " <<
    "Print version" <<
    std::endl;
}

// Checks in the launcher, before the application runs, that the metric group
// asked for (--group, default ComputeBasic; EuStallSampling for -e) exists on
// the target device. The query mode (-q) uses event-based groups, the other
// metric modes time-based ones. If Level Zero or its metrics can't be
// initialized here, the check is skipped: the tool reports that when the
// application starts.
static bool CheckMetricGroup() {
  bool query = !utils::GetEnv("ONEPROF_KernelQuery").empty();
  bool stall = !utils::GetEnv("ONEPROF_StallSampling").empty();
  bool stream = !utils::GetEnv("ONEPROF_RawMetrics").empty() ||
                !utils::GetEnv("ONEPROF_KernelMetrics").empty() ||
                !utils::GetEnv("ONEPROF_Aggregation").empty() || stall;
  if (!query && !stream) {
    return true;
  }

  std::string group = utils::GetEnv("ONEPROF_MetricGroup");
  if (stall) {
    group = kStallSamplingGroup;
  } else if (group.empty()) {
    group = "ComputeBasic";
  }

  if (utils::ze::InitGpuDrivers() != ZE_RESULT_SUCCESS) {
    return true;
  }
  std::vector<ze_device_handle_t> device_list = utils::ze::GetDeviceList();
  std::string value = utils::GetEnv("ONEPROF_DeviceId");
  uint32_t device_id = value.empty() ? 0 : std::stoul(value);
  if (device_id >= device_list.size()) {
    return true;
  }

  zet_metric_group_sampling_type_flag_t type = query ?
    ZET_METRIC_GROUP_SAMPLING_TYPE_FLAG_EVENT_BASED :
    ZET_METRIC_GROUP_SAMPLING_TYPE_FLAG_TIME_BASED;
  std::vector<std::string> names;
  if (!GetMetricGroupNames(device_list[device_id], type, &names) ||
      names.empty()) {
    return true;
  }
  if (std::find(names.begin(), names.end(), group) != names.end()) {
    return true;
  }

  std::cerr << "[ERROR] Metric group " << group << " is not available on " <<
    "device #" << device_id << " for " <<
    (query ? "the query mode (-q)" : "time-based sampling") <<
    ". Available groups:" << std::endl;
  for (const std::string& name : names) {
    std::cerr << "  " << name << std::endl;
  }
  if (stall) {
    // The xe driver lists the group only if metric streams are allowed.
    std::cerr << "[INFO] EU stall sampling needs a device that supports it " <<
      "and " << utils::GetPerfParanoidHint() << " set to 0" << std::endl;
  }
  return false;
}

extern "C" PTI_EXPORT
int ParseArgs(int argc, char* argv[]) {
  bool metric_list = false;
  bool finalization = false;

  int app_index = 1;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--raw-metrics") == 0 ||
        strcmp(argv[i], "-m") == 0) {
      utils::SetEnv("ONEPROF_RawMetrics", "1");
      ++app_index;
    } else if (strcmp(argv[i], "--kernel-intervals") == 0 ||
               strcmp(argv[i], "-i") == 0) {
      utils::SetEnv("ONEPROF_KernelIntervals", "1");
      ++app_index;
    } else if (strcmp(argv[i], "--kernel-metrics") == 0 ||
               strcmp(argv[i], "-k") == 0) {
      utils::SetEnv("ONEPROF_KernelMetrics", "1");
      ++app_index;
    } else if (strcmp(argv[i], "--aggregation") == 0 ||
               strcmp(argv[i], "-a") == 0) {
      utils::SetEnv("ONEPROF_Aggregation", "1");
      ++app_index;
    } else if (strcmp(argv[i], "--kernel-query") == 0 ||
               strcmp(argv[i], "-q") == 0) {
      utils::SetEnv("ONEPROF_KernelQuery", "1");
      ++app_index;
    } else if (strcmp(argv[i], "--stall-sampling") == 0 ||
               strcmp(argv[i], "-e") == 0) {
      utils::SetEnv("ONEPROF_StallSampling", "1");
      ++app_index;
    } else if (strcmp(argv[i], "--device") == 0 ||
               strcmp(argv[i], "-d") == 0) {
      ++i;
      if (i >= argc) {
        std::cout << "[ERROR] Device ID is not specified" << std::endl;
        return -1;
      }
      utils::SetEnv("ONEPROF_DeviceId", argv[i]);
      app_index += 2;
    } else if (strcmp(argv[i], "--group") == 0 ||
               strcmp(argv[i], "-g") == 0) {
      ++i;
      if (i >= argc) {
        std::cout << "[ERROR] Metric group is not specified" << std::endl;
        return -1;
      }
      utils::SetEnv("ONEPROF_MetricGroup", argv[i]);
      app_index += 2;
    } else if (strcmp(argv[i], "--sampling-interval") == 0 ||
               strcmp(argv[i], "-s") == 0) {
      ++i;
      if (i >= argc) {
        std::cout << "[ERROR] Sampling is not specified" << std::endl;
        return -1;
      }
      utils::SetEnv("ONEPROF_SamplingInterval", argv[i]);
      app_index += 2;
    } else if (strcmp(argv[i], "--hot-instructions") == 0) {
      ++i;
      if (i >= argc) {
        std::cout << "[ERROR] Hot instruction count is not specified" <<
          std::endl;
        return -1;
      }
      std::string count(argv[i]);
      if (count.empty() || count.size() > 9 ||
          count.find_first_not_of("0123456789") != std::string::npos) {
        std::cerr << "[ERROR] Invalid hot instruction count: " << count <<
          std::endl;
        exit(EXIT_FAILURE);
      }
      utils::SetEnv("ONEPROF_HotInstructions", argv[i]);
      app_index += 2;
    } else if (strcmp(argv[i], "--output") == 0 ||
               strcmp(argv[i], "-o") == 0) {
      ++i;
      if (i >= argc) {
        std::cout << "[ERROR] Log file name is not specified" << std::endl;
        return -1;
      }
      utils::SetEnv("ONEPROF_LogFilename", argv[i]);
      app_index += 2;
    } else if (strcmp(argv[i], "--raw-data-path") == 0 ||
               strcmp(argv[i], "-p") == 0) {
      ++i;
      if (i >= argc) {
        std::cout << "[ERROR] Raw data path is not specified" << std::endl;
        return -1;
      }
      utils::SetEnv("ONEPROF_RawDataPath", argv[i]);
      app_index += 2;
    } else if (strcmp(argv[i], "--finalize") == 0 ||
               strcmp(argv[i], "-f") == 0) {
      ++i;
      if (i >= argc) {
        std::cout << "[ERROR] File name is not specified" << std::endl;
        return -1;
      }
      utils::SetEnv("ONEPROF_ResultFile", argv[i]);
      app_index += 2;
      finalization = true;
    } else if (strcmp(argv[i], "--no-finalize") == 0) {
      utils::SetEnv("ONEPROF_NoFinalize", "1");
      ++app_index;
    } else if (strcmp(argv[i], "--device-list") == 0) {
      PrintDeviceList();
      return 0;
    } else if (strcmp(argv[i], "--metric-list") == 0) {
      metric_list = true;
      ++app_index;
    } else if (strcmp(argv[i], "--version") == 0) {
#ifdef PTI_VERSION
      std::cout << TOSTRING(PTI_VERSION) << std::endl;
#endif
      return 0;
    } else {
      break;
    }
  }

  if (utils::GetEnv("ONEPROF_KernelQuery") == "1") {
    if (utils::GetEnv("ONEPROF_RawMetrics") == "1" ||
        utils::GetEnv("ONEPROF_KernelIntervals") == "1" ||
        utils::GetEnv("ONEPROF_KernelMetrics") == "1" ||
        utils::GetEnv("ONEPROF_Aggregation") == "1") {
      std::cerr << "[ERROR] Query mode cannot be combined with other modes" <<
        std::endl;
      return 0;
    }
  }

  if (utils::GetEnv("ONEPROF_StallSampling") == "1") {
    // -k/-a/-q need the time stamps of the OA groups: EU stall reports have
    // none. -m (raw stall reports) and -i (kernel intervals) can be combined.
    if (utils::GetEnv("ONEPROF_KernelQuery") == "1" ||
        utils::GetEnv("ONEPROF_KernelMetrics") == "1" ||
        utils::GetEnv("ONEPROF_Aggregation") == "1") {
      std::cerr << "[ERROR] Stall sampling (-e) cannot be combined with " <<
        "-k, -a or -q" << std::endl;
      exit(EXIT_FAILURE);
    }
    std::string group = utils::GetEnv("ONEPROF_MetricGroup");
    if (!group.empty() && group != kStallSamplingGroup) {
      std::cerr << "[ERROR] Stall sampling (-e) always uses the " <<
        kStallSamplingGroup << " group, it cannot be combined with -g " <<
        group << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // The count is used when the stall sampling report is printed: after an -e
  // run, or with -e -f for an -e result file.
  if (!utils::GetEnv("ONEPROF_HotInstructions").empty() &&
      utils::GetEnv("ONEPROF_StallSampling") != "1") {
    std::cerr << "[ERROR] --hot-instructions can only be used with " <<
      "stall sampling (-e)" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (finalization) {
    Finalize();
    return 0;
  }

  if (metric_list) {
    std::string value = utils::GetEnv("ONEPROF_DeviceId");
    uint32_t device_id = value.empty() ? 0 : std::stoul(value);
    PrintMetricList(device_id);
    return 0;
  }

  if (!CheckMetricGroup()) {
    exit(EXIT_FAILURE);
  }

  return app_index;
}

extern "C" PTI_EXPORT
void SetToolEnv() {
  utils::SetEnv("ZE_ENABLE_TRACING_LAYER", "1");
  utils::SetEnv("ZET_ENABLE_METRICS", "1");
}

static ProfOptions ReadArgs() {
  std::string value;
  uint32_t flags = 0;
  uint32_t device_id = 0;
  uint32_t sampling_interval = 1000000;
  std::string metric_group("ComputeBasic");
  std::string log_file;
  std::string raw_data_path;
  std::string result_file;
  uint32_t hot_instruction_count = kStallHotInstructionCount;

  value = utils::GetEnv("ONEPROF_RawMetrics");
  if (!value.empty()) {
    flags |= (1 << PROF_RAW_METRICS);
  }

  value = utils::GetEnv("ONEPROF_KernelIntervals");
  if (!value.empty()) {
    flags |= (1 << PROF_KERNEL_INTERVALS);
  }

  value = utils::GetEnv("ONEPROF_KernelMetrics");
  if (!value.empty()) {
    flags |= (1 << PROF_KERNEL_METRICS);
  }

  value = utils::GetEnv("ONEPROF_Aggregation");
  if (!value.empty()) {
    flags |= (1 << PROF_AGGREGATION);
  }

  value = utils::GetEnv("ONEPROF_KernelQuery");
  if (!value.empty()) {
    flags |= (1 << PROF_KERNEL_QUERY);
  }

  value = utils::GetEnv("ONEPROF_MetricGroup");
  if (!value.empty()) {
    metric_group = value;
  }

  value = utils::GetEnv("ONEPROF_StallSampling");
  if (!value.empty()) {
    flags |= (1 << PROF_STALL_SAMPLING);
    metric_group = kStallSamplingGroup;
  }

  value = utils::GetEnv("ONEPROF_LogFilename");
  if (!value.empty()) {
    log_file = value;
  }

  value = utils::GetEnv("ONEPROF_RawDataPath");
  if (!value.empty()) {
    raw_data_path = value;
  }

  value = utils::GetEnv("ONEPROF_ResultFile");
  if (!value.empty()) {
    result_file = value;
  }

  value = utils::GetEnv("ONEPROF_NoFinalize");
  if (!value.empty()) {
    flags |= (1 << PROF_NO_FINALIZE);
  }

  value = utils::GetEnv("ONEPROF_DeviceId");
  if (!value.empty()) {
    device_id = std::stoul(value);
  }

  value = utils::GetEnv("ONEPROF_SamplingInterval");
  if (!value.empty()) {
    sampling_interval = std::stoul(value);
    sampling_interval *= 1000;
  }

  value = utils::GetEnv("ONEPROF_HotInstructions");
  if (!value.empty()) {
    hot_instruction_count = std::stoul(value);
  }

  if (result_file.empty()) {
    result_file = ResultStorage::GetResultFileName(
        raw_data_path, utils::GetPid());
    PTI_ASSERT(!result_file.empty());
  }

  return ProfOptions(
      flags, device_id, sampling_interval, metric_group,
      log_file, raw_data_path, result_file, hot_instruction_count);
}

void EnableProfiling() {
  ze_result_t status = utils::ze::InitGpuDrivers();
  if (status == ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE) {
    std::cout <<
      "[WARNING] Unable to initialize Level Zero Metrics API" << std::endl;
    std::cout << "  Please check that metrics libraries are installed " <<
      "and " << utils::GetPerfParanoidHint() << " is set to 0" << std::endl;
    exit(EXIT_FAILURE);
  }
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);
  profiler = Profiler::Create(ReadArgs());
}

void DisableProfiling() {
  if (profiler != nullptr) {
    delete profiler;
  }
}

void Finalize() {
  utils::SetEnv("ZET_ENABLE_METRICS", "1");

  ze_result_t status = utils::ze::InitGpuDrivers();
  if (status == ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE) {
    std::cout <<
      "[WARNING] Unable to initialize Level Zero Metrics API" << std::endl;
    std::cout << "  Please check that metrics libraries are installed " <<
      "and " << utils::GetPerfParanoidHint() << " is set to 0" << std::endl;
    exit(EXIT_FAILURE);
  }
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  Finalizer* finalizer = Finalizer::Create(ReadArgs());
  if (finalizer != nullptr) {
    finalizer->Report();
    delete finalizer;
  }
}
