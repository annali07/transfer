/*===--------- dpaintrin.h - Header file for all DPA intrinsics -----------===//
 *
 * NVIDIA_COPYRIGHT_BEGIN
 *
 * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 *
 * NVIDIA_COPYRIGHT_END
 */

#ifndef __DPAINTRIN_H
#define __DPAINTRIN_H

/* User needs to define following macro to use DPA intrinsics  */
#ifndef DPA_INTRIN_VERSION_USED
#define DPA_INTRIN_VERSION_USED (DPA_INTRIN_VERSION(1, 3))
#endif

#if (DPA_INTRIN_VERSION_USED == (DPA_INTRIN_VERSION(1, 3)))

#if defined(__riscv_xfenceheap)

#define __dpa_thread_fence(MEMORY_SPACE, PRED_OP, SUCC_OP) \
        __dpa_thread_fence_internal_1_3(MEMORY_SPACE, PRED_OP, SUCC_OP);

#define __DPA_HEAP   __MSPACE_HEAP
#define __DPA_MEMORY __MSPACE_MEMORY
#define __DPA_MMIO   __MSPACE_MMIO
#define __DPA_SYSTEM __MSPACE_SYSTEM

#define __DPA_R  __MOP_R
#define __DPA_W  __MOP_W
#define __DPA_RW __MOP_RW

#define __dpa_thread_memory_fence(OP1,OP2) __dpa_thread_fence(__DPA_MEMORY,OP1,OP2)
#define __dpa_thread_outbox_fence(OP1,OP2) __dpa_thread_fence(__DPA_MMIO,OP1,OP2)
#define __dpa_thread_window_fence(OP1,OP2) __dpa_thread_fence(__DPA_MMIO,OP1,OP2)

#define __dpa_thread_system_fence() __dpa_thread_fence(__DPA_SYSTEM,__DPA_RW,__DPA_RW)
#define __dpa_thread_window_read_inv() __dpa_thread_fence(__DPA_MMIO,__DPA_R,__DPA_R)
#define __dpa_thread_window_writeback() __dpa_thread_fence(__DPA_MMIO,__DPA_W,__DPA_W)
#define __dpa_thread_memory_writeback() __dpa_thread_fence(__DPA_MEMORY,__DPA_W,__DPA_W)

#endif // __riscv_xfenceheap

#if defined(__riscv_xrpfxp)
#define __dpa_fxp_rcp(OP1) __dpa_fxp_rcp_internal_1_3(OP1)
#define __dpa_fxp_pow2(OP1) __dpa_fxp_pow2_internal_1_3(OP1)
#define __dpa_fxp_log2(OP1) __dpa_fxp_log2_internal_1_3(OP1)
#endif // __riscv_xrpfxp

#if defined(__riscv_xnvcc)
#define __dpa_data_ignore(ADDR) __dpa_data_ignore_internal_1_3(ADDR)
#endif // __riscv_xnvcc

#define __dpa_thread_cycles() __dpa_thread_cycles_internal_1_3()
#define __dpa_thread_inst_ret() __dpa_thread_inst_ret_internal_1_3()
#define __dpa_thread_time() __dpa_thread_time_internal_1_3()

#else

#error Bad value for DPA_INTRIN_VERSION_USED

#endif // DPA_INTRIN_VERSION_USED

#endif // __DPAINTRIN_H
