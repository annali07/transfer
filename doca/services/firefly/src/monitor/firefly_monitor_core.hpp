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

#ifndef FIREFLY_MONITOR_CORE_H_
#define FIREFLY_MONITOR_CORE_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <doca_error.h>

#define FIREFLY_MONITOR_VERSION "1.2.0"		/* Firefly's version */

#define INVALID_VALUE_STRING "NA" 		/* Mark a failure to find the real value */
#define MAX_TIME_STR_LEN 48
#define CANONICAL_ID_LEN 64

typedef enum PTP_STATE {
	STATE_STABLE,				/* PTP is in a stable state */
	STATE_FAULTY,				/* PTP is currently out of sync */
	STATE_RECOVERED				/* PTP managed to recover from a sync error */
} ptp_state_t;

struct sampling_value {
	int64_t max;				/* Maximal (abs) sampled value */
	int64_t average;			/* Average value across samples */
};

union raw_sample {
	struct sampling_value sample;		/* Hold the full analyzed information */
	int64_t raw;				/* Hold a single raw sampling value */
};

/*
 * The set of details we collect regarding the PTP state, IEEE 1588 field info was inspired by the following:
 * https://github.com/YangModels/yang/blob/main/experimental/ieee/1588/ni-ieee1588-ptp.yang
 */
struct ptp_info {
	bool gmPresent;				/* Is the Grandmaster present? */
	ptp_state_t ptp_stability;		/* Stability state of the PTP */
	char ptp_time[MAX_TIME_STR_LEN];	/* The accurate timestamp in string format */
	char sys_time[MAX_TIME_STR_LEN];	/* The system's time in string format */
	int32_t error_count;			/* Number of errors we encountered thus far */
	char last_error_time[MAX_TIME_STR_LEN];	/* Timestamp for the last encountered error */
	char gmIdentity[CANONICAL_ID_LEN];	/* Grandmaster identity (canonicalized string) */
	char portIdentity[CANONICAL_ID_LEN];	/* Our own identity (canonicalized string) */
	union raw_sample master_offset;		/* Offset from the master clock (in nano seconds) */
	int64_t currentUtcOffset;		/* Current offset from UTC */
	bool timeTraceable;			/* PTP timeTraceable property */
	bool frequencyTraceable;		/* PTP frequencyTraceable property */
	uint8_t grandmasterPriority1;		/* Priority1 field of the Grandmaster clock */
	uint8_t gmClockClass;			/* Clock class of the Grandmaster clock */
	uint8_t gmClockAccuracy;		/* Clock accuracy of the Grandmaster clock */
	uint8_t grandmasterPriority2;		/* Priority2 field of the Grandmaster clock */
	uint16_t gmOffsetScaledLogVariance;	/* Offset scaled log variance of the Grandmaster clock */
};

/*
 * Translate the stability state into a user-facing string
 *
 * @state [in]: Stability state enum
 * @return: User-facing stability state string
 */
const char * get_stability_string(ptp_state_t state);

/*
 * Report the version of the program and exit.
 *
 * @param [in]: Unused (came from DOCA ARGP)
 * @doca_config [in]: Unused (came from DOCA ARGP)
 * @return: The function exits with EXIT_SUCCESS
 */
[[noreturn]] doca_error_t firefly_monitor_version_callback(void *param, void *doca_config);

/*
 * Report the results from the monitoring round to the standard output
 *
 * @ptp_state [in]: PTP State to be reported
 * @return: DOCA_SUCCESS on success and DOCA_ERROR otherwise
 */
doca_error_t report_monitoring_result_to_stdout(struct ptp_info *ptp_state);

#endif /* FIREFLY_MONITOR_CORE_H_ */
