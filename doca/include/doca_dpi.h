/*
 * Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
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
 * @file doca_dpi.h
 * @page doca dpi
 * @defgroup DPI Deep packet inspection
 * DOCA Deep packet inspection library. For more details please refer to the
 * user guide on DOCA devzone.
 *
 * @{
 */

#ifndef DOCA_DPI_H_
#define DOCA_DPI_H_

#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <linux/types.h>

#include <doca_buf.h>
#include <doca_compat.h>
#include <doca_ctx.h>
#include <doca_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************************
 * DOCA_DPI JOB TYPES
 *********************************************************************************************************************/

/**
 * @brief DOCA_DPI job types.
 */
enum doca_dpi_job_types {
	DOCA_DPI_JOB = DOCA_ACTION_DPI_FIRST + 1,
};

/*********************************************************************************************************************
 * DOCA_DPI JOB defines
 *********************************************************************************************************************/

/**
 * @brief DOCA_DPI job definition.
 */
struct doca_dpi_job {
	struct doca_job base;
	/**< Opaque struct. */
	const struct doca_buf *pkt;
	/**< The packet to be inspected. */
	bool initiator;
	/**< Indicates to which direction the packet belongs.
	  * 1 - if the packet arrives from client to server.
	  * 0 - if the packet arrives from server to client.
	  * Typically, the first packet will arrive from the initiator (client).
	  */
	uint32_t payload_offset;
	/**< Indicates where the packet's payload begins. */
	struct doca_dpi_flow_ctx *flow_ctx;
	/**< The flow context handler, created by calling doca_dpi_flow_create(). */
	struct doca_dpi_result *result;
	/**< The inspection result buffer, caller must pre-allocate it and ensure it is not freed until result is returned. */
};

/*********************************************************************************************************************
 * DOCA_DPI result defines
 *********************************************************************************************************************/

/**
 * @brief Status of enqueued flow
 */
enum doca_dpi_flow_status_t {
	DOCA_DPI_STATUS_LAST_PACKET = 1 << 1,
	/**< Indicates there are no more packets in queue from this flow. */
	DOCA_DPI_STATUS_DESTROYED = 1 << 2,
	/**< Indicates flow was destroyed while being processed */
	DOCA_DPI_STATUS_NEW_MATCH = 1 << 3,
	/**< Indicates flow was matched on current dequeue */
};

/**
 * @brief Signature action. Some signatures may come with an action.
 */
enum doca_dpi_sig_action_t {
	DOCA_DPI_SIG_ACTION_NA,
	/**< Action not available for signature */
	DOCA_DPI_SIG_ACTION_ALERT,
	/**< Alert */
	DOCA_DPI_SIG_ACTION_PASS,
	/**< Signature indicates that the flow is allowed */
	DOCA_DPI_SIG_ACTION_DROP,
	/**< Signature indicates that the flow should be dropped */
	DOCA_DPI_SIG_ACTION_REJECT,
	/**< Send RST/ICMP unreach error to the sender of the matching packet */
	DOCA_DPI_SIG_ACTION_REJECTSRC,
	/**< Send RST/ICMP unreach error to the sender of the matching packet */
	DOCA_DPI_SIG_ACTION_REJECTDST,
	/**< Send RST/ICMP error packet to receiver of the matching packet */
	DOCA_DPI_SIG_ACTION_REJECTBOTH,
	/**< Send RST/ICMP error packets to both sides of the conversation */
};

/**
 * @brief Signature info
 */
struct doca_dpi_sig_info {
	uint32_t sig_id;
	/**< Signature ID as provided in the signature */
	int action;
	/**< The action as provided in the signature */
};

/**
 * @brief DOCA_DPI result definition.
 */
struct doca_dpi_result {
	const struct doca_buf *pkt;
	/**< The packet inspected for this result. */
	bool matched;
	/**< Indicates flow was matched */
	struct doca_dpi_sig_info info;
	/**< Signature information */
	int status_flags;
	/**< doca_dpi_flow_status flags */
};

/*********************************************************************************************************************
 * DOCA_DPI Context
 *********************************************************************************************************************/

/**
 * @brief Opaque DPI context
 */
struct doca_dpi;

/**
 * @brief Opaque flow context
 */
struct doca_dpi_flow_ctx;

/**
 * @brief L2-L4 flow information, used to uniquely define a flow.
 */
struct doca_dpi_parsing_info {
	__be16 ethertype;
	/**< Ethertype of the packet in network byte order*/
	uint8_t l4_protocol;
	/**< Layer 4 protocol */
	in_port_t l4_dport;
	/**< Layer 4 destination port in network byte order*/
	in_port_t l4_sport;
	/**< Layer 4 source port in network byte order*/
	union {
		struct in_addr ipv4;
		/**< Ipv4 destination address in network byte order */
		struct in6_addr ipv6;
		/**< Ipv6 destination address in network byte order */
	} dst_ip;
	/**< IP destination address */
	union {
		struct in_addr ipv4;
		/**< Ipv4 source address in network byte order */
		struct in6_addr ipv6;
		/**< Ipv6 source address in network byte order */
	} src_ip;
	/**< IP source address */
};

/**
 * @brief Extra signature data
 */
struct doca_dpi_sig_data {
	uint32_t sig_id;
	/**< Signature ID as provided in the signature */
	char name[1024];
	/**< Signature name */
};

/**
 * @brief DPI statistics
 */
struct doca_dpi_stat_info {
	uint32_t nb_scanned_pkts;
	/**< Total number of scanned packets */
	uint32_t nb_matches;
	/**< Total number of signature matches */
	uint32_t nb_http_parser_based;
	/**< Total number of http signature matches */
	uint32_t nb_ssl_parser_based;
	/**< Total number of ssl signature matches */
	uint32_t nb_tcp_based;
	/**< Total number of tcp signature matches */
	uint32_t nb_udp_based;
	/**< Total number of udp signature matches */
	uint32_t nb_other_l4;
	/**< Total number of other l4 signature matches */
	uint32_t nb_other_l7;
	/**< Total number of other l7 signature matches */
};

/**
 * @brief Create a DOCA_DPI instance.
 *
 * This function must be invoked first before any function in the API.
 * It should be invoked once per process.
 *
 * This call will probe the first regex device it finds (0), when we still use the dpdk-regex as backend.
 *
 * @param [out] dpi
 * Instance pointer to be created, MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - not enough memory for allocation.
 * - DOCA_ERROR_NOT_SUPPORTED - the required engine is not supported.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_create(struct doca_dpi **dpi);

/**
 * @brief Destroy DOCA_DPI instance.
 *
 * @param [in] dpi
 * Instance to be destroyed, MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_IN_USE - the attached workq is not detached yet. Please call doca_ctx_stop().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_destroy(struct doca_dpi *dpi);

/**
 * Determine if a given device is suitable for use with doca_dpi.
 *
 * @param [in] devinfo
 * Device to check.
 *
 * @return
 * DOCA_SUCCESS - Device can be used with doca_dpi.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - Device cannot be used with doca_dpi.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_is_supported(const struct doca_devinfo *devinfo);

/**
 * @brief Check if given device is capable of excuting a specific DOCA_DPI job.
 *
 * @param [in] devinfo
 * The DOCA device information
 *
 * @param [in] job_type
 * DOCA_DPI job_type to check for support.
 *
 * @return
 * DOCA_SUCCESS - in case device supports job_type.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support this DOCA_DPI job.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_job_get_supported(const struct doca_devinfo *devinfo, enum doca_dpi_job_types job_type);

/**
 * @brief Set the cdo file.
 *
 * The cdo file contains signature information.
 * The cdo file must be loaded before doca_dpi instance is started.
 * So the typical usage sequence is: create doca_dpi instance --> set signatures --> start doca_dpi instance.
 *
 * Database update:
 * When a new signatures database is available, the doca_dpi instance must be stopped first, then,
 * the user may call this function again.
 * So the typical usage sequence is: stop doca_dpi instance --> set new signatures.
 * The newly loaded CDO must contain the signatures of the previously loaded CDO or result will be undefined.
 *
 * @param [in] dpi
 * DOCA_DPI Instance, MUST NOT BE NULL.
 *
 * @param [in] cdo_file
 * CDO file created by the DPI compiler.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_IN_USE - the required engine resource is not released yet. Please call doca_ctx_stop().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_set_signatures(struct doca_dpi *dpi, const char *cdo_file);

/**
 * @brief Get all signatures of a DPI instance.
 *
 * It is the responsibility of the user to free the array.
 * Because this function copies all the sig info, it is highly recommended
 * to call this function only once after loading the database, and not during
 * packet processing.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [out] sig_data
 * Output of the sig data.
 *
 * @param [out] total_sigs
 * Output of the number of signatures.
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failure.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_get_signatures(const struct doca_dpi *dpi,
				     struct doca_dpi_sig_data **sig_data, uint32_t *total_sigs);

/**
 * @brief Get a specific signature data of a DPI instance.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [in] sig_id
 * The signature ID.
 *
 * @param [out] sig_data
 * Output of the sig data.
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_get_signature(const struct doca_dpi *dpi, uint32_t sig_id,
				    struct doca_dpi_sig_data *sig_data);

/**
 * @brief Set the maximum number of packets packets that can be stored for a DOCA_DPI workq.
 *
 * DOCA_DPI may require that packets are stored to ensure signatures are detected that occur
 * accross multiple packets of the same flow.
 * This value sets the maximum number of packets that can be simultaenously stored for all flows
 * beings processed on a workq.
 * After this function is called, all the "memory" of the previous packets enqueued are deleted.
 *
 * @param [in] dpi
 * DOCA_DPI Instance, MUST NOT BE NULL.
 *
 * @param [in] per_workq_packet_pool_size
 * The value to be set.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_IN_USE - the required engine resource is not released yet. Please call doca_ctx_stop().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_set_per_workq_packet_pool_size(struct doca_dpi *dpi, uint32_t per_workq_packet_pool_size);

/**
 * @brief Return the value in the corresponding set command.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [out] per_workq_packet_pool_size
 * Output of the maximum inflight job number.
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_get_per_workq_packet_pool_size(const struct doca_dpi *dpi, uint32_t *per_workq_packet_pool_size);

/**
 * @brief Set the maximum number of active flows that can be supported by a DOCA_DPI workq.
 *
 * @param [in] dpi
 * DOCA_DPI Instance, MUST NOT BE NULL.
 *
 * @param [in] per_workq_max_flows
 * The value to be set.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_IN_USE - the required engine resource is not released yet. Please call doca_ctx_stop().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_set_per_workq_max_flows(struct doca_dpi *dpi, uint32_t per_workq_max_flows);

/**
 * @brief Return the configures maximum active flows per workq.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [out] per_workq_max_flows
 * The maximum parallel flows supported per workq.
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_get_per_workq_max_flows(const struct doca_dpi *dpi, uint32_t *per_workq_max_flows);

/**
 * @brief Set the maximum length that DPI guarantees to provide a match on
 *
 * It includes across consecutive packets.
 * Must be <= 5000
 * For example:
 *        Signature = A.*B
 *        max_sig_match_len = 5
 * DPI guarantee that ACCCB will be found (len <= 5)
 * DPI does not guarantee that ACCCCCCCB will be found (len > 5)
 *
 * @param [in] dpi
 * DOCA_DPI Instance, MUST NOT BE NULL.
 *
 * @param [in] max_sig_match_len
 * The value to be set.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_IN_USE - the required engine resource is not released yet. Please call doca_ctx_stop().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_set_max_sig_match_len(struct doca_dpi *dpi, uint16_t max_sig_match_len);

/**
 * @brief Get the maximum signature match length of a DPI instance.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [out] max_sig_match_len
 * Output of the maximum signature match length
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_get_max_sig_match_len(const struct doca_dpi *dpi, uint16_t *max_sig_match_len);


/**
 * @brief Enable or disable 'in order' mode
 *
 * Due to the parallel nature of pattern matching accelerators that may be used by DPI, there is no guarantee that
 * results from jobs sent will be returned in the same order.
 * Enabling 'in order' mode guarantees that results returned per workq will be in the order they were sent.
 *
 * @note This mode is disabled by default. Choosing to enable it may have a negative impact on performance.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [out] enabled
 * Set to true to turn 'in order' mode on, false to disable it.
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 * - DOCA_ERROR_IN_USE - the required engine resource is not released yet. Please call doca_ctx_stop().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_set_in_order_mode(const struct doca_dpi *dpi, bool enabled);

/**
 * @brief Convert DOCA_DPI instance into context for use with workQ
 *
 * @param [in] dpi
 * DOCA_DPI instance. This must remain valid until after the context is no longer required.
 *
 * @return
 * Non NULL - doca_ctx object on success.
 * Error:
 * - NULL.
 */
__DOCA_EXPERIMENTAL
struct doca_ctx *doca_dpi_as_ctx(struct doca_dpi *dpi);

/**
 * @brief Creates a new flow for a workq
 *
 * Must be called before enqueuing any new packet using job_submit().
 * A flow must not be created on 2 different queues.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [in] workq
 * The DPI workq on which to create the flow
 *
 * @param [in] parsing_info
 * L3/L4 information.
 *
 * @param [out] flow_ctx
 * The created flow.
 *
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failure.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_flow_create(struct doca_dpi *dpi, struct doca_workq *workq,
				  const struct doca_dpi_parsing_info *parsing_info,
				  struct doca_dpi_flow_ctx **flow_ctx);

/**
 * @brief Destroys a flow on a workq
 *
 * Should be called when a flow is terminated or times out
 *
 * @param [in] flow_ctx
 * The flow context to destroy.
 *
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_flow_destroy(struct doca_dpi_flow_ctx *flow_ctx);

/**
 * @brief Get the latest match of a flow.
 *
 * @param [in] flow_ctx
 * The flow context of the flow to be queried.
 *
 * @param [out] result
 * Output, latest match on this flow.
 * Only "matched" and "info" fields in the result parameter are valid.
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_get_flow_match(const struct doca_dpi_flow_ctx *flow_ctx,
				     struct doca_dpi_result *result);

/**
 * @brief Get DPI statistics.
 *
 * @param [in] dpi
 * The DOCA_DPI instance.
 *
 * @param [in] clear
 * Clear the statistics after fetching them.
 *
 * @param [out] stats
 * Output struct containing the statistics.
 *
 * @return
 * DOCA_SUCCESS - upon success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_dpi_get_stats(const struct doca_dpi *dpi, bool clear,
			        struct doca_dpi_stat_info *stats);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif /* DOCA_DPI_H_ */
