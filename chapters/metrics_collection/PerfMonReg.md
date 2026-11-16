# Metrics Collection with Performance Monitoring (PM) Register
## Overview
Performance monitoring (PM) register is a special register inside Intel(R) Processor Graphics execution unit (EU) that can be set to collect some particular hardware events (one event at a time).

To set collection for an event, one have to define three register fields:
- Event ID - which event to collect;
- Coarse Filter - filter by shader type and EU raws;
- Fine Filter - filter by instruction type and EUs inside a raw.

Exact event list and filter options can be found in Programmer's Reference Manual for Intel(R) Processor Graphics (Kaby Lake), [Volume 14: Observability](https://cdrdv2.intel.com/v1/dl/getContent/723971).

**Supported OS**:
- Linux

**Supported HW**:
- The driver configuration below (Event Set) is tested on Intel(R) Data Center GPU Max 1550 (Xe-HPC, i915 driver) and Intel(R) Arc(TM) B570 Graphics (Xe2, xe driver): the configuration is added and the stream opens. Whether the PM register at offset `0xE458` (from the Kaby Lake manual) counts the expected events on these GPUs is not verified, and other GPUs are not tested

**Needed Headers**:
- [i915_drm.h](https://github.com/torvalds/linux/blob/master/include/uapi/drm/i915_drm.h) (i915) or [xe_drm.h](https://github.com/torvalds/linux/blob/master/include/uapi/drm/xe_drm.h) (xe, Linux 6.11+) kernel headers (e.g. the `linux-libc-dev` package) and `xf86drm.h` from [libdrm](https://dri.freedesktop.org/libdrm/) (e.g. the `libdrm-dev` package) to **set**
- [Graphics Technology Pin (GT Pin)](https://www.intel.com/content/www/us/en/developer/articles/tool/gtpin.html) headers to **read**

**Needed Libraries**:
- [libdrm](https://dri.freedesktop.org/libdrm/) to **set**
- [Graphics Technology Pin (GT Pin)](https://www.intel.com/content/www/us/en/developer/articles/tool/gtpin.html) libraries to **read**

## How To Use
### Event Set
To tune PM register to collect a particular event one may use Direct Rendering Manager (DRM) interface to deal with Intel(R) Processor Graphics driver. The steps differ between the i915 and the xe kernel drivers; the [GPU PerfMon Set](../../samples/gpu_perfmon_set) sample picks the driver at run time and supports both.

#### i915 Kernel Driver
First of all one have to open DRM interface for i915 driver:
```cpp
int fd = drmOpenWithType("i915", NULL, DRM_NODE_RENDER);
if (fd < 0) {
  fd = drmOpenWithType("i915", NULL, DRM_NODE_PRIMARY);
}
assert(fd >= 0);
```
Second step is to prepare PM register configuration:
```cpp
#define PERF_GUID_LENGTH 37

// PM register is in a group of flexible registers
std::vector<unsigned> flex_regs(2);
flex_regs[0] = PERF_REG_OFFSET;
flex_regs[1] = reg_value; // combined filters + event_id value

std::string guid = GetGuid(flex_regs);

struct drm_i915_perf_oa_config param = {0,};
assert(sizeof(param.uuid) == PERF_GUID_LENGTH - 1);
assert(flex_regs.size() % 2 == 0);

memcpy(param.uuid, guid.c_str(), sizeof(param.uuid));

// Register values
param.boolean_regs_ptr = 0;
param.mux_regs_ptr = 0;
param.flex_regs_ptr = reinterpret_cast<uint64_t>(flex_regs.data());

// Number of register values
param.n_boolean_regs = 0;
param.n_mux_regs = 0;
param.n_flex_regs = static_cast<uint32_t>(flex_regs.size() / 2);

config_id = SendIoctl(fd, DRM_IOCTL_I915_PERF_ADD_CONFIG, &param);
```
The only unobvious field here is GUID, it should be formed as following:
```cpp
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
```
Finally one have to use received `config_id` to actually enable event collection:
```cpp
// OA report format of the GPU generation: I915_OA_FORMAT_A32u40_A4u32_B8_C8
// on Gen12, I915_OA_FORMAT_A24u40_A14u32_B8_C8 on Xe-HP and newer (e.g. PVC).
// i915 rejects a format the GPU doesn't support with EINVAL.
uint64_t properties[] = {
  DRM_I915_PERF_PROP_SAMPLE_OA, 1,
  DRM_I915_PERF_PROP_OA_METRICS_SET, static_cast<uint64_t>(config_id),
  DRM_I915_PERF_PROP_OA_FORMAT, I915_OA_FORMAT_A24u40_A14u32_B8_C8 };

struct drm_i915_perf_open_param param = { 0, };
param.flags = 0;
param.flags |= I915_PERF_FLAG_FD_CLOEXEC;
param.flags |= I915_PERF_FLAG_FD_NONBLOCK;
param.properties_ptr = reinterpret_cast<uint64_t>(properties);
param.num_properties = sizeof(properties) / (2 * sizeof(uint64_t));

int stream_fd = SendIoctl(fd, DRM_IOCTL_I915_PERF_OPEN, &param);
```
Starting from this moment PM register will be incremented every cycle target event happened.

To stop event collection and release stream configuration one have to close stream descriptor:
```cpp
close(stream_fd);
drmClose(fd);
```

#### xe Kernel Driver
The xe driver (Linux 6.11+) provides the same functionality through the `DRM_IOCTL_XE_OBSERVATION` ioctl of [xe_drm.h](https://github.com/torvalds/linux/blob/master/include/uapi/drm/xe_drm.h). Open DRM interface for xe driver:
```cpp
int fd = drmOpenWithType("xe", NULL, DRM_NODE_RENDER);
if (fd < 0) {
  fd = drmOpenWithType("xe", NULL, DRM_NODE_PRIMARY);
}
assert(fd >= 0);
```
Add the PM register configuration (same register offset, value and GUID as for i915):
```cpp
std::vector<uint32_t> regs(2);
regs[0] = PERF_REG_OFFSET;
regs[1] = reg_value; // combined filters + event_id value

struct drm_xe_oa_config config = {};
memcpy(config.uuid, guid.c_str(), sizeof(config.uuid));
config.n_regs = static_cast<uint32_t>(regs.size() / 2);
config.regs_ptr = reinterpret_cast<uint64_t>(regs.data());

struct drm_xe_observation_param param = {};
param.observation_type = DRM_XE_OBSERVATION_TYPE_OA;
param.observation_op = DRM_XE_OBSERVATION_OP_ADD_CONFIG;
param.param = reinterpret_cast<uint64_t>(&config);

config_id = SendIoctl(fd, DRM_IOCTL_XE_OBSERVATION, &param);
```
Then open the stream. xe takes the stream properties as a chain of `drm_xe_ext_set_property` extensions, and the OA report format is encoded from its type, counter select, counter size and BC report fields:
```cpp
// Xe2 and newer (e.g. BMG, LNL) use PEC formats, e.g. PEC64u64:
uint64_t format = DRM_XE_OA_FMT_TYPE_PEC | (1 << 8) | (1 << 16);
// Xe-HP / Xe-HPC (e.g. DG2, PVC) use the OAG format A24u40_A14u32_B8_C8:
// uint64_t format = DRM_XE_OA_FMT_TYPE_OAG | (5 << 8);
// xe rejects a format the GPU doesn't support with EINVAL.

const uint64_t properties[][2] = {
  {DRM_XE_OA_PROPERTY_SAMPLE_OA, 1},
  {DRM_XE_OA_PROPERTY_OA_METRIC_SET, static_cast<uint64_t>(config_id)},
  {DRM_XE_OA_PROPERTY_OA_FORMAT, format}};
const size_t count = sizeof(properties) / sizeof(properties[0]);

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

int stream_fd = SendIoctl(fd, DRM_IOCTL_XE_OBSERVATION, &param);
```
The stream is closed the same way as for i915.

#### Permissions
Both drivers allow these calls only for root by default. To allow other users, set the sysctl of the driver in use to `0` (it is reset on reboot):
- i915 kernel driver:
```sh
echo "0" | sudo tee /proc/sys/dev/i915/perf_stream_paranoid
```
- xe kernel driver:
```sh
echo "0" | sudo tee /proc/sys/dev/xe/observation_paranoid
```
### Event Read
Since PM register is accessible only from GPU kernel, it's impossible to read it directly from host side. One of the possible ways to read PM results is binary instrumentation with Graphics Technology Pin (GT Pin): its code generator interface (`IGtGenCoder::StartPerfmon`/`StopPerfmon` in `api/igt_gen_coder.h`, GT Pin 5.0.0) emits instructions that read the PM register around the instrumented code. Reading PM results is not covered by the samples in this repository and is not tested.

Refer to Graphics Technology Pin (GT Pin) [chapter](../binary_instrumentation/GTPin.md) to learn more on binary instrumentation techniques.

## Usage Details
- refer to Programmer's Reference Manual for Intel(R) Processor Graphics (Kaby Lake), [Volume 14: Observability](https://cdrdv2.intel.com/v1/dl/getContent/723971) to learn more on performance monitoring (PM) register
- refer to Graphics Technology Pin (GT Pin) Binary Instrumentation [chapter](../binary_instrumentation/GTPin.md) to learn more on the ways of reading PM events

## Samples
- [GPU PerfMon Set](../../samples/gpu_perfmon_set)