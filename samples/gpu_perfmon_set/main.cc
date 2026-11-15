//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <string>

#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <drm/i915_drm.h>
#include <xf86drm.h>

// The xe observation (OA) uAPI landed in Linux 6.11. With older kernel
// headers the sample builds without the xe path and supports i915 only.
#if defined(__has_include)
#  if __has_include(<drm/xe_drm.h>)
#    include <drm/xe_drm.h>
#  endif
#endif
#if defined(DRM_IOCTL_XE_OBSERVATION)
#  define PTI_XE_OBSERVATION 1
#endif

#include "pti_assert.h"

#define MAX_STR_LEN 128
#define PERF_GUID_LENGTH 37
#define PERF_REG_OFFSET 0xE458

enum class Driver { kI915, kXe };

const char* GetDriverName(Driver driver) {
  return driver == Driver::kXe ? "xe" : "i915";
}

int OpenDrm(const char* name) {
  int fd = drmOpenWithType(name, NULL, DRM_NODE_RENDER);
  if (fd < 0) {
    fd = drmOpenWithType(name, NULL, DRM_NODE_PRIMARY);
  }
  return fd;
}

// Picks the kernel driver at run time, so one build works with both.
int OpenDrm(Driver& driver) {
  driver = Driver::kI915;
  int fd = OpenDrm("i915");
  if (fd < 0) {
    int xe_fd = OpenDrm("xe");
    if (xe_fd >= 0) {
      driver = Driver::kXe;
      fd = xe_fd;
    }
  }
  return fd;
}

void CloseDrm(int fd) {
  drmClose(fd);
}

int SendIoctl(int fd, unsigned request, void* arg) {
  return drmIoctl(fd, request, arg);
}

std::string GetGuid(const std::vector<unsigned>& regs) {
  std::string regs_string;
  for (size_t i = 0; i < regs.size(); ++i) {
    regs_string += regs[i];
  }

  char guid[PERF_GUID_LENGTH] = { 0 };
  std::hash<std::string> hash;
  snprintf(guid, sizeof(guid), "%08x-%04x-%04x-%04x-%012x", 0, 0, 0, 0,
    static_cast<uint32_t>(hash(regs_string)));
  return guid;
}

std::string ConfigureRegistersI915(int fd, unsigned reg_value,
                                   int& config_id) {
  std::vector<unsigned> flex_regs(2);
  flex_regs[0] = PERF_REG_OFFSET;
  flex_regs[1] = reg_value;

  std::string guid = GetGuid(flex_regs);

  struct drm_i915_perf_oa_config param = {0,};
  PTI_ASSERT(sizeof(param.uuid) == PERF_GUID_LENGTH - 1);
  PTI_ASSERT(flex_regs.size() % 2 == 0);
  
  memcpy(param.uuid, guid.c_str(), sizeof(param.uuid));
  
  param.boolean_regs_ptr = 0;
  param.mux_regs_ptr = 0;
  param.flex_regs_ptr = reinterpret_cast<uint64_t>(flex_regs.data());
  
  param.n_boolean_regs = 0;
  param.n_mux_regs = 0;
  PTI_ASSERT(flex_regs.size() < (std::numeric_limits<uint32_t>::max)());
  param.n_flex_regs = static_cast<uint32_t>(flex_regs.size() / 2);

  config_id = SendIoctl(fd, DRM_IOCTL_I915_PERF_ADD_CONFIG, &param);
  return guid;
}

int OpenPerfStreamI915(int fd, int config_id, uint64_t format) {
  PTI_ASSERT(config_id >= 0);
  uint64_t properties[] = {
    DRM_I915_PERF_PROP_SAMPLE_OA, 1,
    DRM_I915_PERF_PROP_OA_METRICS_SET, static_cast<uint64_t>(config_id),
    DRM_I915_PERF_PROP_OA_FORMAT, format };

  struct drm_i915_perf_open_param param = { 0, };
  param.flags = 0;
  param.flags |= I915_PERF_FLAG_FD_CLOEXEC;
  param.flags |= I915_PERF_FLAG_FD_NONBLOCK;
  param.properties_ptr = reinterpret_cast<uint64_t>(properties);
  param.num_properties = sizeof(properties) / (2 * sizeof(uint64_t));

  return SendIoctl(fd, DRM_IOCTL_I915_PERF_OPEN, &param);
}

int OpenPerfStreamI915(int fd, int config_id) {
  // Gen12 (TGL/DG1/ADL) uses the A32u40_A4u32_B8_C8 OA report format. Xe-HP
  // and newer (DG2, PVC, MTL) don't support it for the OA unit and need
  // A24u40_A14u32_B8_C8, so i915 rejects the first one with EINVAL there.
  int stream_fd = OpenPerfStreamI915(fd, config_id,
                                     I915_OA_FORMAT_A32u40_A4u32_B8_C8);
  if (stream_fd == -1 && errno == EINVAL) {
    stream_fd = OpenPerfStreamI915(fd, config_id,
                                   I915_OA_FORMAT_A24u40_A14u32_B8_C8);
  }
  return stream_fd;
}

#if defined(PTI_XE_OBSERVATION)
std::string ConfigureRegistersXe(int fd, unsigned reg_value, int& config_id) {
  std::vector<uint32_t> regs(2);
  regs[0] = PERF_REG_OFFSET;
  regs[1] = reg_value;

  std::string guid = GetGuid(std::vector<unsigned>(regs.begin(), regs.end()));

  struct drm_xe_oa_config config = {};
  PTI_ASSERT(sizeof(config.uuid) == PERF_GUID_LENGTH - 1);
  memcpy(config.uuid, guid.c_str(), sizeof(config.uuid));
  config.n_regs = static_cast<uint32_t>(regs.size() / 2);
  config.regs_ptr = reinterpret_cast<uint64_t>(regs.data());

  struct drm_xe_observation_param param = {};
  param.observation_type = DRM_XE_OBSERVATION_TYPE_OA;
  param.observation_op = DRM_XE_OBSERVATION_OP_ADD_CONFIG;
  param.param = reinterpret_cast<uint64_t>(&config);

  config_id = SendIoctl(fd, DRM_IOCTL_XE_OBSERVATION, &param);
  return guid;
}

uint64_t MakeXeOaFormat(uint32_t type, uint32_t counter_sel,
                        uint32_t counter_size, uint32_t bc_report) {
  return (type & 0xffu) | ((counter_sel & 0xffu) << 8) |
    ((counter_size & 0xffu) << 16) | ((bc_report & 0xffu) << 24);
}

int OpenPerfStreamXe(int fd, int config_id, uint64_t format) {
  PTI_ASSERT(config_id >= 0);
  const uint64_t properties[][2] = {
    {DRM_XE_OA_PROPERTY_SAMPLE_OA, 1},
    {DRM_XE_OA_PROPERTY_OA_METRIC_SET, static_cast<uint64_t>(config_id)},
    {DRM_XE_OA_PROPERTY_OA_FORMAT, format}};
  const size_t count = sizeof(properties) / sizeof(properties[0]);

  // xe takes the stream properties as a chain of set-property extensions.
  struct drm_xe_ext_set_property ext[count];
  memset(ext, 0, sizeof(ext));
  for (size_t i = 0; i < count; ++i) {
    ext[i].base.name = DRM_XE_OA_EXTENSION_SET_PROPERTY;
    ext[i].base.next_extension = (i + 1 < count) ?
      reinterpret_cast<uint64_t>(&ext[i + 1]) : 0;
    ext[i].property = static_cast<uint32_t>(properties[i][0]);
    ext[i].value = properties[i][1];
  }

  struct drm_xe_observation_param param = {};
  param.observation_type = DRM_XE_OBSERVATION_TYPE_OA;
  param.observation_op = DRM_XE_OBSERVATION_OP_STREAM_OPEN;
  param.param = reinterpret_cast<uint64_t>(&ext[0]);

  // The stream fd is opened with O_CLOEXEC and is blocking by default, like
  // the i915 stream above with I915_PERF_FLAG_FD_CLOEXEC.
  return SendIoctl(fd, DRM_IOCTL_XE_OBSERVATION, &param);
}

int OpenPerfStreamXe(int fd, int config_id) {
  // The OA unit only accepts the report formats of its generation (see
  // oa_formats in drivers/gpu/drm/xe/xe_oa.c) and returns EINVAL for others:
  // Xe2+ (BMG, LNL) uses PEC formats, Xe-HP/Xe-LPG (DG2, PVC, MTL) uses
  // A24u40_A14u32_B8_C8, Gen12 uses A32u40_A4u32_B8_C8. Both OAG formats
  // have the same encoding.
  const uint64_t formats[] = {
    MakeXeOaFormat(DRM_XE_OA_FMT_TYPE_PEC, 1, 1, 0),  // PEC64u64
    MakeXeOaFormat(DRM_XE_OA_FMT_TYPE_OAG, 5, 0, 0),  // A24u40_A14u32_B8_C8
  };
  int stream_fd = -1;
  for (uint64_t format : formats) {
    stream_fd = OpenPerfStreamXe(fd, config_id, format);
    if (stream_fd != -1 || errno != EINVAL) {
      break;
    }
  }
  return stream_fd;
}
#endif // PTI_XE_OBSERVATION

void ClosePerfStream(int fd) {
  int status = close(fd);
  PTI_ASSERT(status == 0);
}

int GetDrmCardNumber(int fd) {
  struct stat file_info;
  if (fstat(fd, &file_info)) {
    return -1;
  }

  int major_number = major(file_info.st_rdev);
  int minor_number = minor(file_info.st_rdev);

  char drm_path[MAX_STR_LEN] = { 0 };
  snprintf(drm_path, sizeof(drm_path), "/sys/dev/char/%d:%d/device/drm",
    major_number, minor_number);

  DIR* drm_dir = opendir(drm_path);
  PTI_ASSERT(drm_dir != nullptr);

  dirent* entry = nullptr;
  int card = -1;
  while ((entry = readdir(drm_dir)) != nullptr) {
    if (entry->d_type == DT_DIR && strncmp(entry->d_name, "card", 4) == 0) {
      card = strtol(entry->d_name + 4, nullptr, 10);
      break;
    }
  }
  
  closedir(drm_dir);
  return card;
}

int GetPerfConfigId(int fd, int card, const char* guid) {
  char file_path[MAX_STR_LEN] = { 0 };
  snprintf(file_path, sizeof(file_path),
           "/sys/class/drm/card%d/metrics/%s/id", card, guid);

  std::ifstream file {file_path};
  if (!file) {
    return -1;
  }

  std::string buffer;
  std::getline(file, buffer);
  PTI_ASSERT(buffer.size() > 0);

  return strtol(buffer.c_str(), 0, 0);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Specify perfmon register value" << std::endl;
    return 0;
  }

  unsigned value = std::stoul(argv[1]);

  bool test_mode = false;
  if (argc > 2) {
    if (strcmp(argv[2], "-t") == 0) {
      test_mode = true;
    }
  }

  Driver driver = Driver::kI915;
  int fd = OpenDrm(driver);
  if (fd < 0) {
    std::cout << "Can't open DRM for i915 or xe driver (status: " <<
      -fd << ")" << std::endl;
    return 0;
  }
  const char* driver_name = GetDriverName(driver);

#if !defined(PTI_XE_OBSERVATION)
  if (driver == Driver::kXe) {
    std::cout << "The xe driver is not supported by this build: it needs " <<
      "kernel headers with the xe observation uAPI (Linux 6.11+)" << std::endl;
    CloseDrm(fd);
    return 0;
  }
#endif

  int config_id = -1;
  std::string guid;
  if (driver == Driver::kI915) {
    guid = ConfigureRegistersI915(fd, value, config_id);
  } else {
#if defined(PTI_XE_OBSERVATION)
    guid = ConfigureRegistersXe(fd, value, config_id);
#endif
  }
  if (config_id == -1) {
    if (errno == EADDRINUSE) {
      std::cout << "Configuration with the given GUID is already added" <<
        std::endl;
      
      int card = GetDrmCardNumber(fd);
      PTI_ASSERT(card >= 0);
      config_id = GetPerfConfigId(fd, card, guid.c_str());
      PTI_ASSERT(config_id >= 0);
    } else {
      std::cout << "Adding " << driver_name <<
        " perf configuration is failed (" <<
        errno << ": " << strerror(errno) << ")" << std::endl;
      CloseDrm(fd);
      return 0;
    }
  }

  int stream_fd = -1;
  if (driver == Driver::kI915) {
    stream_fd = OpenPerfStreamI915(fd, config_id);
  } else {
#if defined(PTI_XE_OBSERVATION)
    stream_fd = OpenPerfStreamXe(fd, config_id);
#endif
  }
  if (stream_fd == -1) {
    std::cout << "Opening " << driver_name << " perf stream is failed (" <<
      errno << ": " << strerror(errno) << ")" << std::endl;
    CloseDrm(fd);
    return 0;
  }

  std::cout << "GPU PefMon configuration is completed" << std::endl;
  std::cout << "Press ENTER to deconfigure the driver..." << std::endl;
  if (!test_mode) {
    std::cin.get();
  }

  ClosePerfStream(stream_fd);
  CloseDrm(fd);
  return 0;
}
