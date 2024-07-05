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
 * @file doca_buf.h
 * @page doca buf
 * @defgroup DOCACore Core
 * @defgroup BUF DOCA Buffer
 * @ingroup DOCACore
 * The DOCA Buffer is used for reference data. It holds the information on a memory region that belongs to
 * a DOCA memory map, and its descriptor is allocated from DOCA Buffer Inventory.
 *
 * @{
 */

#ifndef DOCA_BUF_H_
#define DOCA_BUF_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <doca_compat.h>
#include <doca_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * DOCA Buffer element
 ******************************************************************************/

/**
 * @brief Opaque structure representing a data buffer, that can be read by registered DOCA devices.
 */
struct doca_buf;

enum doca_buf_extension {
	DOCA_BUF_EXTENSION_NONE = 0,
};

/**
 * @brief Increase the object reference count by 1.
 *
 * @param [in] buf
 * DOCA Buf element.
 * @param [out] refcount
 * The number of references to the object before this operation took place.
 *
 * @return
 * - DOCA_ERROR_NOT_SUPPORTED
 *
 * @note This function is not supported yet.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_refcount_add(struct doca_buf *buf, uint16_t *refcount);

/**
 * @brief Decrease the object reference count by 1, if 0 reached, return the element back to the inventory.
 *
 * @details When refcont 0 reached, all related resources should be released. For example if the element points into
 * some mmap its state will be adjusted accordingly.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 * @param [out] refcount
 * The number of references to the object before this operation took place. Can be NULL.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_NOT_PERMITTED - buf is the next element in some list.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_refcount_rm(struct doca_buf *buf, uint16_t *refcount);

/**
 * @brief Get the reference count of the object.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 * @param [out] refcount
 * The number of references to the object. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_get_refcount(struct doca_buf *buf, uint16_t *refcount);

/*******************************************************************************
 * DOCA Buffer: Data placement
 *******************************************************************************
 *
 * head   -->            +-------------------+
 *                       |                   |
 *                       |                   |
 *                       |                   |
 *                       |                   |
 *                       |                   |
 * data   -->            +-------------------+
 *                       | data              |
 *                       |                   |
 *                       |                   |
 *                       |                   |
 *                       |                   |
 *                       |                   |
 * data + data_len -->   +-------------------+
 *                       |                   |
 *                       |                   |
 *                       |                   |
 *                       |                   |
 * head + len      -->   +-------------------+
 */

/**
 * @brief Get the buffer's length.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 * @param [out] len
 * The length of the buffer. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_get_len(const struct doca_buf *buf, size_t *len);

/**
 * @brief Get the buffer's head.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 * @param [out] head
 * The head of the buffer. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_get_head(const struct doca_buf *buf, void **head);

/**
 * @brief Get buffer's data length.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 * @param [out] data_len
 * The data length of the buffer. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_get_data_len(const struct doca_buf *buf, size_t *data_len);

/**
 * @brief Get the buffer's data.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 * @param [out] data
 * The data of the buffer. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_get_data(const struct doca_buf *buf, void **data);

/**
 * Set data pointer and data length
 *
 *
 *         +-----------+-----+-----------------+
 * Before  |           |data |                 |
 *         +-----------+-----+-----------------+
 *
 *                 __data_len__
 *                /            \
 *         +-----+--------------+--------------+
 * After   |     |data          |              |
 *         +-----+--------------+--------------+
 *              /
 *            data
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 * @param [in] data
 * Data address. MUST NOT BE NULL.
 * @param [in] data_len
 * Data length.
 * @note The range [data, data + data_len] must be in [head, head + len]. Otherwise undefined behaviour.
 *
 * @return
 * DOCA_SUCCESS - always
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_set_data(struct doca_buf *buf, void *data, size_t data_len);

/**
 * Reset the data length to 0 (data will still point to the same location)
 *
 *
 *                 __data_len__
 *                /            \
 *         +-----+--------------+--------------+
 * Before  |     |data          |              |
 *         +-----+--------------+--------------+
 *              /
 *            data
 *
 *                 data_len = 0
 *                /
 *         +-----+-----------------------------+
 * After   |     |                             |
 *         +-----+-----------------------------+
 *              /
 *            data
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_reset_data_len(struct doca_buf *buf);

/**
 * @brief Get next DOCA Buf in linked list.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 *
 * @param [out] next_buf
 * The next DOCA Buf in the linked list, *next_buf will be NULL if the no other element in the list. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_list_next(struct doca_buf *buf, struct doca_buf **next_buf);

/**
 * @brief Get last DOCA Buf in linked list.
 *
 * @param [in] buf
 * DOCA Buf element.
 *
 * @param [out] last_buf
 * The last DOCA Buf in the linked list, which may be buf.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_list_last(struct doca_buf *buf, struct doca_buf **last_buf);

/**
 * @brief Check if provided DOCA Buf is the last element in a linked list.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 *
 * @param [out] is_last
 * True if buf is the last element, false if it is not. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_list_is_last(const struct doca_buf *buf, bool *is_last);

/**
 * @brief Check if provided DOCA Buf is the first element in a linked list.
 *
 * @param [in] buf
 * DOCA Buf element.
 *
 * @param [out] is_first
 * True if buf is the first element, false if it is not.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_list_is_first(const struct doca_buf *buf, bool *is_first);

/**
 * @brief Check if provided DOCA Buf is a linked list.
 *
 * @param [in] buf
 * DOCA Buf element. MUST NOT BE NULL.
 *
 * @param [out] is_in_list
 * 1 if buf is part of a linked list, 0 if it is not. MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_is_in_list(const struct doca_buf *buf, uint8_t *is_in_list);

/**
 * @brief Get the number of the elements in list.
 *
 * @param [in] buf
 * DOCA Buf element. Buf must be a head of a list.
 *
 * @param [out] num_elements
 * Number of elements in list.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid input had been received.
 * - DOCA_ERROR_NOT_PERMITTED - if the buffer is not a head of a list.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_list_num_elements(const struct doca_buf *buf, uint32_t *num_elements);

/**
 * @brief Append list2 to list1.
 *
 * Before:
 *           +----+  +----+  +----+
 * list1 ->  |1   |->|2   |->|3   |
 *           +----+  +----+  +----+
 *
 *           +----+  +----+
 * list2 ->  |4   |->|5   |
 *           +----+  +----+
 *
 * After:
 *
 *           +----+  +----+  +----+  +----+  +----+
 * list1 ->  |1   |->|2   |->|3   |->|4   |->|5   |
 *           +----+  +----+  +----+  +----+  +----+
 *                                  /
 *                               list2
 * @param [in] list1
 * DOCA Buf representing list1. MUST NOT BE NULL AND MUST BE HEAD OF LIST.
 *
 * @param [in] list2
 * DOCA Buf representing list2. MUST NOT BE NULL AND MUST BE HEAD OF LIST.
 *
 * @return
 * DOCA_SUCCESS - always.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_list_chain(struct doca_buf *list1, struct doca_buf *list2);

/**
 * @brief Separate list2 from list1.
 *
 * Before:
 *           +----+  +----+  +----+  +----+  +----+
 * list1 ->  |1   |->|2   |->|3   |->|4   |->|5   |
 *           +----+  +----+  +----+  +----+  +----+
 *                                  /
 *                               list2
 *
 * After:
 *           +----+  +----+  +----+
 * list1 ->  |1   |->|2   |->|3   |
 *           +----+  +----+  +----+
 *
 *           +----+  +----+
 * list2 ->  |4   |->|5   |
 *           +----+  +----+
 *
 * @param [in] list1
 * DOCA Buf representing list1. MUST NOT BE NULL.
 * @param [in] list2
 * DOCA Buf representing list2, list2 should be contained in list1.
 * list2 must be different from list1. MUST NOT BE NULL
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if list2 is not part of list1.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_buf_list_unchain(struct doca_buf *list1, struct doca_buf *list2);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* DOCA_BUF_H_ */
