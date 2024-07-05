/*
 * Copyright (c) 2022-2023 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
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
 * @file doca_erasure_coding.h
 * @page DOCA_ERASURE_CODING
 * @defgroup DOCA_ERASURE_CODING DOCA Erasure Coding engine
 * DOCA Erasure Coding library. For more details please refer to the user guide on DOCA devzone.
 *
 * @{
 */
#ifndef DOCA_ERASURE_CODING_H_
#define DOCA_ERASURE_CODING_H_

#include <inttypes.h>

#include <doca_buf.h>
#include <doca_compat.h>
#include <doca_ctx.h>
#include <doca_dev.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************************
 * DOCA EC JOBS
 *********************************************************************************************************************/

/**
 * @brief Opaque structure representing a DOCA EC matrix (coding_matrix).
 */
struct doca_matrix;

/** @brief Available jobs for DOCA EC. */
enum doca_ec_job_types {
	DOCA_EC_JOB_GALOIS_MUL = DOCA_ACTION_EC_FIRST + 1, /**< Galois coding matrix multiplication job */
	DOCA_EC_JOB_CREATE, /**< Encoding job - create redundancy blocks (backup blocks) */
	DOCA_EC_JOB_UPDATE, /**< Update job - update redundancy blocks because a few data blocks were updated */
	DOCA_EC_JOB_RECOVER, /**< Recover job - recover lost data blocks by using the remaining data blocks and
			       redundancy blocks  */
};

/**
 * @brief Galois multiplication.
 * This job will galois multiply the src buffer with the coding matrix
 * and output the result to dst buffer
 * @note recommended to work with other jobs for EC workflow
 *
 * @note src_buff and dst_buff should be in multiplication of block size.
 * for example create job:
 *
 * coding matrix is 10x4 - 10 original blocks, 4 redundancy blocks
 * src_buff size : 10x64KB = 640KB
 * rdnc_buff size : 4x64KB = 256KB
 *
 * @note buff size should be at 64B padd. For example: 500B size should be padd to be 512B.
 * minimum 64B.
 *
 * @param [in] coding_matrix
 * create from doca_ec_matrix_from_raw
 * @param [in] src_buff
 * sequence containing data blocks - block_1, block_2 ,....
 * @param [in] dst_buff
 * sequence containing all multiplication outcome blocks - dst_block_1, dst_block_2 ,....
 */
struct doca_ec_job {
	struct doca_job base;		   /**< Common job data. */
	struct doca_matrix *coding_matrix; /**< coding matrix (see below doca_ec_matrix_from_raw) */
	struct doca_buf const *src_buff;   /**< Source data buffer. */
	struct doca_buf *dst_buff;	   /**< Destination data buffer. */
};

/**
 * @brief Jobs to be dispatched via EC library.
 * This job will galois multiply the src buffer with the coding matrix
 * and output the result to dst buffer
 *
 * @note src_buff and dst_buff should be in multiplication of block size.
 * for example create job:
 *
 * coding matrix is 10x4 - 10 original blocks, 4 redundancy blocks
 * src_buff size : 10x64KB = 640KB
 * rdnc_buff size : 4x64KB = 256KB
 *
 * @note buff size should be at 64B padd. For example: 500B size should be padd to be 512B.
 * minimum 64B.
 *
 * create job:
 * create redundancy blocks (backup blocks):
 * @param [in] create_matrix
 * create from doca_ec_matrix_gen
 * @param [in] src_original_data_buff
 * sequence containing all original blocks - block_1, block_2 ,.... (order do matter)
 * @param [in] dst_rdnc_buff
 * sequence containing all redundancy blocks - rdnc_block_1, rdnc_block_2 ,....
 */
struct doca_ec_job_create {
	struct doca_job base;		   /**< Common job data. */
	struct doca_matrix *create_matrix; /**< create matrix (see below doca_ec_matrix_gen) */
	struct doca_buf const *src_original_data_buff;   /**< Source original data buffer. */
	struct doca_buf *dst_rdnc_buff;	   /**< Destination redundancy data buffer. */
};

/**
 * @brief Update job
 * Update redundancy blocks because a few data blocks were updated
 *
 * @note src_buff and dst_buff should be in multiplication of block size.
 * for example create job:
 *
 * coding matrix is 10x4 - 10 original blocks, 4 redundancy blocks
 * src_buff size : 10x64KB = 640KB
 * rdnc_buff size : 4x64KB = 256KB
 *
 * @note buff size should be at 64B padd. For example: 500B size should be padd to be 512B.
 * minimum 64B.
 *
 * @param [in] update_matrix
 * create from doca_ec_update_matrix_gen
 * @param [in] src_data_rdnc_buff
 * sequence containing for each block old data, new data and then all redundancy blocks
 * - block_old_1, block_new_1, block_old_2, block_new_2 ,.... ,rdnc_block_1, rdnc_block_2 ,....
 * @param [in] dst_updated_rdnc_buff
 * sequence containing all redundancy blocks (after update) - rdnc_block1, rdnc_block2,....
 */
struct doca_ec_job_update {
	struct doca_job base;		   /**< Common job data. */
	struct doca_matrix *update_matrix; /**< update matrix (see below doca_ec_update_matrix_gen) */
	struct doca_buf const *src_data_rdnc_buff;   /**< Source old & new data with rdnc blocks. */
	struct doca_buf *dst_updated_rdnc_buff;	   /**< Destination updated redundancy blocks. */
};

/**
 * @brief recover job
 * recover lost data blocks by using the remaining data blocks and redundancy blocks
 *
 * @note src_buff and dst_buff should be in multiplication of block size.
 * for example create job:
 *
 * coding matrix is 10x4 - 10 original blocks, 4 redundancy blocks
 * src_buff size : 10x64KB = 640KB
 * rdnc_buff size : 4x64KB = 256KB
 *
 * @note buff size should be at 64B padd. For example: 500B size should be padd to be 512B.
 * minimum 64B.
 *
 * @param [in] recover_matrix
 * create from doca_ec_recover_matrix_gen
 * @param [in] src_remaining_data_buff
 * sequence containing all remaining original blocks and pad them with redundancy blocks at size of original data
 * - block_1, block_2, block_4 ,.... ,rdnc_block_1, rdnc_block_2 ,....
 * @param [in] dst_recovered_data_buff
 * sequence containing all missing/recovered blocks - block_3, block_5 ,....
 *
 */
struct doca_ec_job_recover {
	struct doca_job base;				/**< Common job data. */
	struct doca_matrix *recover_matrix;		/**< recover matrix (see below doca_ec_recover_matrix_create) */
	struct doca_buf const *src_remaining_data_buff; /**< Source remaining blocks buffer. */
	struct doca_buf *dst_recovered_data_buff;	/**< Destination data buffer. */
};

/*********************************************************************************************************************
 * DOCA EC Context
 *********************************************************************************************************************/

/**
 * @brief Opaque structure representing a DOCA EC instance.
 */
struct doca_ec;

/**
 * @brief Create a DOCA EC instance.
 *
 * @param [out] ec
 * Pointer to pointer to be set to point to the created doca_ec instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - ec argument is a NULL pointer.
 * - DOCA_ERROR_NO_MEMORY - failed to alloc doca_ec.
 * - DOCA_ERROR_INITIALIZATION - failed to initialize a mutex.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_create(struct doca_ec **ec);

/**
 * @brief Destroy a DOCA EC instance.
 *
 * @param [in] ec
 * Pointer to instance to be destroyed.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_IN_USE - if unable to gain exclusive access to the ec instance
 *                       or if one or more work queues are still attached. These must be detached first.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_destroy(struct doca_ec *ec);

/**
 * @brief Convert EC instance into context for use with workQ
 *
 * @param [in] ctx
 * EC instance. This must remain valid until after the context is no longer required.
 *
 * @return
 * Non NULL - doca_ctx object on success.
 * Error:
 * - NULL.
 */
__DOCA_EXPERIMENTAL
struct doca_ctx *doca_ec_as_ctx(struct doca_ec *ctx);

/**
 * @brief Check if given device is capable for given doca_ec job.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [in] job_type
 * doca_ec job type. See enum doca_ec_job_types.
 *
 * @return
 * DOCA_SUCCESS - in case the job is supported.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - failed to query device capabilities
 *                              or provided devinfo does not support the given doca_ec job.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_job_get_supported(struct doca_devinfo *devinfo, enum doca_ec_job_types job_type);

/**
 * @brief Get maximum buffer size for DOCA EC job.
 *
 * @param [in] devinfo
 * The DOCA device information
 * @param [in] job_type
 * doca_ec job type. See enum doca_ec_job_types.
 * @param [out] max_buffer_size
 * The max buffer size for DOCA EC operation in bytes.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - failed to query device capabilities.
 *                              or provided devinfo does not support the given doca_ec job.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_get_max_buffer_size(const struct doca_devinfo *devinfo, enum doca_ec_job_types job_type,
					 uint32_t *max_buffer_size);

/**
 * @brief Get the maximum supported number of elements in DOCA linked-list buffer for ec job.
 *
 * @param [in] devinfo
 * The DOCA device information.
 * @param [out] max_list_num_elem
 * The maximum supported number of elements in DOCA linked-list buffer.
 * The value 1 indicates that only a single element is supported.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_get_max_list_buf_num_elem(const struct doca_devinfo *devinfo, uint32_t *max_list_num_elem);

/*********************************************************************************************************************
 * DOCA EC Matrix Gen
 *********************************************************************************************************************/

/**
 * @brief Types of coding matrix used for erasure codes.
 */
enum doca_ec_matrix_types {
	/**
	 * Cauchy matrix of coding.
	 *
	 * Cauchy matrix example of encoding coefficients where high portion of matrix
	 * is identity matrix I and lower portion is constructed as 1/(i + j) | i != j,
	 * i:{0,k-1} j:{k,m-1}.  Any sub-matrix of a Cauchy matrix should be invertable.
	 *
	 * This is the recommended type to use.
	 */
	DOCA_CODING_MATRIX_CAUCHY = 1,
	/**
	 * Vandermonde matrix of coding.
	 *
	 * Vandermonde matrix example of encoding coefficients where high portion of
	 * matrix is identity matrix I and lower portion is constructed as 2^{i*(j-k+1)}
	 * i:{0,k-1} j:{k,m-1}. Commonly used method for choosing coefficients in
	 * erasure encoding but does not guarantee invertable for every sub matrix. For
	 * large pairs of m and k it is possible to find cases where the decode matrix
	 * chosen from sources and parity is not invertable. Users may want to adjust
	 * for certain pairs m and k. If m and k satisfy one of the following
	 * inequalities, no adjustment is required:
	 *
	 * - k <= 3
	 * - k = 4, m <= 25
	 * - k = 5, m <= 10
	 * - k <= 21, m-k = 4
	 * - m - k <= 3.
	 *
	 * Because this matrix does not guarantee invertable it is less recommended to use.
	 */
	DOCA_CODING_MATRIX_VANDERMONDE = 2,
};

/**
 * @brief Generate coding matrix for Erasure Code encode i.e. most basic encode matrix.
 * This is neccery for create job type, and other gen matrix
 *
 * @param [in] ctx
 * ctx of doca EC
 * @param [in] type
 * provided in enum doca_ec_matrix_types, the type should be consistent in recovery/update process.
 * @param [in] data_block_count
 * original data block count
 * @param [in] rdnc_block_count
 * redundancy block count
 * @param [out] matrix
 * output object
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_matrix_create(struct doca_ec *ctx, enum doca_ec_matrix_types type, size_t data_block_count,
				   size_t rdnc_block_count, struct doca_matrix **matrix);

/**
 * @brief Create coding matrix for Erasure Code encode i.e. most basic encode matrix from char array.
 * This is substitute for doca_ec_matrix_gen which is neccery for create job type, and other gen matrix
 *
 * @param [in] ctx
 * ctx of doca EC
 * @param [in] data
 * coding matrix in size data_block_count * rdnc_block_count
 * @param [in] data_block_count
 * original data block count
 * @param [in] rdnc_block_count
 * redundancy block count
 * @param [out] matrix
 * output object
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_matrix_from_raw(struct doca_ec *ctx, uint8_t *data, size_t data_block_count,
				     size_t rdnc_block_count, struct doca_matrix **matrix);

/**
 * @brief Generate coding matrix for Erasure Code update.
 * To use this function must call doca_ec_matrix_create before.
 *
 * @param [in] coding_matrix
 * matrix generated with gen function (see doca_ec_matrix_gen)
 * @param [in] ctx
 * ctx of doca EC
 * @param [in] update_indices
 * array containing which data to update (note that the indexes should match to the create function data block order)
 * @param [in] n_updates
 * update_indices count
 * @param [out] matrix
 * output object
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_update_matrix_create(const struct doca_matrix *coding_matrix, struct doca_ec *ctx,
					  uint32_t update_indices[], size_t n_updates, struct doca_matrix **matrix);
/**
 * @brief Generate coding matrix for Erasure Code recovery from failure.
 * To use this function must call doca_ec_matrix_create before.
 *
 * @param [in] coding_matrix
 * matrix generated with gen function (see doca_ec_matrix_gen)
 * @param [in] ctx
 * ctx of doca EC
 * @param [in] missing_indices
 * array containing which data blocks are missing (note that the indexes should match to the create function data block
 * order)
 * @param [in] n_missing
 * missing_indices count
 * @param [out] matrix
 * output object
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_recover_matrix_create(const struct doca_matrix *coding_matrix, struct doca_ec *ctx,
					   uint32_t missing_indices[], size_t n_missing, struct doca_matrix **matrix);

/**
 * @brief Destroy coding matrix used for EC job.
 *
 * @param [in] matrix
 * matrix generated with create function
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - in case of invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_ec_matrix_destroy(struct doca_matrix *matrix);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DOCA_EC_H_ */

/** @} */
