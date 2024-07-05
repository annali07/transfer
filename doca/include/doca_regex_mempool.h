/*
 * Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
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
 * @file doca_regex_mempool.h
 * @page mempool
 * @defgroup RegExMp RegEx engine memory pool
 *
 * Define functions to allow easy creation and use of memory pools.
 *
 * @{
 */

#ifndef DOCA_REGEX_MEMPOOL_H_
#define DOCA_REGEX_MEMPOOL_H_

#include <doca_compat.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct doca_regex_mempool;

/**
 * Create a memory pool.
 *
 * @note Supports single producer and single consumer only.
 *
 * @param [in] elem_size
 * Size of an element to be stored in the memory pool.
 *
 * @param [in] nb_elems
 * Number of element stored in the memory pool.
 *
 * @return
 * Pointer to the memory pool on success or NULL on failure.
 */
__DOCA_EXPERIMENTAL
struct doca_regex_mempool *doca_regex_mempool_create(size_t elem_size, size_t nb_elems);

/**
 * Destroy a memory pool and all objects it owned.
 *
 * @note all pointers to elements in this pool must be cleared before this call. Failure to do socmay result in
 * undefined behaviour.
 *
 * @param  [in] pool
 * Memory pool to be destroyed. Must not be NULL.
 */
__DOCA_EXPERIMENTAL
void doca_regex_mempool_destroy(struct doca_regex_mempool *pool);

/**
 * Get the capacity of a mempool
 *
 * @param [in] pool
 * The pool to query
 *
 * @return
 * The capacity / size of the pool
 */
__DOCA_EXPERIMENTAL
size_t doca_regex_mempool_get_capacity(struct doca_regex_mempool const *pool);

/**
 * Get the size of element which can be stored in this pool
 *
 * @param [in] pool
 * The pool to query
 *
 * @return
 * The size available for storage of an element within the pool
 */
__DOCA_EXPERIMENTAL
size_t doca_regex_mempool_get_element_size(struct doca_regex_mempool const *pool);
/**
 * Get the number of elements available for use within the pool
 *
 * @param [in] pool
 * The pool to query
 *
 * @return
 * The number of free elements
 */
__DOCA_EXPERIMENTAL
size_t doca_regex_mempool_get_free_count(struct doca_regex_mempool const *pool);

/**
 * Get an object from the memory pool.
 *
 * @note Supports single producer and single consumer only.
 *
 * @param [in] pool
 * Pool from which to get a free object.
 *
 * @return
 * Pointer to an object or NULL if the pool is exhausted.
 */
__DOCA_EXPERIMENTAL
void *doca_regex_mempool_get_obj(struct doca_regex_mempool *pool);

/**
 * Put an object back into the memory pool.
 *
 * @note Supports single producer and single consumer only.
 *
 * @param [in] pool
 * Pool which created obj.
 *
 * @param [in] obj
 * Object created by pool which is being returned to the free state.
 */
__DOCA_EXPERIMENTAL
void doca_regex_mempool_put_obj(struct doca_regex_mempool *pool, void *obj);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif /* DOCA_REGEX_MEMPOOL_H_ */
