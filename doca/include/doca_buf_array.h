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
 * @file doca_buf_array.h
 * @page doca buf array
 * @defgroup BUF_ARRAY DOCA Buffer Array
 * @ingroup DOCACore
 * The DOCA buffer array represents an array of fixed size doca_bufs (for multiple doca_dev).
 * Can act as a free list or direct access mode.
 *
 * @{
 */

#ifndef DOCA_BUF_ARRAY_H_
#define DOCA_BUF_ARRAY_H_

#include <stddef.h>
#include <stdint.h>

#include <doca_compat.h>
#include <doca_error.h>
#include <doca_buf.h>
#include <doca_mmap.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * DOCA Buf Array
 ******************************************************************************/

struct doca_gpu;
struct doca_dpa;

/**
 * @brief Opaque structure representing a doca buffer array.
 */
struct doca_buf_arr;
struct doca_gpu_buf_arr;

/**
 * @brief Allocates a doca_buf_arr.
 *
 * @param [in] mmap
 * The mmap managing the memory chunk. Must be populated with memory chunk.
 * @param [out] buf_arr
 * The newly created doca_buf_arr.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate a doca_buf_arr.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_arr_create(struct doca_mmap *mmap, struct doca_buf_arr **buf_arr);

/**
 * @brief Configures the buf array to be created on the gpu device
 *
 * @param [in] buf_arr
 * The doca_buf_arr
 * @param [in] gpu_handler
 * The gpu device handler.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_BAD_STATE - if doca_buf_arr is already started
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_arr_set_target_gpu(struct doca_buf_arr *buf_arr, struct doca_gpu *gpu_handler);

/**
 * @brief Sets the buf array params
 *
 * @param [in] buf_arr
 * The doca_buf_arr
 * @param [in] size
 * Size in bytes of a single element (must be > 0).
 * @param [in] num_elem
 * Number of elements in the doca_buf_arr (must be > 0).
 * @param [in] start_offset
 * Offset from mmap start to set doca_buf_arr.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_BAD_STATE - if doca_buf_arr is already started
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_arr_set_params(struct doca_buf_arr *buf_arr, size_t size, uint32_t num_elem, uint32_t start_offset);

/**
 * @brief This method enables the allocation of doca_bufs.
 *
 * @note Before calling this function, the mmap with which the buf array was created must be started.
 *
 * @param [in] buf_arr
 * The doca_buf_arr to start
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_BAD_STATE -
 *	- if doca_buf_arr is already started once.
 *	- if target device is missing
 * - DOCA_ERROR_NOT_PERMITTED - if mmap is not started.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate enough space for configuration structure
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_arr_start(struct doca_buf_arr *buf_arr);

/**
 * @brief Stops a started doca buf array
 *
 * @param [in] buf_arr
 * The doca_buf_arr to stop
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_arr_stop(struct doca_buf_arr *buf_arr);

/**
 * @brief Destroys a doca buf array instance
 *
 * @param [in] buf_arr
 * The doca_buf_arr to destroy
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_arr_destroy(struct doca_buf_arr *buf_arr);

/**
 * @brief Retrieves the handle in the gpu memory space of a doca_buf_arr
 *
 * @param [in] buf_arr
 * The doca_buf_arr
 * @param [out] gpu_buf_arr
 * A pointer to the handle in the gpu memory space
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_BAD_STATE - if doca_buf_arr is not started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_arr_get_gpu_handle(const struct doca_buf_arr *buf_arr, struct doca_gpu_buf_arr **gpu_buf_arr);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* DOCA_BUF_ARRAY_H_ */
