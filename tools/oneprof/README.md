# GPU Metrics Collection Tool for Data Parallel C++ (DPC++)
## Overview
This tool provides GPU hardware metrics collection capabilities for the compute applications based on Intel runtimes for OpenCL(TM) and Level Zero, like DPC++, Intel(R) Implicit SPMD Program Compiler (Intel(R) ISPC) and OpenMP* GPU offload programs.

The following capabilities are available:
```
Usage: ./oneprof[.exe] [options] <application> <args>
Options:
--raw-metrics [-m]               Collect raw metric stream for the device
--kernel-intervals [-i]          Collect raw kernel intervals for the device
--kernel-metrics [-k]            Collect over-time metrics for each kernel instance
--aggregation [-a]               Collect aggregated metrics for each kernel instance in time-based mode
--kernel-query [-q]              Collect aggregated metrics for each kernel instance in query-based mode
--stall-sampling [-e]            Collect EU stall sampling: stall reasons for each kernel and its hottest instructions (Level Zero and OpenCL kernels)
--device [-d] <ID>               Target device for profiling (default is 0)
--group [-g] <NAME>              Target metric group to collect (default is ComputeBasic)
--sampling-interval [-s] <VALUE> Sampling interval for metrics collection in us (default is 1000 us)
--output [-o] <FILENAME>         Print console logs into the file
--raw-data-path [-p] <DIRECTORY> Path to store raw metic data into (default is process folder)
--finalize [-f] <FILENAME>       Print output from collected result file
--no-finalize                    Do not finalize and do not report collection results
--device-list                    Print list of available devices
--metric-list                    Print list of available metrics
--version                        Print version
```

**Raw Metrics** mode allows to grab all the HW metric reports while application execution and dump them in CSV format for further investigation (doesn't depend on compute runtime), e.g.:
```
== Raw Metrics ==

SubDeviceId,GpuTime[ns],GpuCoreClocks[cycles],AvgGpuCoreFrequencyMHz[MHz],ResultUncertainty[%],GPU_BUSY[%],IA_VERTEX[events],GPGPU_THREADGROUP_COUNT[events],ASYNC_GPGPU_THREADGROUP_COUNT[events],RASTERIZER_SAMPLE_OUTPUT[events],ICACHE_HIT[events],ICACHE_MISS[events],XVE_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL[events],XVE_INST_EXECUTED_ALU1_ALL[events],XVE_INST_EXECUTED_SEND_ALL[events],XVE_INST_ISSUED_ALL[events],XVE_SHARED_FUNCTION_ACCESS_HOLD[%],XVE_STALL[%],XVE_THREADS_OCCUPANCY_ALL[%],XVE_INST_EXECUTED_ALU2_ALL[events],XVE_MULTIPLE_PIPE_ACTIVE[%],LOAD_STORE_CACHE_PARTIAL_WRITE_COUNT[events],SLM_BANK_CONFLICT_COUNT[events],SLM_BYTE_READ[bytes],SLM_BYTE_WRITE[bytes],LOAD_STORE_CACHE_BYTE_READ[bytes],LOAD_STORE_CACHE_BYTE_WRITE[bytes],LOAD_STORE_CACHE_ACCESS[events],LOAD_STORE_CACHE_HIT[events],L3_ATOMIC_ACCESS[events],L3_HIT[events],L3_MISS[events],L3_READ[events],L3_WRITE[events],L3_STALL[%],COMPRESSOR_INPUT[events],COMPRESSOR_OUTPUT[events],GPU_MEMORY_BYTE_READ[bytes],GPU_MEMORY_BYTE_WRITE[bytes],GPU_MEMORY_BYTE_READ_RATE[GBpS],GPU_MEMORY_BYTE_WRITE_RATE[GBpS],GPU_MEMORY_L3_READ[events],GPU_MEMORY_L3_WRITE[events],GPU_MEMORY_REQUEST_QUEUE_FULL[%],TLB_MISS[events],ASYNC_GPGPU_THREAD_EXIT_COUNT[messages],GPGPU_DISPATCH[%],COMMAND_PARSER_COMPUTE_ENGINE_BUSY[%],COMMAND_PARSER_COMPUTE_ENGINE_DISPATCH_KERNEL_COUNT[events],COMMAND_PARSER_COPY_ENGINE_BUSY[%],COMMAND_PARSER_FLUSH_COUNT[events],COMMAND_PARSER_RENDER_ENGINE_BUSY[%],COMMAND_PARSER_RENDER_ENGINE_DISPATCH_KERNEL_COUNT[events],XVE_PIPE_ALU0_AND_ALU1_ACTIVE[%],XVE_PIPE_ALU0_AND_ALU2_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU1_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU2_ALL_UTILIZATION[%],HOST_TO_GPUMEM_TRANSACTION_READ[events],HOST_TO_GPUMEM_TRANSACTION_WRITE[events],SYSMEM_TRANSACTION_READ[events],SYSMEM_TRANSACTION_WRITE[events],QueryBeginTime[ns],CoreFrequencyMHz[MHz],ReportReason,ContextIdValid,ContextId,SourceId,StreamMarker,
0,853333,1023936,1199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,11518317653333,1199,1,0,0,0,0,
0,853333,1023936,1199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,11518318506666,1199,1,0,0,0,0,
0,853333,1023936,1199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,11518319360000,1199,1,0,0,0,0,
0,853333,1023936,1199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,11518320213333,1199,1,0,0,0,0,
...
```

**Kernel Intervals** mode dumps execution intervals for all of the kernels and transfers on the device to be able to correlate raw metrics with kernels, e.g.:
```
== Raw Kernel Intervals ==

Kernel,SubDeviceId,Time[ns],Start[ns],End[ns],
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],0,8486458,11521201187604,11521209674062,

[INFO] Result file is result.144526.bin
```

**Kernel Metrics** mode automatically correlates metric reports to kernels giving an ability to track metrics bevahiour for a kernel over time:
```
== Kernel Metrics ==

Kernel,SubDeviceId,GpuTime[ns],GpuCoreClocks[cycles],AvgGpuCoreFrequencyMHz[MHz],ResultUncertainty[%],GPU_BUSY[%],IA_VERTEX[events],GPGPU_THREADGROUP_COUNT[events],ASYNC_GPGPU_THREADGROUP_COUNT[events],RASTERIZER_SAMPLE_OUTPUT[events],ICACHE_HIT[events],ICACHE_MISS[events],XVE_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL[events],XVE_INST_EXECUTED_ALU1_ALL[events],XVE_INST_EXECUTED_SEND_ALL[events],XVE_INST_ISSUED_ALL[events],XVE_SHARED_FUNCTION_ACCESS_HOLD[%],XVE_STALL[%],XVE_THREADS_OCCUPANCY_ALL[%],XVE_INST_EXECUTED_ALU2_ALL[events],XVE_MULTIPLE_PIPE_ACTIVE[%],LOAD_STORE_CACHE_PARTIAL_WRITE_COUNT[events],SLM_BANK_CONFLICT_COUNT[events],SLM_BYTE_READ[bytes],SLM_BYTE_WRITE[bytes],LOAD_STORE_CACHE_BYTE_READ[bytes],LOAD_STORE_CACHE_BYTE_WRITE[bytes],LOAD_STORE_CACHE_ACCESS[events],LOAD_STORE_CACHE_HIT[events],L3_ATOMIC_ACCESS[events],L3_HIT[events],L3_MISS[events],L3_READ[events],L3_WRITE[events],L3_STALL[%],COMPRESSOR_INPUT[events],COMPRESSOR_OUTPUT[events],GPU_MEMORY_BYTE_READ[bytes],GPU_MEMORY_BYTE_WRITE[bytes],GPU_MEMORY_BYTE_READ_RATE[GBpS],GPU_MEMORY_BYTE_WRITE_RATE[GBpS],GPU_MEMORY_L3_READ[events],GPU_MEMORY_L3_WRITE[events],GPU_MEMORY_REQUEST_QUEUE_FULL[%],TLB_MISS[events],ASYNC_GPGPU_THREAD_EXIT_COUNT[messages],GPGPU_DISPATCH[%],COMMAND_PARSER_COMPUTE_ENGINE_BUSY[%],COMMAND_PARSER_COMPUTE_ENGINE_DISPATCH_KERNEL_COUNT[events],COMMAND_PARSER_COPY_ENGINE_BUSY[%],COMMAND_PARSER_FLUSH_COUNT[events],COMMAND_PARSER_RENDER_ENGINE_BUSY[%],COMMAND_PARSER_RENDER_ENGINE_DISPATCH_KERNEL_COUNT[events],XVE_PIPE_ALU0_AND_ALU1_ACTIVE[%],XVE_PIPE_ALU0_AND_ALU2_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU1_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU2_ALL_UTILIZATION[%],HOST_TO_GPUMEM_TRANSACTION_READ[events],HOST_TO_GPUMEM_TRANSACTION_WRITE[events],SYSMEM_TRANSACTION_READ[events],SYSMEM_TRANSACTION_WRITE[events],QueryBeginTime[ns],CoreFrequencyMHz[MHz],ReportReason,ContextIdValid,ContextId,SourceId,StreamMarker,
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],0,533802,640461,1199,0,99.998,0,0,72,0,7798725,1026,82.3449,4168431,68792832,16671711,45874664,0.0255296,15.4526,97.6387,0,1.77102,0,0,0,0,283313904,0,8335051,6245871,0,2090806,1484,2089951,2378,0,22,1,94208,1024,0.176485,0.00191831,1429,28,0,197,1152,0.219842,99.9978,1,99.998,5,0,0,1.77099,0,4.51978,74.5913,0,467,58,1,11,11524057173333,1199,1,0,0,0,0,
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],0,853333,1023936,1199,0,100,0,0,108,0,15763730,0,83.6716,6790413,112045611,27161758,74715520,0.0020638,16.1612,98.5568,0,1.50675,0,0,0,0,461390424,0,13577350,7606106,0,5971928,2150,5967147,7006,0,0,0,137728,0,0.1614,0,2150,0,0,14,3456,0.206263,100,0,100,0,0,0,1.50675,0,4.60533,75.9905,0,733,94,0,9,11524058026666,1199,1,0,0,0,0,
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],0,853333,1023936,1199,0,100.003,0,0,108,0,15875758,0,83.8903,6805983,112302220,27224005,74886536,0.00215739,15.955,98.9639,0,1.51268,0,0,0,0,462449432,0,13608607,7126942,0,6482605,2152,6477728,7006,0,0,0,137216,0,0.1608,0,2152,0,0,5,3456,0.206263,100.003,0,100.003,0,0,0,1.51268,0,4.61589,76.1646,0,733,94,0,9,11524058880000,1199,1,0,0,0,0,
...
```

**Aggregation** mode provides a single metric report for a kernel to see cumulative results. These are results of automatic aggregation for the metrics, collected in time-based mode:
```
== Aggregated Kernel Metrics ==

Kernel,SubDeviceId,KernelTime[ns],GpuTime[ns],GpuCoreClocks[cycles],AvgGpuCoreFrequencyMHz[MHz],ResultUncertainty[%],GPU_BUSY[%],IA_VERTEX[events],GPGPU_THREADGROUP_COUNT[events],ASYNC_GPGPU_THREADGROUP_COUNT[events],RASTERIZER_SAMPLE_OUTPUT[events],ICACHE_HIT[events],ICACHE_MISS[events],XVE_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL[events],XVE_INST_EXECUTED_ALU1_ALL[events],XVE_INST_EXECUTED_SEND_ALL[events],XVE_INST_ISSUED_ALL[events],XVE_SHARED_FUNCTION_ACCESS_HOLD[%],XVE_STALL[%],XVE_THREADS_OCCUPANCY_ALL[%],XVE_INST_EXECUTED_ALU2_ALL[events],XVE_MULTIPLE_PIPE_ACTIVE[%],LOAD_STORE_CACHE_PARTIAL_WRITE_COUNT[events],SLM_BANK_CONFLICT_COUNT[events],SLM_BYTE_READ[bytes],SLM_BYTE_WRITE[bytes],LOAD_STORE_CACHE_BYTE_READ[bytes],LOAD_STORE_CACHE_BYTE_WRITE[bytes],LOAD_STORE_CACHE_ACCESS[events],LOAD_STORE_CACHE_HIT[events],L3_ATOMIC_ACCESS[events],L3_HIT[events],L3_MISS[events],L3_READ[events],L3_WRITE[events],L3_STALL[%],COMPRESSOR_INPUT[events],COMPRESSOR_OUTPUT[events],GPU_MEMORY_BYTE_READ[bytes],GPU_MEMORY_BYTE_WRITE[bytes],GPU_MEMORY_BYTE_READ_RATE[GBpS],GPU_MEMORY_BYTE_WRITE_RATE[GBpS],GPU_MEMORY_L3_READ[events],GPU_MEMORY_L3_WRITE[events],GPU_MEMORY_REQUEST_QUEUE_FULL[%],TLB_MISS[events],ASYNC_GPGPU_THREAD_EXIT_COUNT[messages],GPGPU_DISPATCH[%],COMMAND_PARSER_COMPUTE_ENGINE_BUSY[%],COMMAND_PARSER_COMPUTE_ENGINE_DISPATCH_KERNEL_COUNT[events],COMMAND_PARSER_COPY_ENGINE_BUSY[%],COMMAND_PARSER_FLUSH_COUNT[events],COMMAND_PARSER_RENDER_ENGINE_BUSY[%],COMMAND_PARSER_RENDER_ENGINE_DISPATCH_KERNEL_COUNT[events],XVE_PIPE_ALU0_AND_ALU1_ACTIVE[%],XVE_PIPE_ALU0_AND_ALU2_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU1_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU2_ALL_UTILIZATION[%],HOST_TO_GPUMEM_TRANSACTION_READ[events],HOST_TO_GPUMEM_TRANSACTION_WRITE[events],SYSMEM_TRANSACTION_READ[events],SYSMEM_TRANSACTION_WRITE[events],QueryBeginTime[ns],CoreFrequencyMHz[MHz],ReportReason,ContextIdValid,ContextId,SourceId,StreamMarker,
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],0,8180312,7891819,9856197,1252,0,100,0,0,1015,0,151038776,1026,83.7422,65372638,1078692630,261488955,719302024,0.00322611,15.9456,98.8753,0,1.52587,0,0,0,0,4442062580,0,130713361,69416010,0,61305674,14282,61256334,63693,0,114,10,889344,38912,1.1912,0.0510514,14023,283,0,292,31385,0.201356,100,1,66.2233,5,0,0,1.52587,0,4.606,76.0021,0,6800,870,2,95,11526936320000,1199,1,0,0,0,0,

[INFO] Result file is result.145601.bin
```

**Kernel Query** mode also provides a single metric report for a kernel instance, but it is based on query mode:
```
== Query Kernel Metrics ==

Kernel,SubDeviceId,GpuTime[ns],GpuCoreClocks[cycles],AvgGpuCoreFrequencyMHz[MHz],ResultUncertainty[%],GPU_BUSY[%],IA_VERTEX[events],GPGPU_THREADGROUP_COUNT[events],ASYNC_GPGPU_THREADGROUP_COUNT[events],RASTERIZER_SAMPLE_OUTPUT[events],ICACHE_HIT[events],ICACHE_MISS[events],XVE_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL[events],XVE_INST_EXECUTED_ALU1_ALL[events],XVE_INST_EXECUTED_SEND_ALL[events],XVE_INST_ISSUED_ALL[events],XVE_SHARED_FUNCTION_ACCESS_HOLD[%],XVE_STALL[%],XVE_THREADS_OCCUPANCY_ALL[%],XVE_INST_EXECUTED_ALU2_ALL[events],XVE_MULTIPLE_PIPE_ACTIVE[%],LOAD_STORE_CACHE_PARTIAL_WRITE_COUNT[events],SLM_BANK_CONFLICT_COUNT[events],SLM_BYTE_READ[bytes],SLM_BYTE_WRITE[bytes],LOAD_STORE_CACHE_BYTE_READ[bytes],LOAD_STORE_CACHE_BYTE_WRITE[bytes],LOAD_STORE_CACHE_ACCESS[events],LOAD_STORE_CACHE_HIT[events],L3_ATOMIC_ACCESS[events],L3_HIT[events],L3_MISS[events],L3_READ[events],L3_WRITE[events],L3_STALL[%],COMPRESSOR_INPUT[events],COMPRESSOR_OUTPUT[events],GPU_MEMORY_BYTE_READ[bytes],GPU_MEMORY_BYTE_WRITE[bytes],GPU_MEMORY_BYTE_READ_RATE[GBpS],GPU_MEMORY_BYTE_WRITE_RATE[GBpS],GPU_MEMORY_L3_READ[events],GPU_MEMORY_L3_WRITE[events],GPU_MEMORY_REQUEST_QUEUE_FULL[%],TLB_MISS[events],ASYNC_GPGPU_THREAD_EXIT_COUNT[messages],GPGPU_DISPATCH[%],COMMAND_PARSER_COMPUTE_ENGINE_BUSY[%],COMMAND_PARSER_COMPUTE_ENGINE_DISPATCH_KERNEL_COUNT[events],COMMAND_PARSER_COPY_ENGINE_BUSY[%],COMMAND_PARSER_FLUSH_COUNT[events],COMMAND_PARSER_RENDER_ENGINE_BUSY[%],COMMAND_PARSER_RENDER_ENGINE_DISPATCH_KERNEL_COUNT[events],XVE_PIPE_ALU0_AND_ALU1_ACTIVE[%],XVE_PIPE_ALU0_AND_ALU2_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU1_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU2_ALL_UTILIZATION[%],HOST_TO_GPUMEM_TRANSACTION_READ[events],HOST_TO_GPUMEM_TRANSACTION_WRITE[events],SYSMEM_TRANSACTION_READ[events],SYSMEM_TRANSACTION_WRITE[events],QueryBeginTime[ns],CoreFrequencyMHz[MHz],CoreFrequencyChanged,QuerySplitOccurred,ReportId,ReportsCount,OverrunOccured,MidQueryTimer,MidQueryProgramming,MidQueryMarker,MidQueryCtxSwitch,MidQueryC6,MidQueryFreqChange,MidQueryMmioTrigger,ReportError,ReportLost,ReportInconsistent,ReportCtxSwitchLost,ReportWithoutWorkload,ReportContextMismatch,ReportQueryModeMismatch,
_ZTSZZL11RunAndCheckN4sycl3_V15queueERKSt6vectorIfSaIfEES6_RS4_jfENKUlRNS0_7handlerEE_clES9_E6__GEMM[SIMD32 {1; 1024; 1} {1024; 1; 1}],0,6409010,10253782,1599,0,100,0,0,1024,0,155148309,1026,82.733,67174400,1108410368,268697600,739114960,0.0042294,16.0771,97.7122,0,1.50261,0,0,0,0,4564451328,0,134316032,70487034,0,63838630,3248,63774207,67675,3.901e-05,272,34,62976,121344,0.00982617,0.0189333,1834,775,0,300,32768,0.195284,100,1,0,1,0,0,1.50261,0,4.54943,75.0679,0,4688,1042,3,18,11529872342447,1600,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

Kernel,SubDeviceId,GpuTime[ns],GpuCoreClocks[cycles],AvgGpuCoreFrequencyMHz[MHz],ResultUncertainty[%],GPU_BUSY[%],IA_VERTEX[events],GPGPU_THREADGROUP_COUNT[events],ASYNC_GPGPU_THREADGROUP_COUNT[events],RASTERIZER_SAMPLE_OUTPUT[events],ICACHE_HIT[events],ICACHE_MISS[events],XVE_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL[events],XVE_INST_EXECUTED_ALU1_ALL[events],XVE_INST_EXECUTED_SEND_ALL[events],XVE_INST_ISSUED_ALL[events],XVE_SHARED_FUNCTION_ACCESS_HOLD[%],XVE_STALL[%],XVE_THREADS_OCCUPANCY_ALL[%],XVE_INST_EXECUTED_ALU2_ALL[events],XVE_MULTIPLE_PIPE_ACTIVE[%],LOAD_STORE_CACHE_PARTIAL_WRITE_COUNT[events],SLM_BANK_CONFLICT_COUNT[events],SLM_BYTE_READ[bytes],SLM_BYTE_WRITE[bytes],LOAD_STORE_CACHE_BYTE_READ[bytes],LOAD_STORE_CACHE_BYTE_WRITE[bytes],LOAD_STORE_CACHE_ACCESS[events],LOAD_STORE_CACHE_HIT[events],L3_ATOMIC_ACCESS[events],L3_HIT[events],L3_MISS[events],L3_READ[events],L3_WRITE[events],L3_STALL[%],COMPRESSOR_INPUT[events],COMPRESSOR_OUTPUT[events],GPU_MEMORY_BYTE_READ[bytes],GPU_MEMORY_BYTE_WRITE[bytes],GPU_MEMORY_BYTE_READ_RATE[GBpS],GPU_MEMORY_BYTE_WRITE_RATE[GBpS],GPU_MEMORY_L3_READ[events],GPU_MEMORY_L3_WRITE[events],GPU_MEMORY_REQUEST_QUEUE_FULL[%],TLB_MISS[events],ASYNC_GPGPU_THREAD_EXIT_COUNT[messages],GPGPU_DISPATCH[%],COMMAND_PARSER_COMPUTE_ENGINE_BUSY[%],COMMAND_PARSER_COMPUTE_ENGINE_DISPATCH_KERNEL_COUNT[events],COMMAND_PARSER_COPY_ENGINE_BUSY[%],COMMAND_PARSER_FLUSH_COUNT[events],COMMAND_PARSER_RENDER_ENGINE_BUSY[%],COMMAND_PARSER_RENDER_ENGINE_DISPATCH_KERNEL_COUNT[events],XVE_PIPE_ALU0_AND_ALU1_ACTIVE[%],XVE_PIPE_ALU0_AND_ALU2_ACTIVE[%],XVE_INST_EXECUTED_ALU0_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU1_ALL_UTILIZATION[%],XVE_INST_EXECUTED_ALU2_ALL_UTILIZATION[%],HOST_TO_GPUMEM_TRANSACTION_READ[events],HOST_TO_GPUMEM_TRANSACTION_WRITE[events],SYSMEM_TRANSACTION_READ[events],SYSMEM_TRANSACTION_WRITE[events],QueryBeginTime[ns],CoreFrequencyMHz[MHz],CoreFrequencyChanged,QuerySplitOccurred,ReportId,ReportsCount,OverrunOccured,MidQueryTimer,MidQueryProgramming,MidQueryMarker,MidQueryCtxSwitch,MidQueryC6,MidQueryFreqChange,MidQueryMmioTrigger,ReportError,ReportLost,ReportInconsistent,ReportCtxSwitchLost,ReportWithoutWorkload,ReportContextMismatch,ReportQueryModeMismatch,
...
```

***Note, that metric names provided by the tool may be not obvious.*** To learn what each metric exactly means, use `--metric-list` option. E.g. `Throughput` metrics may be represented in bytes, and to calculate actual throughput  one should devide it to `GpuTime`:
```
Group 1: ComputeBasic (ComputeBasic)
    Metric 0: GpuTime[ns] (Time elapsed on the GPU during the measurement.) [UINT64, EVENT, ComputeBasic]
    ...
    Metric 67: SourceId (SourceId) [UINT64, RAW, ComputeBasic]
    Metric 68: StreamMarker (Stream marker value.) [UINT64, RAW, ComputeBasic]
    ...
```

**EU Stall Sampling** mode (`-e`) samples the instruction pointers of the EU threads and the reason each thread is stalled (the time-based `EuStallSampling` metric group). The samples are summed for each kernel, with each stall reason as a percentage of the kernel's samples, and for each kernel's ten hottest instructions (the offset in the kernel, the most frequent reason, and the instruction disassembled with IGA), e.g. for [ze_gemm](../../samples/ze_gemm) on Intel(R) Arc(TM) B570 Graphics:
```
== EU Stall Sampling ==

Kernel,Samples,Active[%],PSDepStall[%],ControlStall[%],PipeStall[%],SendStall[%],DistStall[%],SbidStall[%],SyncStall[%],InstrFetchStall[%],OtherStall[%],
GEMM,5182,53.88,0.00,0.68,25.30,0.00,3.55,14.90,0.00,1.68,0.02,
<unknown>,658,0.46,0.00,0.00,0.15,0.00,0.15,99.09,0.00,0.15,0.00,

== Hot Instructions: GEMM ==

Offset,Samples,Top Reason,Instruction,
0x2c8,772,SbidStall,"sync.nop null {Compacted,$4.dst}",
0x210,274,Active,"mov (16|M16) r42.0<2>:ud r17.0<1;1,0>:ud {Compacted}",
0x2b8,273,Active,"(W) cmp (32|M0) (lt)f1.0 null<1>:d r4.8<0;1,0>:d r5.4<0;1,0>:d {I@1}",
...
```
- Kernels created through Level Zero (including DPC++ and OpenMP offload on Level Zero) and OpenCL kernels enqueued on the target device are mapped; samples in other code (e.g. kernels the runtime creates internally) are reported as `<unknown>`.
- OpenCL kernels are mapped only with compute runtime 25.48 or newer: older versions report a GPU address that is not the one the kernel runs from, so oneprof doesn't map OpenCL kernels there (an `[INFO]` line says so) and their samples are reported as `<unknown>`.
- `-e` always uses the `EuStallSampling` group and can be combined with `-m` (raw stall reports) and `-i`, not with `-k`, `-a`, `-q` or another `-g`. `-s` sets the sampling interval.
- The group exists on both tested GPUs. With the xe kernel driver it is listed only when `/proc/sys/dev/xe/observation_paranoid` is 0; if the device has no such group, oneprof lists the available groups and exits.
- The instruction disassembly needs the IGA library (`libiga64`, part of the compute runtime) at build time; without it (and on Windows) the `Instruction` column is empty.

## Supported OS
- Linux
- Windows (*under development*)

## Tested Configurations
- Devices: Intel(R) Data Center GPU Max 1550 (i915 kernel driver), Intel(R) Arc(TM) B570 and Arc(TM) Pro B70 Graphics (xe kernel driver), and an Arrow Lake iGPU (i915 kernel driver); other devices are not tested
- Software: the versions listed in [SOFTWARE](../../SOFTWARE)
- Metrics (OA) collection: tested with both kernel drivers
- On the Arrow Lake iGPU (compute runtime 26.27), the query mode (`-q`) fails in the runtime, and there is no `EuStallSampling` group (`-e` is not available)

## Prerequisites
- [CMake](https://cmake.org/) (version 3.22 and above)
- [Git](https://git-scm.com/) (version 1.8 and above)
- [Python](https://www.python.org/) (version 3.9 and above)
- [OpenCL(TM) ICD Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
- [oneAPI Level Zero loader](https://github.com/oneapi-src/level-zero)
- [Intel(R) Graphics Compute Runtime for oneAPI Level Zero and OpenCL(TM) Driver](https://github.com/intel/compute-runtime)
- [Intel(R) Metrics Discovery Application Programming Interface](https://github.com/intel/metrics-discovery)
    - one may need to install `libdrm-dev` package to build the library from sources
    - one may need to allow metrics collection for non-root users:
        ```sh
        echo 0 | sudo tee /proc/sys/dev/i915/perf_stream_paranoid  # i915 kernel driver (e.g. PVC)
        echo 0 | sudo tee /proc/sys/dev/xe/observation_paranoid    # xe kernel driver (e.g. BMG)
        ```
- [Metrics Library for Metrics Discovery API (Metrics Library for MD API)](https://github.com/intel/metrics-library)
- Optional, for the instruction disassembly in `--stall-sampling` (Linux): the IGA library (`libiga64`) of the [Intel(R) Graphics Compiler](https://github.com/intel/intel-graphics-compiler); the IGA headers are downloaded at build time if they are not installed

## Build and Run
### Linux
Run the following commands to build the sample:
```sh
cd <pti>/tools/oneprof
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Use this command line to run the tool:
```sh
./oneprof [options] <target_application>
```
One may use e.g. [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
./oneprof -a -s 100 -g ComputeBasic ../../../samples/dpc_gemm/build/dpc_gemm
```
The metric groups differ between GPUs: `ComputeBasic` exists on both tested GPUs, and `./oneprof --metric-list` (or `gpuinfo -m`) lists the groups of the current GPU.
### Windows
Use Microsoft* Visual Studio x64 command prompt to run the following commands and build the sample:
```sh
cd <pti>\tools\oneprof
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_LIBRARY_PATH=<opencl_icd_lib_path> ..
nmake
```
Use this command line to run the tool:
```sh
oneprof.exe [options] <target_application>
```
One may use e.g. [dpc_gemm](../../samples/dpc_gemm) as target application, e.g.:
```sh
oneprof.exe -a -s 100 -g ComputeBasic ..\..\..\samples\dpc_gemm\build\dpc_gemm.exe
```
