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
 * @file doca_eth_rxq.h
 * @page DOCA ETH RXQ
 * @defgroup DOCAETHRXQ DOCA ETH RXQ
 * DOCA ETH RXQ library.
 *
 * @{
 */
#ifndef DOCA_ETH_RXQ_H_
#define DOCA_ETH_RXQ_H_

#include <doca_buf.h>
#include <doca_ctx.h>
#include <doca_mmap.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************************
 * DOCA ETH RXQ Context
 *********************************************************************************************************************/

/**
 * Opaque structure representing a DOCA ETH RXQ instance.
 */
struct doca_eth_rxq;
struct doca_gpu_eth_rxq;

/**
 * RX queue type.
 */
enum doca_eth_rxq_type {
	DOCA_ETH_RXQ_TYPE_CYCLIC = 0,
};

/**
 * @brief Create a DOCA ETH RXQ instance.
 *
 * @param [out] eth_rxq
 * Pointer to pointer to be set to point to the created doca_eth_rxq instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - eth_rxq argument is a NULL pointer.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate resources.
 * - DOCA_ERROR_INITIALIZATION - failed to initialize eth_rxq.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_create(struct doca_eth_rxq **eth_rxq);

/**
 * @brief Destroy a DOCA ETH RXQ instance.
 *
 * @param [in] eth_rxq
 * Pointer to instance to be destroyed.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - eth_rxq argument is a NULL pointer.
 * - DOCA_ERROR_BAD_STATE - the associated ctx was not stopped before calling doca_eth_rxq_destroy().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_destroy(struct doca_eth_rxq *eth_rxq);

/**
 * @brief Convert doca_eth_rxq instance into a generalised context for use with doca core objects.
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 *
 * @return
 * Non NULL upon success, NULL otherwise.
 */
__DOCA_EXPERIMENTAL
struct doca_ctx *doca_eth_rxq_as_doca_ctx(struct doca_eth_rxq *eth_rxq);

/**
 * @brief Set queue size property for doca_eth_rxq.
 * can only be called before calling doca_ctx_start().
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 * @param [in] num_packets
 * Queue max number of packets to use in context.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_set_num_packets(struct doca_eth_rxq *eth_rxq, uint32_t num_packets);

/**
 * @brief Set max packet size property for doca_eth_rxq.
 * can only be called before calling doca_ctx_start().
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 * @param [in] max_pkt_sz
 * Max packet size to use in context.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_set_max_packet_size(struct doca_eth_rxq *eth_rxq, uint16_t max_pkt_sz);

/**
 * @brief Set RX queue type property for doca_eth_rxq.
 * can only be called before calling doca_ctx_start().
 *
 * @note in doca_eth_rxq_create() the default type is DOCA_ETH_RXQ_TYPE_CYCLIC
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 * @param [in] type
 * RX queue type - see enum doca_eth_rxq_type
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_set_type(struct doca_eth_rxq *eth_rxq, enum doca_eth_rxq_type type);

/**
 * @brief Get the DPDK queue ID of the doca_eth receive queue.
 * can only be called after calling doca_ctx_start().
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 * @param [out] flow_queue_id
 * The queue ID to be used in rte_flow or doca_flow
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context was not started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_get_flow_queue_id(struct doca_eth_rxq *eth_rxq, uint16_t *flow_queue_id);

/**
 * @brief Get a gpu handle of a doca_eth_rxq.
 *
 * @details This method should be used after ctx is started.
 * The expected flow is as follows:
 * 1. bind the ctx to a gpu device using doca_ctx_set_data_path_on_gpu()
 * 2. start the ctx using doca_ctx_start()
 * 3. call doca_eth_rxq_get_gpu_handle() to get the gpu_handle
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 * @param [out] eth_rxq_ext
 * A doca gpu eth_rxq handle.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is not started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_get_gpu_handle(const struct doca_eth_rxq *eth_rxq,
	struct doca_gpu_eth_rxq **eth_rxq_ext);

/**
 * @brief Get the maximum packet size supported by the device
 *
 * @param [in] devinfo
 * Pointer to doca_devinfo instance.
 * @param [out] max_packet_size
 * The max packet size.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_get_max_packet_size_supported(const struct doca_devinfo *devinfo, uint16_t *max_packet_size);

/**
 * @brief Check if RX queue type is supported
 *
 * @param [in] devinfo
 * Pointer to doca_devinfo instance.
 * @param [in] type
 * RX queue type - see enum doca_eth_rxq_type
 * @param [out] type_supported
 * Flag to indicate if type is supported.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_get_type_supported(const struct doca_devinfo *devinfo, enum doca_eth_rxq_type type,
	uint8_t *type_supported);

/**
 * @brief Get the required size for the Eth packet buffer of a doca_eth_rxq.
 *
 * @details This function should be used for calculating the minimum size of
 * the doca_mmap given to doca_eth_rxq_set_pkt_buffer().
 *
 * @note Must be called after doca_eth_rxq_set_num_packets() and doca_eth_rxq_set_max_packet_size().
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 * @param [out] size
 * The required size for the eth packet buffer.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - in case of uninitialized queue size (packet_size, num_packets).
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_get_pkt_buffer_size(const struct doca_eth_rxq *eth_rxq, uint32_t *size);

/**
 * @brief Set Eth packet buffer for a doca_eth_rxq.
 * can only be called before calling doca_ctx_start().
 *
 * @param [in] eth_rxq
 * Pointer to doca_eth_rxq instance.
 * @param [in] mmap
 * The mmap consist of the memrange for the Eth packet buffer.
 * @param [in] offset
 * The offset from mmap start to set the packet buffer.
 * @param [in] size
 * The size of the Eth packet buffer.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_eth_rxq_set_pkt_buffer(struct doca_eth_rxq *eth_rxq, struct doca_mmap *mmap,
	uint32_t offset, uint32_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_ETH_RXQ_H_ */

/** @} */