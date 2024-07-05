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
 * @file doca_sync_event.h
 * @page doca sync event
 * @defgroup SE DOCA Sync Event
 * @ingroup DOCACore
 * DOCA Sync Event
 * DOCA Sync Event is a software synchronization mechanism of parallel execution across the CPU, DPU, DPA, and GPU.
 * It is an abstraction around 64-bit value which can be updated, read, and waited upon
 * from any of these units to achieve synchronization between executions on them.
 * @{
 */

#ifndef DOCA_SYNC_EVENT_H_
#define DOCA_SYNC_EVENT_H_

#include <doca_compat.h>
#include <doca_error.h>
#include <doca_ctx.h>

#ifdef __cplusplus
extern "C" {
#endif

struct doca_sync_event;
struct doca_dpa_sync_event;
struct doca_gpu_sync_event;
struct doca_dpa;
struct doca_gpu;

/**
 * @brief Create a Sync Event handle.
 *
 * @details
 * Creates CPU handle - Host CPU or DPU's CPU.
 *
 * @param [out] event
 * The created doca_sync_event instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - event argument is a NULL pointer.
 * - DOCA_ERROR_NO_MEMORY - failed to alloc doca_sync_event.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_create(struct doca_sync_event **event);

/**
 * @brief Create a Sync Event handle from an export.
 *
 * @details
 * Creates a DPU handle.
 * The DOCA Device should be capable of importing an exported Sync Event
 * (see doca_sync_event_get_create_from_export_supported capability).
 *
 * @note
 * The Sync Event can only be configured and exported by the exporting process.
 *
 * @param [in] dev
 * doca_dev instance to be attached to the create doca_sync_event.
 *
 * @param [in] data
 * Exported doca_sync_event data stream, created by doca_sync_event_export_to_* call.
 *
 * @param [in] sz
 * Size of exported doca_sync_event data stream, created by doca_sync_event_export_to_* call.
 *
 * @param [out] event
 * The created doca_sync_event instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - provided doca_dev does not support creating Sync Event from export.
 * - DOCA_ERROR_NO_MEMORY - failed to alloc doca_sync_event.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_create_from_export(struct doca_dev *dev, const uint8_t *data, size_t sz, struct doca_sync_event **event);

/**
 * Check if given device is capable of creating Sync Event from an export.
 *
 * @param [in] devinfo
 * The DOCA device information.
 *
 * @return
 * DOCA_SUCCESS - in case device supports creating Sync Event from an export.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support importing an exported Sync Event.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_get_create_from_export_supported(const struct doca_devinfo *devinfo);

/**
 * @brief Destroy a Sync Event instance.
 *
 * @param [in] event
 * doca_sync_event to be destroyed.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - event argument is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_destroy(struct doca_sync_event *event);

/**
 * @brief Associate a CPU device context as the Sync Event Publisher.
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @param [in] dev
 * doca_dev instance associated with CPU.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_publisher_add_location_cpu(struct doca_sync_event *event, struct doca_dev *dev);

/**
 * @brief Associate a DOCA DPA context as the Sync Event Publisher.
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @param [in] dpa
 * doca_dpa instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_publisher_add_location_dpa(struct doca_sync_event *event, struct doca_dpa *dpa);

/**
 * @brief Associate a DOCA GPU context as the Sync Event Publisher.
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @param [in] gpu
 * doca_gpu instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_publisher_add_location_gpu(struct doca_sync_event *event, struct doca_gpu *gpu);

/**
 * @brief Declare Sync Event Publisher as the DPU.
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - event argument is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_publisher_add_location_dpu(struct doca_sync_event *event);

/**
 * Associate a CPU device context as the doca_sync_event Subscriber,
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @param [in] dev
 * doca_dev instance associated with CPU.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_subscriber_add_location_cpu(struct doca_sync_event *event, struct doca_dev *dev);

/**
 * @brief Associate a DOCA DPA context as the Sync Event Sublisher.
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @param [in] dpa
 * doca_dpa instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_subscriber_add_location_dpa(struct doca_sync_event *event, struct doca_dpa *dpa);

/**
 * @brief Associate a DOCA GPU context as the Sync Event Subscriber.
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @param [in] gpu
 * doca_gpu instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_subscriber_add_location_gpu(struct doca_sync_event *event, struct doca_gpu *gpu);

/**
 * @brief Declare Sync Event Publisher as the DPU.
 *
 * @param [in] event
 * Target doca_sync_event instance to set.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - event argument is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_subscriber_add_location_dpu(struct doca_sync_event *event);

/**
 * @brief Set the 64-bit value's address for a Sync Event.
 *
 * @details
 * Setting external address is allowed only for CPU/DPU configured Sync Event.
 *
 * @param [in] event
 * Pointer to se event instance to be configured.
 *
 * @param [in] addr
 * Allocated address pointer.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_BAD_STATE - setting address for event which has already been started is not allowed.
 * - DOCA_ERROR_NOT_SUPPORTED - addr is in unsupported address space.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_set_addr(struct doca_sync_event *event, uint64_t *addr);

/**
 * @brief Start a Sync Event to be operate as stand-alone DOCA Core object only.
 *
 * @details
 * Starting a Sync Event with doca_sync_event_start means it can't be operate as (and converted to) DOCA Context.
 *
 * @param [in] event
 * Pointer to se event instance to be started.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - event argument is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_start(struct doca_sync_event *event);

/**
 * @brief Stop a Sync Event which has been previously started with 'doca_sync_event_start'.
 *
 * @param [in] event
 * Pointer to se event instance to be stoped.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - event argument is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_stop(struct doca_sync_event *event);

/**
 * @brief Convert a Sync Event to a DOCA context.
 *
 * @details
 * Set the Sync Event to operate as a DOCA Context only,
 * hence it can be interacted with through the supported DOCA Context API.
 *
 * Sync Event CTX supports the following operations: start/stop/workq_add/workq_rm/get_event_driven_supported.
 * A device can't be attached to a sync event ctx.
 *
 * A user can use an attached (to Sync Event CTX) DOCA WorkQ
 * to perform operations on the underlying Sync Event asynchronously
 * by submitting jobs to the attached DOCA WorkQ(see enum doca_sync_event_job_types).
 *
 * It is suggested to use Sync Event in this mode to wait on a Sync Event in a blocking manner.
 *
 * @param [in] event
 * The doca_sync_event to be converted
 *
 * @return
 * The matching doca_ctx instance in case of success,
 * NULL otherwise.
 */
__DOCA_EXPERIMENTAL
struct doca_ctx *doca_sync_event_as_ctx(struct doca_sync_event *event);

/**
 * Sync Event job types for performing operation on a Sync Event thorugh DOCA job submission on a DOCA WorkQ
 */
enum doca_sync_event_job_types {
	DOCA_SYNC_EVENT_JOB_WAIT_GT = DOCA_ACTION_SYNC_EVENT_FIRST + 1, /**< Wait for the Sync Event to be grater than some value */
	DOCA_SYNC_EVENT_JOB_GET,                                        /**< Get the value of the Sync Event */
	DOCA_SYNC_EVENT_JOB_UPDATE_SET,                                 /**< Set the Sync Event value with some value */
	DOCA_SYNC_EVENT_JOB_UPDATE_ADD,                                 /**< Increment atomically the Sync Event value by some value */
};

/**
 * Result of a Sync Event job. Held inside the doca_event::result.u64 field.
 */
struct doca_sync_event_result {
	doca_error_t result; /**< Operation status */
};

/**
 * @brief Check if a given device is capable of executing a specific Sync Event job.
 *
 * @param [in] devinfo
 * The DOCA device information.
 *
 * @param [in] type
 * doca_sync_event_job_types available for this device. see enum doca_sync_event_job_types.
 *
 * @return
 * DOCA_SUCCESS - in case device supports the Sync Event job type.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support this Sync Event job.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_job_get_supported(struct doca_devinfo *devinfo, enum doca_sync_event_job_types type);

/**
 * Wait job to be dispatched on a DOCA WorkQ which attached to some Sync Event.
 * Wait for the Sync Event to be grater than the given value.
 */
struct doca_sync_event_job_wait {
	struct doca_job base; /**< Inherits from DOCA Job */
	uint64_t value;       /**< Threshold to wait for the Sync Event to be grater than.
				*  Valid values must be in the range [0, 254] and
				*  can be submitted for event with value in the range [0, 254] -
				*  other scenarios result in undefined behaviour. */
	uint64_t mask;        /**< Mask for comparing the Sync Event value -
				*  mask must be consistent only of 0, 1, 2, 4 or 8 consecutive FFs */
};

/**
 * Get job to be dispatched on a DOCA WorkQ which attached to the Sync Event.
 * Get the value of the Sync Event.
 */
struct doca_sync_event_job_get {
	struct doca_job base; /**< Inherits from DOCA Job */
	uint64_t *value; /**< The Sync Event value */
};

/**
 * Set job to be dispatched on a DOCA WorkQ which attached to some Sync Event.
 * Set the value of the Sync Event to the given value.
 */
struct doca_sync_event_job_update_set {
	struct doca_job base; /**< Inherits from DOCA Job */
	uint64_t value; /**< Value to set the Sync Event to */
};



/**
 * Add job to be dispatched on a DOCA WorkQ which attached to some Sync Event.
 * Atomically increment the value of the Sync Event by the given value.
 */
struct doca_sync_event_job_update_add {
	struct doca_job base; /**< Inherits from DOCA Job */
	uint64_t value; /**< Value to set the Sync Event to */
	uint64_t *fetched; /**< Fetched Sync Event value */
};

/**
 * @brief Export Sync Event to be shared with the DPU.
 *
 * @details
 * Create export data stream used for synchronize between the x86 CPU HOST to DPU ARM.
 * Sync Event should be properly configured, both subscriber and publisher must be declared as either CPU or DPU location.
 * The underlying DOCA Device should be capable of exporting to DPU (see doca_sync_event_get_export_to_dpu_supported capability).
 * A Sync Event can be exported from the Host CPU only.
 *
 * The exported data stream an be used from the DPU to created an exported Sync Event
 * (see doca_sync_event_create_from_export).
 *
 * @param [in] event
 * Target doca_sync_event instance to export.
 *
 * @param [in] dev
 * Target dev to export.
 *
 * @param [out] data
 * The created export data stream.
 *
 * @param [out] sz
 * Size of created export data stream.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support this Sync Event action.
 * - DOCA_ERROR_NO_MEMORY - failed to alloc data stream.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_export_to_dpu(struct doca_sync_event *event, struct doca_dev *dev, const uint8_t **data, size_t *sz);

/**
 * Check if a DOCA device is capable of exporting an associated Sync Event to the DPU
 * using doca_sync_event_export_to_dpu.
 *
 * @param [in] devinfo
 * The DOCA device information.
 *
 * @return
 * DOCA_SUCCESS - in case device supports exporting an associated Sync Event to DPU.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support exporting an associated Sync Event to DPU.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_get_export_to_dpu_supported(const struct doca_devinfo *devinfo);

/** DPA sync event handle type definition */
typedef uint64_t doca_dpa_dev_sync_event_t;

/**
 * @brief Export Sync Event to be shared with the DPA.
 *
 * @details
 * Create Sync Event DPA handle used for synchronize between the x86 CPU HOST and the DPA.
 * Sync Event should be properly configured, either subscriber or publisher should be declared as DPA location.
 * The underlying DOCA Device should be capable of exporting to DPA (see doca_sync_event_get_export_to_dpa_supported capability).
 * A Sync Event can be exported from the Host CPU only.
 *
 * The DOCA DPA Sync Event is an handle to be used from the DPA to perform operations on the associated Sync Event.
 *
 * @param [in] event
 * Target doca_sync_event instance to export.
 *
 * @param [in] dpa
 * The associated DOCA DPA Context.
 *
 * @param [out] dpa_dev_se_handle
 * DOCA DPA device sync event handle that can be passed to a kernel.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support this Sync Event action.
 * - DOCA_ERROR_NO_MEMORY - failed to alloc doca_dpa_sync_event.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_export_to_dpa(struct doca_sync_event *event, struct doca_dpa *dpa, doca_dpa_dev_sync_event_t *dpa_dev_se_handle);

/**
 * Check if a DOCA device is capable of exporting an associated Sync Event to the DPA
 * using doca_sync_event_export_to_dpa.
 *
 * @param [in] devinfo
 * The DOCA device information.
 *
 * @return
 * DOCA_SUCCESS - in case device supports exporting an associated Sync Event to DPA.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support exporting an associated Sync Event to DPA.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_get_export_to_dpa_supported(const struct doca_devinfo *devinfo);

/** GPU sync event handle type definition */
typedef uint64_t doca_gpu_dev_sync_event_t;

/**
 * @brief Export Sync Event to be shared with the GPU.
 *
 * @details
 * Create Sync Event GPU handle used for synchronize between the x86 CPU HOST and the DPA.
 * Sync Event should be properly configured, either subscriber or publisher should be declared as GPU location.
 * The underlying DOCA Device should be capable of exporting to GPU (see doca_sync_event_get_export_to_gpu_supported capability).
 * A Sync Event can be exported from the Host CPU only.
 *
 * The DOCA GPU Sync Event is an handle to be used from the GPU to perform operations on the associated Sync Event.
 *
 * @param [in] event
 * Target doca_sync_event instance to export.
 *
 * @param [in] gpu
 * The associated DOCA GPU Context.
 *
 * @param [out] gpu_dev_se
 * DOCA GPU device sync event handle that can be passed to a kernel.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support this Sync Event action.
 * - DOCA_ERROR_NO_MEMORY - failed to alloc doca_gpu_sync_event.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_export_to_gpu(struct doca_sync_event *event, struct doca_gpu *gpu, doca_gpu_dev_sync_event_t **gpu_dev_se);

/**
 * Check if a DOCA device is capable of exporting an associated Sync Event to the GPU
 * using doca_sync_event_export_to_gpu.
 *
 * @param [in] devinfo
 * The DOCA device information.
 *
 * @return
 * DOCA_SUCCESS - in case device supports exporting an associated Sync Event to GPU.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - provided devinfo does not support exporting an associated Sync Event to GPU.
 *
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_get_export_to_gpu_supported(const struct doca_devinfo *devinfo);


typedef struct doca_sync_event_remote {
	uint64_t data[2];
} doca_sync_event_remote_t;

/**
 * @brief Obtain an handle for interacting with the associated DOCA Syn Event from a remote node
 *
 * @param [in] event
 * Target doca_sync_event instance to export for remote.
 *
 * @param [out] handle
 * The remote handle associated with the given doca_sync_event instance.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_export_remote(struct doca_sync_event *event, doca_sync_event_remote_t *handle);


/**
 * @brief Get the value of a Sync Event synchronously.
 *
 * @param [in] event
 * Target doca_sync_event instance to read its value.
 *
 * @param [out] value
 * The returned doca_sync_event value.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_get(struct doca_sync_event *event, uint64_t *value);

/**
 * @brief Atomically increase the value of a Sync Event by some value synchronously.
 *
 * @param [in] event
 * Target doca_sync_event instance to increment.
 *
 * @param [in] value
 * The value to increment the doca_sync_event value by.
 *
 * @param [out] fetched
 * The value of the doca_sync_event before the operation.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_update_add(struct doca_sync_event *event, uint64_t value, uint64_t *fetched);

/**
 * @brief Set the value of a Sync Event to some value synchronously.
 *
 * @param [in] event
 * Target doca_sync_event instance to set its value.
 *
 * @param [in] value
 * The value to set the doca_sync_event to.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_update_set(struct doca_sync_event *event, uint64_t value);

/**
 * @brief Wait for the value of a Sync Event to reach some value synchronously in a polling busy wait manner.
 *
 * @param [in] event
 * Target doca_sync_event instance to wait on.
 *
 * @param [in] value
 * The value to wait for the doca_sync_event to be greater than.
 *
 * @param [in] mask
 * Mask to apply (bitwise AND) on the doca_sync_event value for comparison with wait threshold.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_wait_gt(struct doca_sync_event *event, uint64_t value, uint64_t mask);

/**
 * @brief Wait for the value of a Sync Event to reach some value synchronously in a periodically busy wait manner.
 *
 * @details
 * After each polling iteration, call sched_yield
 * sched_yield() causes the calling thread to relinquish the CPU.
 * The thread is moved to the end of the queue for its static priority and a new thread gets to run.
 *
 * @param [in] event
 * Target doca_sync_event instance to wait on.
 *
 * @param [in] value
 * The value to wait for the doca_sync_event to be greater than.
 *
 * @param [in] mask
 * Mask to apply (bitwise AND) on the doca_sync_event value for comparison with wait threshold.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * doca_error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - any of the arguments is a NULL pointer.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_sync_event_wait_gt_yield(struct doca_sync_event *event, uint64_t value, uint64_t mask);


#ifdef __cplusplus
}
#endif

/** @} */

#endif /* DOCA_SYNC_EVENT_H_ */
