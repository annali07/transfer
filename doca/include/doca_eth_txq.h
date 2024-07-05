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
 * @file doca_eth_txq.h
 * @page DOCA ETH TXQ
 * @defgroup DOCAETHTXQ DOCA ETH TXQ
 * DOCA ETH TXQ library.
 *
 * @{
 */
#ifndef DOCA_ETH_TXQ_H_
#define DOCA_ETH_TXQ_H_

#include <doca_buf.h>
#include <doca_ctx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************************
 * DOCA ETH TXQ Context
 *********************************************************************************************************************/

/**
 * Opaque structure representing a DOCA ETH TXQ instance.
 */
struct doca_eth_txq;
struct doca_gpu_eth_txq;

/**
 * TX queue type.
 */
enum doca_eth_txq_type {
	DOCA_ETH_TXQ_TYPE_CYCLIC = 0,
};

/**
 * @brief Create a DOCA ETH TXQ instance.
 *
 * @param [out] eth_txq
 * Pointer to pointer to be set to point to the created doca_eth_txq instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - eth_txq argument is a NULL pointer.
 * - DOCA_ERROR_NO_MEMORY - failed to alloc doca_eth_txq.
 * - DOCA_ERROR_INITIALIZATION - failed to initialize eth_txq.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_create(struct doca_eth_txq **eth_txq);

/**
 * @brief Destroy a DOCA ETH TXQ instance.
 *
 * @param [in] eth_txq
 * Pointer to instance to be destroyed.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - eth_txq argument is a NULL pointer.
 * - DOCA_ERROR_BAD_STATE - the associated ctx was not stopped before calling doca_eth_txq_destroy().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_destroy(struct doca_eth_txq *eth_txq);

/**
 * @brief Convert doca_eth_txq instance into a generalised context for use with doca core objects.
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 *
 * @return
 * Non NULL upon success, NULL otherwise.
 */
__DOCA_EXPERIMENTAL
struct doca_ctx *doca_eth_txq_as_doca_ctx(struct doca_eth_txq *eth_txq);

/**
 * @brief Set queue size property for doca_eth_txq.
 * can only be called before calling doca_ctx_start().
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 * @param [in] size
 * Queue size to use in context.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_set_queue_size(struct doca_eth_txq *eth_txq, uint32_t size);

/**
 * @brief Set TX queue type property for doca_eth_txq.
 * can only be called before calling doca_ctx_start().
 *
 * @note in doca_eth_txq_create() the default type is DOCA_ETH_TXQ_TYPE_CYCLIC
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 * @param [in] type
 * TX queue type - see enum doca_eth_txq_type
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_set_type(struct doca_eth_txq *eth_txq, enum doca_eth_txq_type type);

/**
 * @brief Set offload for the calculation of IPv4 checksum (L3) on transmitted packets.
 * can only be called before calling doca_ctx_start().
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_set_l3_chksum_offload(struct doca_eth_txq *eth_txq);

/**
 * @brief Set offload for the calculation of TCP/UDP checksum (L4) on transmitted packets.
 * can only be called before calling doca_ctx_start().
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_set_l4_chksum_offload(struct doca_eth_txq *eth_txq);

/**
 * @brief Set offload to enable wait on time feature on the queue.
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 * - DOCA_ERROR_NOT_PERMITTED - Wait on time HW support but network device clock is not in REAL TIME mode.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_set_wait_on_time_offload(struct doca_eth_txq *eth_txq);

/**
 * @brief Get a gpu handle of a doca_eth_txq.
 *
 * @details This method should be used after ctx is started.
 * The expected flow is as follows:
 * 1. bind the ctx to a gpu device using doca_ctx_set_data_path_on_gpu()
 * 2. start the ctx using doca_ctx_start()
 * 3. call doca_eth_txq_get_gpu_handle() to get the gpu_handle
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 * @param [out] eth_txq_ext
 * A doca gpu eth_txq handle.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_get_gpu_handle(const struct doca_eth_txq *eth_txq,
	struct doca_gpu_eth_txq **eth_txq_ext);

/**
 * @brief Get the maximum queue size supported by the device
 *
 * @param [in] devinfo
 * Pointer to doca_devinfo instance.
 * @param [out] max_queue_size
 * The max queue size.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_get_max_queue_size_supported(const struct doca_devinfo *devinfo, uint32_t *max_queue_size);

/**
 * @brief Check if TX queue type is supported
 *
 * @param [in] devinfo
 * Pointer to doca_devinfo instance.
 * @param [in] type
 * TX queue type - see enum doca_eth_txq_type
 * @param [out] type_supported
 * Flag to indicate if type is supported.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_get_type_supported(const struct doca_devinfo *devinfo, enum doca_eth_txq_type type,
	uint8_t *type_supported);

/**
 * @brief Check if checksum offload is supported by the device
 *
 * @param [in] devinfo
 * Pointer to doca_devinfo instance.
 * @param [out] offload_supported
 * Flag to indicate if checksum offload is supported.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_get_chksum_offload_supported(const struct doca_devinfo *devinfo, uint8_t *offload_supported);

/**
 * @brief Check if wait on time offload is supported by the network device
 *
 * @param [in] dev
 * Pointer to doca_dev instance.
 * @param [out] wait_on_time_mode
 * Offload wait on time mode (native or DPDK)
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_DRIVER - error query underlying network card driver
 * - DOCA_ERROR_NOT_SUPPORTED - real-time clock is not enable on the network card.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_get_wait_on_time_offload_supported(const struct doca_dev *dev, enum doca_eth_wait_on_time *wait_on_time_mode);

/**
 * @brief Calculate timestamp to use when setting the wait on time on the Tx queue
 *
 * @param [in] eth_txq
 * Pointer to doca_eth_txq instance.
 * @param [in] timestamp_ns
 * Timestamp to indicate when send packets
 * @param [out] wait_on_time_value
 * Value to use to enqueue wait on time in send queue
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_DRIVER - error query underlying network card driver
 * - DOCA_ERROR_NOT_PERMITTED - wait on time clock is not enabled on the network card.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_txq_calculate_timestamp(struct doca_eth_txq *eth_txq, uint64_t timestamp_ns, uint64_t *wait_on_time_value);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_ETH_TXQ_H_ */

/** @} */
