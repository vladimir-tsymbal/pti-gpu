//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

// Helpers used only by sysmon, so that the tool does not depend on the shared
// utils/utils.h and utils/ze_utils.h headers.

#ifndef PTI_TOOLS_SYSMON_SYSMON_UTILS_H_
#define PTI_TOOLS_SYSMON_SYSMON_UTILS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <utility>
#include <vector>

#include <level_zero/ze_api.h>
#include <level_zero/zes_api.h>

// Checks stay active in release builds, as with PTI_ASSERT in utils/pti_assert.h.
#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

#define SYSMON_ASSERT(X) assert(X)

#define SYSMON_STRINGIFY(x) #x
#define SYSMON_TOSTRING(x) SYSMON_STRINGIFY(x)

namespace sysmon {

inline void SetEnv(const char* name, const char* value) {
  SYSMON_ASSERT(name != nullptr);
  SYSMON_ASSERT(value != nullptr);
  int status = setenv(name, value, 1);
  SYSMON_ASSERT(status == 0);
}

inline std::vector<ze_driver_handle_t> GetDriverList() {
  uint32_t driver_count = 0;
  ze_result_t status = zeDriverGet(&driver_count, nullptr);
  SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);

  std::vector<ze_driver_handle_t> driver_list(driver_count);
  if (driver_count > 0) {
    status = zeDriverGet(&driver_count, driver_list.data());
    SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);
  }
  return driver_list;
}

inline std::vector<ze_device_handle_t> GetDeviceList(ze_driver_handle_t driver) {
  SYSMON_ASSERT(driver != nullptr);
  uint32_t device_count = 0;
  ze_result_t status = zeDeviceGet(driver, &device_count, nullptr);
  SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);

  std::vector<ze_device_handle_t> device_list(device_count);
  if (device_count > 0) {
    status = zeDeviceGet(driver, &device_count, device_list.data());
    SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);
  }
  return device_list;
}

inline std::vector<ze_device_handle_t> GetSubDeviceList(ze_device_handle_t device) {
  SYSMON_ASSERT(device != nullptr);
  uint32_t sub_device_count = 0;
  ze_result_t status = zeDeviceGetSubDevices(device, &sub_device_count, nullptr);
  SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);

  std::vector<ze_device_handle_t> sub_device_list(sub_device_count);
  if (sub_device_count > 0) {
    status = zeDeviceGetSubDevices(device, &sub_device_count, sub_device_list.data());
    SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);
  }
  return sub_device_list;
}

inline ze_api_version_t GetDriverVersion(ze_driver_handle_t driver) {
  SYSMON_ASSERT(driver != nullptr);
  ze_api_version_t version = ZE_API_VERSION_FORCE_UINT32;
  ze_result_t status = zeDriverGetApiVersion(driver, &version);
  SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);
  return version;
}

// Structure type for zeDeviceGetProperties: ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES
// is deprecated; drivers implementing API 1.2 or newer take the _1_2 variant.
inline ze_structure_type_t GetDevicePropertiesType(ze_driver_handle_t driver) {
  return GetDriverVersion(driver) >= ZE_API_VERSION_1_2 ?
      ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES_1_2 :
      ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES;
}

// A GPU as seen by both APIs: the core handle (zeDeviceGet) for the ze*
// property calls and the Sysman handle for the zes* calls. In the legacy mode
// (ZES_ENABLE_SYSMAN=1, no zesInit) both are the same handle.
struct Device {
  ze_driver_handle_t driver;
  ze_device_handle_t core;
  zes_device_handle_t sysman;
  // Sysman sub-device id of the core device if it is one tile of the Sysman
  // device (FLAT hierarchy), otherwise UINT32_MAX.
  uint32_t tile;
};

inline std::vector<zes_driver_handle_t> GetSysmanDriverList() {
  uint32_t driver_count = 0;
  ze_result_t status = zesDriverGet(&driver_count, nullptr);
  std::vector<zes_driver_handle_t> driver_list;
  if (status == ZE_RESULT_SUCCESS && driver_count > 0) {
    driver_list.resize(driver_count);
    status = zesDriverGet(&driver_count, driver_list.data());
    SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);
  }
  return driver_list;
}

inline std::vector<zes_device_handle_t> GetSysmanDeviceList(
    zes_driver_handle_t driver) {
  SYSMON_ASSERT(driver != nullptr);
  uint32_t device_count = 0;
  ze_result_t status = zesDeviceGet(driver, &device_count, nullptr);
  std::vector<zes_device_handle_t> device_list;
  if (status == ZE_RESULT_SUCCESS && device_count > 0) {
    device_list.resize(device_count);
    status = zesDeviceGet(driver, &device_count, device_list.data());
    SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);
  }
  return device_list;
}

static_assert(sizeof(zes_uuid_t::id) == sizeof(ze_device_uuid_t::id),
              "Sysman and core device UUIDs differ in size");

// UUIDs a core device of this Sysman device can have, with the sub-device id
// they stand for: the device's own UUID (zes_device_ext_properties_t;
// UINT32_MAX) and those of its sub-devices. With
// ZE_FLAT_DEVICE_HIERARCHY=FLAT (the default) the core API lists each tile of
// a multi-tile GPU such as PVC as a device, while Sysman lists the whole GPU
// with the tiles as sub-devices.
inline std::vector<std::pair<zes_uuid_t, uint32_t>> GetSysmanDeviceUuids(
    zes_device_handle_t device) {
  SYSMON_ASSERT(device != nullptr);
  std::vector<std::pair<zes_uuid_t, uint32_t>> uuids;

  zes_device_ext_properties_t ext_props{
      ZES_STRUCTURE_TYPE_DEVICE_EXT_PROPERTIES, };
  zes_device_properties_t props{ZES_STRUCTURE_TYPE_DEVICE_PROPERTIES, };
  props.pNext = &ext_props;
  if (zesDeviceGetProperties(device, &props) == ZE_RESULT_SUCCESS) {
    uuids.emplace_back(ext_props.uuid, UINT32_MAX);
  }

  uint32_t sub_count = 0;
  if (zesDeviceGetSubDevicePropertiesExp(device, &sub_count, nullptr) ==
          ZE_RESULT_SUCCESS && sub_count > 0) {
    std::vector<zes_subdevice_exp_properties_t> sub_props(
        sub_count, {ZES_STRUCTURE_TYPE_SUBDEVICE_EXP_PROPERTIES, });
    if (zesDeviceGetSubDevicePropertiesExp(
            device, &sub_count, sub_props.data()) == ZE_RESULT_SUCCESS) {
      for (uint32_t i = 0; i < sub_count; ++i) {
        uuids.emplace_back(sub_props[i].uuid, sub_props[i].subdeviceId);
      }
    }
  }
  return uuids;
}

// Pairs every core device of the drivers with its Sysman device by UUID (the
// two APIs may enumerate them in a different order). Each tile of a FLAT
// multi-tile GPU is paired with the Sysman device of the whole GPU. In the
// legacy mode the core handle is used as the Sysman handle. Core devices
// without a Sysman device are left out.
inline std::vector<Device> GetDevices(
    const std::vector<ze_driver_handle_t>& driver_list, bool legacy) {
  std::vector<std::pair<zes_device_handle_t,
                        std::vector<std::pair<zes_uuid_t, uint32_t>>>>
    sysman_list;
  if (!legacy) {
    for (auto driver : GetSysmanDriverList()) {
      for (auto device : GetSysmanDeviceList(driver)) {
        sysman_list.emplace_back(device, GetSysmanDeviceUuids(device));
      }
    }
  }

  std::vector<Device> devices;
  for (auto driver : driver_list) {
    for (auto core : GetDeviceList(driver)) {
      if (legacy) {
        devices.push_back({driver, core, core, UINT32_MAX});
        continue;
      }
      ze_device_properties_t core_props{GetDevicePropertiesType(driver), };
      ze_result_t status = zeDeviceGetProperties(core, &core_props);
      SYSMON_ASSERT(status == ZE_RESULT_SUCCESS);
      zes_device_handle_t match = nullptr;
      uint32_t tile = UINT32_MAX;
      for (const auto& sysman : sysman_list) {
        for (const auto& uuid : sysman.second) {
          if (memcmp(uuid.first.id, core_props.uuid.id,
                     sizeof(uuid.first.id)) == 0) {
            match = sysman.first;
            tile = uuid.second;
            break;
          }
        }
        if (match != nullptr) {
          break;
        }
      }
      if (match != nullptr) {
        devices.push_back({driver, core, match, tile});
      }
    }
  }
  return devices;
}

}  // namespace sysmon

#endif  // PTI_TOOLS_SYSMON_SYSMON_UTILS_H_
