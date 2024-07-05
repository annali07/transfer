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
 * @file doca_rdma.h
 * @page DOCA RDMA
 * @defgroup DOCARDMA DOCA RDMA
 * DOCA RDMA library. For more details please refer to the user guide on DOCA devzone.
 *
 * @{
 */
#ifndef DOCA_RDMA_H_
#define DOCA_RDMA_H_

#include <stdint.h>

#include <doca_types.h>
#include <doca_buf.h>
#include <doca_ctx.h>
#include <doca_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Possible states for doca_rdma */
enum doca_rdma_state {
	DOCA_RDMA_STATE_RESET = 0,
	DOCA_RDMA_STATE_INIT,
	DOCA_RDMA_STATE_CONNECTED,
	DOCA_RDMA_STATE_ERROR,
};

/** Available transport types for RDMA */
enum doca_rdma_transport_type {
	DOCA_RDMA_TRANSPORT_RC,	/**< RC transport */
	DOCA_RDMA_TRANSPORT_DC,	/**< DC transport, currently not supported */
};

/** gid struct */
struct doca_rdma_gid {
	uint8_t raw[DOCA_GID_BYTE_LENGTH]; /**< The raw value of the GID */
};

/*********************************************************************************************************************
 * DOCA RDMA JOBS
 *********************************************************************************************************************/

/** Available jobs for RDMA. */
enum doca_rdma_job_types {
	DOCA_RDMA_JOB_RECV = DOCA_ACTION_RDMA_FIRST + 1,
	DOCA_RDMA_JOB_SEND,
	DOCA_RDMA_JOB_SEND_IMM,
	DOCA_RDMA_JOB_READ,
	DOCA_RDMA_JOB_WRITE,
	DOCA_RDMA_JOB_WRITE_IMM,
	DOCA_RDMA_JOB_ATOMIC_CMP_SWP,
	DOCA_RDMA_JOB_ATOMIC_FETCH_ADD,
};

/**
 * The jobs to be dispatched via the RDMA library
 */

/** RDMA receive job. */
struct doca_rdma_job_recv {
	struct doca_job base;		/**< Common job data */
	struct doca_buf *dst_buff;	/**< Destination data buffer,
					  *  chain len must not exceed recv_buf_chain_len property
					  */
};

/** RDMA send job. */
struct doca_rdma_job_send {
	struct doca_job base;				/**< Common job data */
	struct doca_buf const *src_buff;		/**< Source data buffer */
	doca_be32_t immediate_data;			/**< Immediate data */
	struct doca_rdma_addr const *rdma_peer_addr;	/**< Optional: For RDMA context of type UD or DC */
};

/** RDMA read/write job. */
struct doca_rdma_job_read_write {
	struct doca_job base;				/**< Common job data */
	struct doca_buf *dst_buff;			/**< Destination data buffer */
	struct doca_buf const *src_buff;		/**< Source data buffer */
	doca_be32_t immediate_data;			/**< Immediate data for write with imm */
	struct doca_rdma_addr const *rdma_peer_addr;	/**< Optional: For RDMA context of type DC */
};

/** RDMA atomic job. */
struct doca_rdma_job_atomic {
	struct doca_job base;				/**< Common job data */
	struct doca_buf *cmp_or_add_dest_buff;		/**< Destination data buffer */
	struct doca_buf *result_buff;			/**< Result of the atomic operation:
							  *  remote original data before add, or remote original data
							  *  before compare
							  */
	uint64_t swap_or_add_data;			/**< For add, the increment value
							  *  for cmp, the new value to swap
							  */
	uint64_t cmp_data;				/**< Value to compare for compare and swap. */
	struct doca_rdma_addr const *rdma_peer_addr;	/**< Optional: For RDMA context of type DC */
};

/** Job result opcodes */
enum doca_rdma_opcode_t {
	DOCA_RDMA_OPCODE_RECV_SEND = 0,
	DOCA_RDMA_OPCODE_RECV_SEND_WITH_IMM,
	DOCA_RDMA_OPCODE_RECV_WRITE_WITH_IMM,
};

/**
 * Result of a RDMA jobs. Will be held inside the doca_event::result field.
 * RDMA Context results_fields_set() should be used to define which ones are required
 */
struct doca_rdma_result {
	doca_error_t result;			/**< Operation result */
	enum doca_rdma_opcode_t opcode;		/**< Opcode in case of doca_rdma_job_recv completion */
	uint32_t length;			/**< Total length of received data in case of completion
						  *  (on send and write jobs this field will be 0)
						  */
	struct doca_rdma_addr *rdma_peer_addr;	/**< Peer Address for UD and DC */
	doca_be32_t immediate_data;		/**< Immediate data, valid only if opcode indicates */
	/** 'dst_buff' data positioning will get updated on RECV and READ ops */
};


/*********************************************************************************************************************
 * DOCA RDMA Context
 *********************************************************************************************************************/

/**
 * Opaque structure representing a DOCA RDMA instance.
 */
struct doca_rdma;

/**
 * @brief Create a DOCA RDMA instance.
 *
 * @param [out] rdma
 * Pointer to pointer to be set to point to the created doca_rdma instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - rdma argument is a NULL pointer.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate resources.
 * - DOCA_ERROR_INITIALIZATION - failed to initialize rdma.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_create(struct doca_rdma **rdma);

/**
 * @brief Destroy a DOCA RDMA instance.
 *
 * @param [in] rdma
 * Pointer to instance to be destroyed.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - rdma argument is a NULL pointer.
 * - DOCA_ERROR_BAD_STATE - the associated ctx was not stopped before calling doca_rdma_destroy().
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_destroy(struct doca_rdma *rdma);

/**
 * @brief Convert doca_rdma instance into a generalised context for use with doca core objects.
 *
 * @param [in] rdma
 * RDMA instance. This must remain valid until after the context is no longer required.
 *
 * @return
 * Non NULL upon success, NULL otherwise.
 */
__DOCA_EXPERIMENTAL
struct doca_ctx *doca_rdma_as_ctx(struct doca_rdma *rdma);

/**
 * @brief Check if given device supports given doca_rdma job.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [in] job_type
 * doca_rdma job type. See enum doca_rdma_job_types.
 *
 * @return
 * DOCA_SUCCESS - in case the job is supported.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support the given doca_rdma job.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_job_get_supported(const struct doca_devinfo *devinfo, enum doca_rdma_job_types job_type);

/**
 * @brief Export doca_rdma connection details object
 * The doca_rdma_conn_details are used in doca_rdma_connect().
 * Can only be called after calling doca_ctx_start().
 *
 * @param [in] rdma
 * Pointer doca_rdma to export connection details for.
 * @param [out] local_rdma_conn_details
 * Exported doca_rdma_conn_details object.
 * @param [out] local_rdma_conn_details_size
 * Size of exported doca_rdma_conn_details object.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if any of the parameters is null.
 * - DOCA_ERROR_BAD_STATE - if called before calling ctx_start().
 * @note stopping and restarting an RDMA context require calling doca_rdma_export() & doca_rdma_connect() again.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_export(const struct doca_rdma *rdma, const void **local_rdma_conn_details,
			      size_t *local_rdma_conn_details_size);

/**
 * @brief Connect to remote doca_rdma peer.
 * Can only be called after calling doca_ctx_start().
 *
 * @param [in] rdma
 * Pointer to doca_rdma to connect.
 * @param [in] remote_rdma_conn_details
 * Exported doca_rdma_conn_details object from remote peer.
 * @param [in] remote_rdma_conn_details_size
 * Size of remote doca_rdma_conn_details object.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if any of the parameters is null.
 * - DOCA_ERROR_BAD_STATE - if context was not started or rdma instance is already connected.
 * - DOCA_ERROR_CONNECTION_ABORTED - if connection failed or connection details object was corrupted.
 * @note stopping and restarting an RDMA context require calling doca_rdma_export() & doca_rdma_connect() again.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_connect(struct doca_rdma *rdma, const void *remote_rdma_conn_details,
			       size_t remote_rdma_conn_details_size);

/*********************************************************************************************************************
 * DOCA RDMA capabilities
 *********************************************************************************************************************/

/**
 * @brief Get the maximal recv queue size for a specific device.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [out] max_recv_queue_size
 * The maximal recv queue size for the given devinfo.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_max_recv_queue_size(const struct doca_devinfo *devinfo, uint32_t *max_recv_queue_size);

/**
 * Get the maximal send queue size for a specific device.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [out] max_send_queue_size
 * The of the maximal send queue size for the given devinfo.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_max_send_queue_size(const struct doca_devinfo *devinfo, uint32_t *max_send_queue_size);

/**
 * @brief Get the maximal message size for a specific device.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [out] max_message_size
 * The maximal message size for the given devinfo.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_max_message_size(const struct doca_devinfo *devinfo, uint32_t *max_message_size);

/**
 * Get the maximal buffer chain length for a specific device, job type and transport type.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [in] job_type
 * The relevant job type
 * @param [in] transport_type
 * The relevant transport type
 * @param [out] max_buf_chain_len
 * The maximal number of chained local buffers (containing data) that can be submitted in the given DOCA RDMA job for
 * the given transport type and devinfo.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_max_buf_chain_len(const struct doca_devinfo *devinfo, enum doca_rdma_job_types job_type,
					     enum doca_rdma_transport_type transport_type,
					     uint32_t *max_buf_chain_len);

/**
 * Get the gid table size for a specific device.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [out] gid_table_size
 * The gid table size for the given devinfo.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_gid_table_size(struct doca_devinfo *devinfo, uint32_t *gid_table_size);

/**
 * Get gids for a specific device by index and number of entries.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [in] start_index
 * The first gid index of interest
 * @param [in] num_entries
 * The number of desired gid indicies
 * @param [in,out] gid_array
 * A 'struct doca_rdma_gid' array of size 'num_entries', that on success will hold the desired gids.
 * Note that it is the user's responsibility to provide an array with enough entries to prevent data corruption
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_gid(struct doca_devinfo *devinfo, uint32_t start_index, uint32_t num_entries,
			       struct doca_rdma_gid *gid_array);

/**
 * @brief Check if DOCA RDMA supports given transport type for a specific device.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [in] transport_type
 * Transport type to query support for.
 *
 * @return
 * DOCA_SUCCESS - in case the transport type is supported.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - failed to query device capabilities
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support the given transport type.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_transport_type_supported(const struct doca_devinfo *devinfo,
						    enum doca_rdma_transport_type transport_type);

/*********************************************************************************************************************
 * DOCA RDMA properties
 *********************************************************************************************************************/

/**
 * @brief Set send queue size property for doca_rdma.
 * The value can be queried using doca_rdma_get_send_queue_size().
 * Queue size will be rounded to the next power of 2.
 * can only be called before calling doca_ctx_start().
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] send_queue_size
 * Send queue size to use in context.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_NOT_SUPPORTED - if the given size is not supported.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_send_queue_size(struct doca_rdma *rdma, uint32_t send_queue_size);

/**
 * @brief Set recv queue size property for doca_rdma.
 * The value can be queried using doca_rdma_get_recv_queue_size().
 * Queue size will be rounded to the next power of 2.
 * Can only be called before calling doca_ctx_start().
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] recv_queue_size
 * Recv queue size to use in context.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_NOT_SUPPORTED - if the given size is not supported.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_recv_queue_size(struct doca_rdma *rdma, uint32_t recv_queue_size);


/**
 * @brief Set transport type for doca_rdma.
 * The value can be queried using doca_rdma_get_transport_type().
 * Can only be called before calling doca_ctx_start().
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] transport_type
 * Transport type to use in context.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_NOT_SUPPORTED - if the given transport type is not supported.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_transport_type(struct doca_rdma *rdma, enum doca_rdma_transport_type transport_type);

/**
 * @brief Set MTU for doca_rdma.
 * The value can be queried using doca_rdma_get_mtu().
 * Can only be called before calling doca_ctx_start().
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] mtu
 * MTU to use in context.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_NOT_SUPPORTED - if the given MTU is not supported.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 * - DOCA_ERROR_UNEXPECTED - if an unexpected error has occurred.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_mtu(struct doca_rdma *rdma, enum doca_mtu_size mtu);

/**
 * @brief Set rdma permissions for doca_rdma.
 * The value can be queried using doca_rdma_get_permissions().
 * Can only be called after calling doca_ctx_dev_add() and before calling doca_ctx_start().
 * The supported permissions are the RDMA access flags.
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] permissions
 * Bitwise combination of RDMA access flags - see enum doca_access_flags
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given or non-RDMA access flags were given.
 * - DOCA_ERROR_BAD_STATE - if context is already started or function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_permissions(struct doca_rdma *rdma, uint32_t permissions);

/**
 * @brief Set whether to use GRH in connection.
 * The value can be queried using doca_rdma_get_grh_enabled().
 * Can only be called before calling doca_ctx_start().
 *
 * If using IB device:
 * If GRH is disabled, the address will rely on LID only.
 * If GRH is enabled, the other side must also use GRH.
 *
 * If using ETH device, GRH must be enabled.
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] grh_enabled
 * GRH setting to use in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started or function was called before adding a device.
 * - DOCA_ERROR_NOT_SUPPORTED - if GRH setting is not supported for the device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_grh_enabled(struct doca_rdma *rdma, bool grh_enabled);

/**
 * @brief Set GID index for doca_rdma.
 * The value can be queried using doca_rdma_get_gid_index().
 * Can only be called before calling doca_ctx_start().
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] gid_index
 * GID index to use in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started or function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_gid_index(struct doca_rdma *rdma, uint32_t gid_index);

/**
 * @brief Set the maximal receive buffer chain length for doca_rdma.
 * The length may be increased and the value in use can be queried using doca_rdma_get_recv_buf_chain_len().
 * Can only be called before calling doca_ctx_start().
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] recv_buf_chain_len
 * recv_buf_chain_len to use in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_recv_buf_chain_len(struct doca_rdma *rdma, uint32_t recv_buf_chain_len);

/**
 * @brief Set SL (service level) for doca_rdma.
 * The value can be queried using doca_rdma_get_sl().
 * Can only be called before calling doca_ctx_start().
 *
 * @param [in] rdma
 * doca_rdma context to set the property for.
 * @param [in] sl
 * SL to use in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property set successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if context is already started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_set_sl(struct doca_rdma *rdma, uint32_t sl);

/**
 * @brief Get send queue size property from doca_rdma.
 * Returns the current send_queue_size set for the doca_rdma_context.
 * The size returned is the actual size being used and might differ from the size set by the user,
 * as the size may be increased.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] send_queue_size
 * Send queue size set in context.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_send_queue_size(const struct doca_rdma *rdma, uint32_t *send_queue_size);

/**
 * @brief Get recv queue size property from doca_rdma.
 * Returns the current recv_queue_size set for the doca_rdma_context.
 * The size returned is the actual size being used and might differ from the size set by the user,
 * as the size may be increased.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] recv_queue_size
 * Recv queue size set in context.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_recv_queue_size(const struct doca_rdma *rdma, uint32_t *recv_queue_size);

/**
 * @brief Get transport_type property from doca_rdma.
 * Returns the current transport_type set for the doca_rdma_context.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] transport_type
 * Transport_type set in context.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_transport_type(const struct doca_rdma *rdma, enum doca_rdma_transport_type *transport_type);

/**
 * @brief Get the MTU property from doca_rdma.
 * Returns the current MTU set for the doca_rdma context.
 * @note If MTU wasn't set by the user explicitly (and a default value was used), it may changed upon connection.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] mtu
 * MTU set in context.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_mtu(const struct doca_rdma *rdma, enum doca_mtu_size *mtu);

/**
 * @brief Get permissions property from doca_rdma.
 * Returns the current permissions set for the doca_rdma_context.
 * Can only be called after calling doca_ctx_dev_add().
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] permissions
 * Bitwise combination of RDMA access flags set in context - see enum doca_access_flags
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_permissions(struct doca_rdma *rdma, uint32_t *permissions);

/**
 * @brief Get GRH setting from doca_rdma.
 * Get the current GRH setting for doca_rdma.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] grh_enabled
 * GRH setting used in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_grh_enabled(const struct doca_rdma *rdma, bool *grh_enabled);

/**
 * @brief Get GID index from doca_rdma.
 * Get the current GID index set for doca_rdma.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] gid_index
 * GID index used in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_gid_index(const struct doca_rdma *rdma, uint32_t *gid_index);

/**
 * @brief Get the maximal receive buffer chain length from doca_rdma.
 * Get the current receive buffer chain length set for doca_rdma.
 * The returned value is the actual value being used and might differ from the size set by the user, as it may be
 * increased.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] recv_buf_chain_len
 * recv_buf_chain_len used in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_recv_buf_chain_len(const struct doca_rdma *rdma, uint32_t *recv_buf_chain_len);

/**
 * @brief Get SL (service level) from doca_rdma.
 * Get the current SL set for doca_rdma.
 *
 * @param [in] rdma
 * doca_rdma context to get the property from.
 * @param [out] sl
 * SL used in doca_rdma.
 *
 * @return
 * DOCA_SUCCESS - if property retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_sl(const struct doca_rdma *rdma, uint32_t *sl);

/**
 * @brief Get current state of doca_rdma.
 * Returns the current state of the doca_rdma_context.
 *
 * @param [in] rdma
 * doca_rdma context to get the state from.
 * @param [out] state
 * State of doca_rdma context.
 *
 * @return
 * DOCA_SUCCESS - if state retrieved successfully.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - if an invalid parameter was given.
 * - DOCA_ERROR_BAD_STATE - if function was called before adding a device.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_rdma_get_state(const struct doca_rdma *rdma, enum doca_rdma_state *state);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_RDMA_H_ */

/** @} */
