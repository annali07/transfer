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
 * Definition of an abstract implementation of IPSec protocol offload.
 */

/**
 * @file doca_ipsec.h
 * @page ipsec
 * @defgroup DOCA_IPSEC IPsec
 * DOCA IPSEC library. For more details please refer to the user guide on DOCA devzone.
 *
 * @{
 */

#ifndef DOCA_IPSEC_H_
#define DOCA_IPSEC_H_

#include <doca_buf.h>
#include <doca_compat.h>
#include <doca_ctx.h>
#include <doca_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief IPSec encryption key type */
enum doca_encryption_key_type {
	DOCA_ENCRYPTION_KEY_AESGCM_128, /**< size of 128 bit */
	DOCA_ENCRYPTION_KEY_AESGCM_256, /**< size of 256 bit */
};

/** @brief IPSec encryption key */
struct doca_encryption_key {
	enum doca_encryption_key_type type; /**< size of enc key */
	union {
		struct {
			uint64_t implicit_iv; /**< The IV is inserted into the GCM engine is calculated by */
			uint32_t salt;	      /**< The salt is inserted into the GCM engine is calculated by */
			void *raw_key;	      /**< Raw key buffer. Actual size of this buffer defined by type. */
		} aes_gcm;
	};
};

/** @brief IPSec replay window size */
enum doca_ipsec_replay_win_size {
	DOCA_IPSEC_REPLAY_WIN_SIZE_32 = 32,   /**< size of 32 bit */
	DOCA_IPSEC_REPLAY_WIN_SIZE_64 = 64,   /**< size of 64 bit */
	DOCA_IPSEC_REPLAY_WIN_SIZE_128 = 128, /**< size of 128 bit */
	DOCA_IPSEC_REPLAY_WIN_SIZE_256 = 256, /**< size of 256 bit */
};

/** @brief IPSec icv length */
enum doca_ipsec_icv_length {
	DOCA_IPSEC_ICV_LENGTH_8 = 8,   /**< size of 8 bit */
	DOCA_IPSEC_ICV_LENGTH_12 = 12, /**< size of 12 bit */
	DOCA_IPSEC_ICV_LENGTH_16 = 16, /**< size of 16 bit */
};

/** @brief IPSec direction of the key, incoming packets or outgoing */
enum doca_ipsec_direction {
	DOCA_IPSEC_DIRECTION_INGRESS_DECRYPT = 0, /**< incoming packets, decription */
	DOCA_IPSEC_DIRECTION_EGRESS_ENCRYPT = 1	  /**< outgoing packets, encription */
};

/** @brief IPSec sa events attributes - when turned on will trigger an event */
struct doca_ipsec_sa_event_attrs {
	uint32_t remove_flow_packet_count;
	/**< Packet counter, Decrements for every packet passing through the SA.
	 * Event are triggered occurs when the counter reaches soft- lifetime and hard-lifetime (0).
	 * When counter reaches hard-lifetime, all passing packets will return a relevant Syndrome.
	 */
	uint32_t remove_flow_soft_lifetime;
	/**< Soft Lifetime threshold value.
	 * When remove_flow_packet_count reaches this value a soft lifetime event is triggered (if armed).
	 * See remove_flow_packet_count field in this struct fro more details.
	 */
	uint32_t soft_lifetime_arm : 1;
	/**< 1 when armed/to arm 0 otherwise. */
	uint32_t hard_lifetime_arm : 1;
	/**< 1 when armed/to arm 0 otherwise. */
	uint32_t remove_flow_enable : 1;
	/**< 1 when remove flow enabled/to enable; 0 otherwise. */
	uint32_t esn_overlap_event_arm : 1;
	/**< 1 when armed/to arm 0 otherwise. */
};

/** @brief IPSec sa sn attributes - attributes for sequence number - only if SN or AR enabled */
struct doca_ipsec_sa_attr_sn {
	uint32_t esn_overlap : 1; /**< new/old indication of the High sequence number MSB - when set is old */
	uint32_t esn_enable : 1;  /**< when set esn is enabled */
	uint64_t sn_initial;	  /**< set the initial sequence number - in antireplay set the lower bound of the window */
};

/** @brief IPSec sa egress attributes - attributes for outgoing data */
struct doca_ipsec_sa_attr_egress {
	uint32_t sn_inc_enable : 1; /**< when set sn increment offloaded */
};

/** @brief IPSec sa egress attributes - attributes for incoming data */
struct doca_ipsec_sa_attr_ingress {
	uint32_t antireplay_enable : 1;
	/**< when enabled activates anti-replay protection window. */
	enum doca_ipsec_replay_win_size replay_win_sz;
	/**< Anti replay window size to enable sequence replay attack handling. */
};

/** @brief IPSec attributes for create jobs */
struct doca_ipsec_sa_attrs {
	struct doca_encryption_key key;					/**< IPSec encryption key */
	enum doca_ipsec_icv_length icv_length;			/**< Authentication Tag length */
	struct doca_ipsec_sa_attr_sn sn_attr;			/**< sn attributes */
	enum doca_ipsec_direction direction;			/**< egress/ingress */
	union {							/**< egress/ingress attr */
		struct doca_ipsec_sa_attr_egress egress;	/**< egress attr */
		struct doca_ipsec_sa_attr_ingress ingress;	/**< ingress attr */
	};
	struct doca_ipsec_sa_event_attrs event;			/**< Reserve future use - ipsec events flags */
};

/**
 * @brief Doca ipsec action type enums, used to specify ipsec job types.
 */
enum doca_ipsec_job_types {
	DOCA_IPSEC_JOB_SA_CREATE = DOCA_ACTION_IPSEC_FIRST + 1, /**< create sa object */
	DOCA_IPSEC_JOB_SA_DESTROY,				/**< destroy sa object */
};

/**
 * @brief DOCA IPSec SA opaque handle.
 * This object should be passed to DOCA Flow to create enc/dec action
 */
struct doca_ipsec_sa;

/**
 * @brief DOCA IPSec SA creation job.
 *
 * The result of this job if doca_workq_progress_retrieve returns:
 * - DOCA_SUCCESS - struct doca_event { .result.ptr } should point to new created `struct doca_ipsec_sa` object.
 * - DOCA_ERROR_IO_FAILED - struct doca_event { .result.u64 } should contain IPSec CTX specific error status code.
 */
struct doca_ipsec_sa_create_job {
	struct doca_job base;		     /**< doca job object */
	struct doca_ipsec_sa_attrs sa_attrs; /**< ipsec sa attr */
};

/**
 * @brief DOCA IPSec SA destroy job.
 *
 * The result of this job as struct doca_event { .result.u64 } should contain SA destroy completion status code.
 */
struct doca_ipsec_sa_destroy_job {
	struct doca_job base;	  /**< doca job object */
	struct doca_ipsec_sa *sa; /**< ipsec sa object (from create) */
};

/**
 * @brief Opaque structure representing a doca ipsec instance.
 */
struct doca_ipsec;

/**
 * @brief Create a DOCA IPSEC instance.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ipsec_create(struct doca_ipsec **ctx);

/**
 * @brief Destroy DOCA IPSEC instance.
 *
 * @param [in] ctx
 * Instance to be destroyed, MUST NOT BE NULL.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_IN_USE - the ctx still in use by one or more workQs.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ipsec_destroy(struct doca_ipsec *ctx);

/**
 * @brief Convert IPSec instance into context for use with workQ
 *
 * @param [in] ctx
 * IPSEC instance. This must remain valid until after the context is no longer required.
 *
 * @return
 * Non NULL - doca_ctx object on success.
 * Error:
 * - NULL.
 */
__DOCA_EXPERIMENTAL
struct doca_ctx *doca_ipsec_as_ctx(struct doca_ipsec *ctx);

/**
 * @brief set the sa pool size for sa objects that are return by doca_ipsec_sa_create_job
 *
 * @note The range of valid values for this property depend upon the device in use. This means that acceptance of a
 * value through this API does not ensure the value is acceptable, this will be validated as part of starting the
 * context
 *
 * @param [in] ctx
 * IPSEC instance.
 *
 * @param [in] pool_size
 * Number of items to have available. default is 20,000
 *
 * @return
 * DOCA_SUCCESS - Property was successfully set
 * Error code - in case of failure:
 * DOCA_ERROR_INVALID_VALUE - received invalid input.
 * DOCA_ERROR_NO_LOCK - Unable to gain exclusive control of ipsec instance.
 * DOCA_ERROR_IN_USE - ipsec instance is currently started.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ipsec_set_sa_pool_size(struct doca_ipsec *ctx, uint32_t pool_size);

/**
 * @brief Convert IPSec event job into sa object
 *
 * @param [in] ev
 * event of ipsec job
 *
 * @return
 * Non NULL - sa object of ipsec.
 * Error:
 * - NULL.
 */
__DOCA_EXPERIMENTAL
struct doca_ipsec_sa *doca_ipsec_sa_from_result(struct doca_event *ev);

/**
 * @brief Check if given device is capable for given doca_ipsec job.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [in] job_type
 * doca_ipsec job type. See enum doca_ipsec_job_types.
 *
 * @return
 * DOCA_SUCCESS - in case the job is supported.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - failed to query device capabilities
 *                              or provided devinfo does not support the given doca_ipsec job.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ipsec_job_get_supported(struct doca_devinfo *devinfo, enum doca_ipsec_job_types job_type);

/**
 * @brief Get is device support sn_enabled capabilities
 *
 * @param [in] devinfo
 * The DOCA device information
 *
 * @return
 * DOCA_SUCCESS - in case of success - capability supported.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - failed to query device capabilities
 *                              or provided devinfo does not support the given capabilitie.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ipsec_sequence_number_get_supported(const struct doca_devinfo *devinfo);

/**
 * @brief Get is device support antireplay_enable capabilities
 *
 * @param [in] devinfo
 * The DOCA device information
 *
 * @return
 * DOCA_SUCCESS - in case of success - capability supported.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - failed to query device capabilities
 *                              or provided devinfo does not support the given capabilitie.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ipsec_antireplay_get_supported(const struct doca_devinfo *devinfo);

/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_IPSEC_H_ */
