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
 * @defgroup PCC_HOST PCC Host
 * DOCA PCC Host library. For more details please refer to the user guide on DOCA devzone.
 *
 * @ingroup PCC
 *
 * @{
 */
#ifndef DOCA_PCC_H_
#define DOCA_PCC_H_

#include <doca_compat.h>
#include <doca_error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations
 */
struct doca_devinfo;
struct doca_dev;
struct doca_pcc;

/**
 * \brief Opaque representation of a PCC Application
 *
 * This is an opaque structure that encapsulates a PCC application.
 * Typically, the DOCA PCC Host application will obtain the value
 * of this structure by linking in the appropriate stub library that is generated by DPACC
 */
struct doca_pcc_app;

/**
 * @brief Get whether the DOCA device supports PCC
 *
 * @param[in]  devinfo - The device to query
 *
 * @return
 * DOCA_SUCCESS - in case of the DOCA device quered has PCC support
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 * - DOCA_ERROR_NOT_SUPPORTED - the device quered does not support PCC
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_devinfo_get_is_pcc_supported(const struct doca_devinfo *devinfo);

/**
 * @brief Create PCC context
 *
 * This function creates a DOCA PCC context given a DOCA device to capture and route PCC events to the DPA.
 *
 * @param[in]  doca_dev - DOCA device
 * @param[out]  pcc - Created PCC context
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 * - DOCA_ERROR_DRIVER - in case of error in a DOCA driver call
 * - DOCA_ERROR_NOT_SUPPORTED - the device does not support PCC
 * - DOCA_ERROR_NO_MEMORY - in case of failure in internal memory allocation
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_create(struct doca_dev *doca_dev, struct doca_pcc **pcc);

/**
 * \brief Destroy a DOCA PCC context
 *
 * This function destroys PCC context created by doca_pcc_create()
 * When the termination is started the process will stop handling PCC events.
 * Issueing a ^c during doca_pcc_wait(...) will also result in the application's termination.
 *
 * @param[in]  pcc - Previously created PCC context
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 * - DOCA_ERROR_DRIVER - in case of error in a DOCA driver call
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_destroy(struct doca_pcc *pcc);

/**
 * @brief Get a minimal required number of threads handling CC events
 *
 * @param[in]  pcc - PCC context
 * @param[in]  min_num_threads - minimal number of threads used by pcc
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid NULL input
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_get_min_num_threads(struct doca_pcc *pcc, uint32_t *min_num_threads);

/**
 * @brief Get a maximal allowed number of threads handling CC events
 *
 * @param[in]  pcc - PCC context
 * @param[in]  max_num_threads - maximal number of threads used by pcc
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid NULL input
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_get_max_num_threads(struct doca_pcc *pcc, uint32_t *max_num_threads);

/**
 * @brief Set program app for PCC context
 *
 * The context represents a program on the DPA that is referenced
 * by the host process that called the context creation API.
 * Must be set before calling doca_pcc_start()
 *
 * @param[in]  pcc - PCC context
 * @param[in]  app - PCC application generated by DPACC
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 * - DOCA_ERROR_BAD_STATE - if PCC context is already started
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_set_app(struct doca_pcc *pcc, struct doca_pcc_app *app);

/**
 * @brief Configure affinity of threads handling CC events
 *
 * Must be set before calling doca_pcc_start()
 *
 * @param[in]  pcc - PCC context
 * @param[in]  num_threads - number of threads used by pcc. Should be constarined by minimum and maximum allowed number
 * (see doca_pcc_get_min_num_threads and doca_pcc_get_max_num_threads)
 * @param[in]  affinity_configs - array of indexes to assign to threads
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid null input or invalid number of threads
 * - DOCA_ERROR_BAD_STATE - if PCC context is already started
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_set_thread_affinity(struct doca_pcc *pcc, uint32_t num_threads, uint32_t *affinity_configs);

/**
 * @brief Start a PCC context
 * Register the pcc process in the NIC hw.
 *
 * @param[in]  pcc - PCC context
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 * - DOCA_ERROR_DRIVER - in case of error in a DOCA driver call
 * - DOCA_ERROR_NO_MEMORY - in case of failure in internal memory allocation
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_start(struct doca_pcc *pcc);

/**
 * @brief Stop a PCC context
 *
 * @param[in]  pcc - PCC context
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 * - DOCA_ERROR_DRIVER - in case of error in a DOCA driver call
 * - DOCA_ERROR_BAD_STATE - in case pcc is not started
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_stop(struct doca_pcc *pcc);

/**
 * @brief Wait on events or timeout from device for given time in seconds.
 *
 * Providing a negative value for wait time will cause the context to wait on events until the user terminates it.
 *
 * @param[in]  pcc - PCC context
 * @param[in]  wait_time - time in seconds to wait
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 * - DOCA_ERROR_BAD_STATE - in case pcc is not started
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_wait(struct doca_pcc *pcc, int wait_time);

/**
 * @brief Process states
 */
typedef enum {
	DOCA_PCC_PS_ACTIVE = 0,
	/**< The process handles CC events (only one process is active at a given time) */
	DOCA_PCC_PS_STANDBY = 1,
	/**< The process is in standby mode (another process is already ACTIVE)*/
	DOCA_PCC_PS_DEACTIVATED = 2,
	/**< The process was deactivated by NIC FW and should be destroyed */
	DOCA_PCC_PS_ERROR = 3,
	/**< The process is in error state and should be destroyed */
} doca_pcc_process_state_t;

/**
 * @brief Return the state of the process.
 *
 * @param[in]  pcc - PCC context
 * @param[out]  process_state - state of the PCC process. In case positive wait_time is specified and expired, DEACTIVATED state will be returned.
 *
 * @return
 * DOCA_SUCCESS - in case of success
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_pcc_get_process_state(const struct doca_pcc *pcc, doca_pcc_process_state_t *process_state);

#ifdef __cplusplus
}
#endif

#endif /* DOCA_PCC_H_ */

/** @} */