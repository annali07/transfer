/*
 * Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
 *
 * This software product is a proprietary product of NVIDIA CORPORATION &
 * AFFILIATES (the "Company") and all right, title, and interest in and to the
 * software product, including all associated intellectual property rights, are
 * and shall remain exclusively with the Company.
 *
 * This software product is governed by the End User License Agreement
 * provided with the software product.
 *
 */

/**
 * @defgroup DPA_DEVICE DPA Device
 * DOCA DPA Device library. For more details please refer to the user guide on DOCA devzone.
 *
 * @ingroup DPA
 *
 * @{
 */

#ifndef DOCA_DPA_DEV_H_
#define DOCA_DPA_DEV_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief declares that we are compiling for the DPA Device
 *
 * @note Must be defined before the first API use/include of DOCA
 */
#define DOCA_DPA_DEVICE
#include <doca_compat.h>

#ifdef __cplusplus
extern "C" {
#endif

struct doca_dpa_dev_sync_event_remote;
/** DPA remote sync event type definition */
typedef struct doca_dpa_dev_sync_event_remote doca_dpa_dev_sync_event_remote_t;

/** DPA memory handle type definition */
__dpa_global__ typedef uint64_t doca_dpa_dev_mem_t;
/** DPA pointer type definition */
__dpa_global__ typedef uint64_t doca_dpa_dev_uintptr_t;
/** DPA endpoint handle type definition */
__dpa_global__ typedef uint64_t doca_dpa_dev_ep_t;

/**
 * \brief Obtains the thread rank
 *
 * Retrieves the thread rank for a given kernel on the DPA.
 * The function returns a number in {0..N-1}, where N is the number of threads requested for launch during a kernel
 * submission
 *
 * @return
 * Returns the thread rank.
 */
__DOCA_EXPERIMENTAL
unsigned int doca_dpa_dev_thread_rank(void);

/**
 * \brief Obtains the number of threads running the kernel
 *
 * Retrieves the number of threads assigned to a given kernel. This is the value `nthreads` that was passed in to
 * 'doca_dpa_kernel_launch_update_set/doca_dpa_kernel_launch_update_add'
 *
 * @return
 * Returns the number of threads running the kernel
 */
__DOCA_EXPERIMENTAL
unsigned int doca_dpa_dev_num_threads(void);

/**
 * \brief Get remote memory key of registered memory
 *
 * This function returns the remote memory key for a memory region that
 * was registered with `doca_dpa_mem_host_register`.
 * The memory region must have been provided remote access capabilities
 *
 * @param mem [in] - Memory to obtain the remote key for
 *
 * @return
 * The function returns the rkey. If the memory region didn’t have remote memory access permissions set, then the
 * function returns an undefined value (regardless, the memory cannot be accessed remotely)
 */
__DOCA_EXPERIMENTAL
uint32_t doca_dpa_dev_mem_rkey_get(doca_dpa_dev_mem_t mem);

/**
 * \brief Initiate a copy data locally from Host
 *
 * This function copies data between two memory regions. The destination buffer, specified by `dest_addr` and `length`
 * will contain the copied data after the memory copy is complete. This is a non-blocking routine
 *
 * @param dest_addr [in] - Buffer to read into
 * @param dest_mem [in] - Memory handle for destination buffer
 * @param src_addr [in] - Host virtual address
 * @param src_mem [in] - Memory handle for source buffer
 * @param length [in] - Size of buffer
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_memcpy_nb(uint64_t dest_addr,
			    doca_dpa_dev_mem_t dest_mem,
			    uint64_t src_addr,
			    doca_dpa_dev_mem_t src_mem,
			    size_t length);

/**
 * \brief Initiate a transpose locally from Host
 *
 * This function transposes a 2D array. The destination buffer, specified by `dest_addr` and `length` will contain the
 * copied data after the operation is complete. This is a non-blocking routine
 *
 * @param dest_addr [in] - Buffer to transpose into
 * @param dest_mem [in] - Memory handle for destination buffer
 * @param src_addr [in] - Buffer to transpose from
 * @param src_mem [in] - Memory handle for source buffer
 * @param length [in] - Size of buffer
 * @param element_size [in] - Size of datatype of one element
 * @param num_columns [in] - Number of columns in 2D array
 * @param num_rows [in] - Number of rows in 2D array
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_memcpy_transpose2D_nb(uint64_t dest_addr,
					doca_dpa_dev_mem_t dest_mem,
					uint64_t src_addr,
					doca_dpa_dev_mem_t src_mem,
					size_t length,
					size_t element_size,
					size_t num_columns,
					size_t num_rows);

/**
 * \brief Wait for all memory copy operations issued previously to complete
 *
 * This function returns when memory copy operations issued on this thread have been completed.
 * After this call returns, the buffers they referenced by the copy operations can be reused. This call is blocking
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_memcpy_synchronize(void);

/**
 * \brief Obtain a pointer to externally allocated memory
 *
 * This function allows the DPA process to obtain a pointer to external memory that was registered on the Host using
 * `doca_dpa_mem_host_register`. The obtained pointer can be used to load/store data directly from the DPA kernel.
 * The memory being accessed through the `dev_ptr` is subject to 64B alignment restriction
 *
 * @param ext_addr [in] - External address being accessed
 * @param mem [in] - Memory handle representing external memory
 * @param dev_ptr [out] - Device address pointing to external address
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_external_ptr_get(uint64_t ext_addr, doca_dpa_dev_mem_t mem, doca_dpa_dev_uintptr_t *dev_ptr);

/**
 * \brief Initiate remote transfer of data on an endpoint
 *
 * This function writes data to a remote peer that is connected with endpoint `ep`.
 * The local buffer, specified by `local_addr` is not ready to be reused after the call returns.
 * Buffers can only be reused after a call to `doca_dpa_dev_ep_synchronize`.
 * This is a non-blocking communication routine. The function may yield the DPA thread internally
 *
 * @param ep [in] - Endpoint to use
 * @param local_addr [in] - Local buffer to send
 * @param length [in] - Length of local buffer
 * @param local_mem [in] - Memory handle for local memory
 * @param raddr [in] - Remote virtual address
 * @param rkey [in] - Access key for remote buffer
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_put_nb(doca_dpa_dev_ep_t ep,
			 uint64_t local_addr,
			 size_t length,
			 doca_dpa_dev_mem_t local_mem,
			 uint64_t raddr,
			 uint32_t rkey);

/**
 * \brief Initiate remote transfer of data and notification on endpoint
 *
 * This function writes data to a remote peer that is connected with endpoint `ep`.
 * The local buffer, specified by `local_addr` is not ready to be reused after the call returns.
 * Buffers can only be reused after a call to `doca_dpa_dev_ep_synchronize`.
 * This is a non-blocking communication routine. The function may yield the DPA thread internally.
 * The data written by this function is visible on the remote side after the completion `event` has been written.
 * The comp count is atomically added to the event
 *
 * @param ep [in] - Endpoint to use
 * @param local_addr [in] - Local buffer to send
 * @param length [in] - Length of local buffer
 * @param local_mem [in] - Memory handle for local memory
 * @param raddr [in] - Remote virtual address
 * @param rkey [in] - Access key for remote buffer
 * @param event [in] - Remote event to update the count of
 * @param comp_count [in] - Value to add to the event count
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_put_signal_add_nb(doca_dpa_dev_ep_t ep,
				uint64_t local_addr,
				doca_dpa_dev_mem_t local_mem,
				size_t length,
				uint64_t raddr,
				uint32_t rkey,
				doca_dpa_dev_sync_event_remote_t event,
				uint64_t comp_count);

/**
 * \brief Initiate remote transfer of data and notification on endpoint
 *
 * This function writes data to a remote peer that is connected with endpoint `ep`.
 * The local buffer, specified by `local_addr` is not ready to be reused after the call returns.
 * Buffers can only be reused after a call to `doca_dpa_dev_ep_synchronize`.
 * This is a non-blocking communication routine. The function may yield the DPA thread internally.
 * The data written by this function is visible on the remote side after the completion `event` has been written.
 * The event count is set as the comp count
 *
 * @param ep [in] - Endpoint to use
 * @param local_addr [in] - Local buffer to send
 * @param length [in] - Length of local buffer
 * @param local_mem [in] - Memory handle for local memory
 * @param raddr [in] - Remote virtual address
 * @param rkey [in] - Access key for remote buffer
 * @param event [in] - Remote event to update the count of
 * @param comp_count [in] - Value to set as the event count
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_put_signal_set_nb(doca_dpa_dev_ep_t ep,
				uint64_t local_addr,
				doca_dpa_dev_mem_t local_mem,
				size_t length,
				uint64_t raddr,
				uint32_t rkey,
				doca_dpa_dev_sync_event_remote_t event,
				uint64_t comp_count);

/**
 * \brief Wait for all operations issued on the endpoint to complete
 *
 * This function returns when communication operations issued on the endpoint have been completed locally. After this
 * call returns, the buffers they referenced by the communication operations can be reused.
 * This call is blocking
 *
 * @param ep [in] - Endpoint to wait on its operations completion
 *
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_ep_synchronize(doca_dpa_dev_ep_t ep);

/**
 * \brief Print to Host
 *
 * This function prints from device to host's standard output stream.
 * Multiple threads may call this routine simultaneously. Printing is a convenience service, and due to limited
 * buffering on the host, not all print statements may appear on the host
 *
 * @param format [in] - Format string that contains the text to be written to stdout (same as from regular printf)
 */
__DOCA_EXPERIMENTAL
void doca_dpa_dev_printf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif /* DOCA_DPA_DEV_H_ */

/** @} */
