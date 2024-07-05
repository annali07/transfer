/*
 * Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
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
 * @file doca_bufpool.h
 * @page doca bufpool
 * @defgroup DOCACore Core
 * @defgroup BUFPOOL DOCA Bufpool
 * @ingroup DOCACore
 * The DOCA Bufpool is an inventory of doca_buf objects, such that each doca_buf is set with a permanent,
 * fixed size memory buffer, right from creation and till destruction, which allows immediate allocation of
 * doca_buf objects.
 *
 * Basic structure example of Bufpool (after creation):
 *
 *                                      +------------------------------------------+
 *                                      |               memory range               |
 *              +-----------+           | +--------+   +--------+       +--------+ |
 *              | doca_mmap |-----------| | buffer |   | buffer |       | buffer | |
 *              +-----------+           | +--------+   +--------+ ..... +--------+ |
 *                                      |  \            \                \         |
 *                                      +------------------------------------------+
 *                                           \            \                \
 *                                            \            \                \
 *                                      +--------------------------------------------+
 *                                      |      |            |                |       |
 *              +--------------+        | +----------+ +----------+     +----------+ |
 *              | doca_bfupool |--------| | doca_buf | | doca_buf |     | doca_buf | |
 *              +--------------+        | +----------+ +----------+ ....+----------+ |
 *                                      +--------------------------------------------+
 *
 * @{
 */

#ifndef DOCA_BUFPOOL_H_
#define DOCA_BUFPOOL_H_

#include <stddef.h>
#include <stdint.h>

#include <doca_error.h>
#include <doca_types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct doca_mmap;
struct doca_buf;
struct doca_bufpool;

/**
 * @brief Allocates a bufpool and sets it with doca_buf objects that are set in advance
 * with constant memory buffers.
 *
 * @param [in] (optional) user_data
 * Bufpool identifier provided by user. If not NULL, pointed user_data will be set.
 * @param [in] num_elements
 * Number of elements in the bufpool (must be > 0).
 * @param [in] extensions
 * Bitmap of extensions enabled for the bufpool described in doca_buf.h.
 * @param [in] element_size
 * Size of a single element (must be > 0).
 * @param [in] element_alignment
 * Element alignment requirement (must be a power of 2, can be 0).
 * @param [in] mmap
 * The mmap managing the memory chunk. Must be populated with memory chunk.
 * @param [out] bufpool
 * The newly created DOCA Bufpool.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate a doca_bufpool.
 * - DOCA_ERROR_BAD_STATE - if no memory range was set to mmap.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_create(const union doca_data *user_data, size_t num_elements, uint32_t extensions,
				 size_t element_size, size_t element_alignment, const struct doca_mmap *mmap, struct doca_bufpool **bufpool);

/**
 * @brief Destroy bufpool structure.
 *
 * Before calling this function, all allocated doca_bufs should be returned back to the bufpool.
 *
 * @param [in] bufpool
 * The DOCA Bufpool to destroy.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_IN_USE - if not all allocated doca_bufs had been returned to the bufpool.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_destroy(struct doca_bufpool *bufpool);

/**
 * @brief Start DOCA bufpool.
 *
 * This method enables the allocation of doca_bufs using doca_bufpool_buf_alloc().
 * Before calling this function, the mmap with which the bufpool was created must be started.
 *
 * @param [in] bufpool
 * The DOCA Bufpool to start.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_BAD_STATE - if the mmap with which the bufpool was created is not started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_start(struct doca_bufpool *bufpool);

/**
 * @brief Stop a started DOCA bufpool.
 *
 * This method disables the allocation of doca_bufs.
 * Calling this method is also possible if there are allocated doca_bufs.
 *
 * @param [in] bufpool
 * The DOCA Bufpool to stop.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_stop(struct doca_bufpool *bufpool);

/**
 * @brief This method acquires a doca_buf from the doca_bufpool, pointing to an allocated empty buffer.
 *
 * @param [in] bufpool
 * The DOCA Bufpool from which to acquire a doca_buf, that was set to point to a memory buffer at doca_bufpool_create().
 * @param [out] buf
 * Pointer to the allocated doca_buf.
 *
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_BAD_STATE - if bufpool is un-started/stopped.
 * - DOCA_ERROR_NO_MEMORY - if the bufpool is empty (all doca_bufs are already allocated).
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_buf_alloc(struct doca_bufpool *bufpool, struct doca_buf **buf);

/**
 * @brief Get the number of elements that was set in the creation of a doca_bufpool.
 *
 * @param [in] bufpool
 * The DOCA Bufpool.
 * @param [out] num_of_elements
 * The number of elements that was set in the creation of bufpool.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_get_num_elements(const struct doca_bufpool *bufpool, uint32_t *num_of_elements);

/**
 * @brief Get the total number of free elements available for allocation in a doca_bufpool.
 *
 * @param [in] bufpool
 * The DOCA Bufpool.
 * @param [out] num_of_free_elements
 * The total number of free elements in bufpool.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_get_num_free_elements(const struct doca_bufpool *bufpool, uint32_t *num_of_free_elements);

/**
 * @brief Get the user_data of a DOCA Bufpool.
 *
 * @details The user_data that was provided to the bufpool upon its creation.
 *
 * @param [in] bufpool
 * The DOCA Bufpool.
 * @param [out] user_data
 * The user_data of bufpool.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_bufpool_get_user_data(const struct doca_bufpool *bufpool, union doca_data *user_data);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* DOCA_BUFPOOL_H_ */
