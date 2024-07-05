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

#include <bsd/string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <doca_error.h>
#include <doca_log.h>

#include "firefly_monitor_core.hpp"

DOCA_LOG_REGISTER(FIREFLY_MONITOR);

const char *
get_stability_string(ptp_state_t state)
{
	switch (state) {
	case STATE_STABLE:
		return "Yes";
	case STATE_FAULTY:
		return "No";
	case STATE_RECOVERED:
		return "Recovered";
	default:
		return INVALID_VALUE_STRING;
	}
}

[[noreturn]] doca_error_t
firefly_monitor_version_callback(void *param, void *doca_config)
{
	printf("DOCA SDK Version: %s\n", doca_version());
	printf("DOCA Firefly Version: %s\n", FIREFLY_MONITOR_VERSION);
	/* We assume that when printing the versions there is no need to continue the program's execution */
	exit(EXIT_SUCCESS);
}

doca_error_t
report_monitoring_result_to_stdout(struct ptp_info *ptp_state)
{
	int i = 0;

	printf("\n\n");
	if (ptp_state->gmPresent) {
		/* gmIdentity:			EC:46:70:FF:FE:10:FE:B9 (ec4670.fffe.10feb9) */
		printf("gmIdentity:                %s\n",	ptp_state->gmIdentity);
		/* portIdentity:		EC:46:70:FF:FE:10:FE:B9 (ec4670.fffe.10feb9-1) */
		printf("portIdentity:              %s\n", 	ptp_state->portIdentity);
		/* master_offset:		23 */
		printf("master_offset (max):       %ld\n",	ptp_state->master_offset.sample.max);
		printf("master_offset (avg):       %ld\n",	ptp_state->master_offset.sample.average);
		/* gmPresent:			true */
		printf("gmPresent:                 true\n");
		/* ptp_stable:			Yes/No/Recovered */
		printf("ptp_stable:                %s\n",	get_stability_string(ptp_state->ptp_stability));
		/* currentUtcOffset:		37 */
		printf("UtcOffset:                 %ld\n",	ptp_state->currentUtcOffset);
		/* timeTraceable:		1 */
		printf("timeTraceable:             %s\n",	(ptp_state->timeTraceable ? "1" : "0"));
		/* frequencyTraceable:		1 */
		printf("frequencyTraceable:        %s\n",	(ptp_state->frequencyTraceable ? "1" : "0"));
		/* grandmasterPriority1:	128 */
		printf("grandmasterPriority1:      %u\n",	ptp_state->grandmasterPriority1);
		/* gmClockClass:		6 */
		printf("gmClockClass:              %u\n",	ptp_state->gmClockClass);
		/* gmClockAccuracy:		0x21 */
		printf("gmClockAccuracy:           0x%x\n",	ptp_state->gmClockAccuracy);
		/* grandmasterPriority2:	128 */
		printf("grandmasterPriority2:      %u\n",	ptp_state->grandmasterPriority2);
		/* gmOffsetScaledLogVariance:	0x34fb */
		printf("gmOffsetScaledLogVariance: 0x%x\n",	ptp_state->gmOffsetScaledLogVariance);

		/* ptp_time:			Thu Sep  1 12:58:19 2022 */
		printf("ptp_time:                  %s\n",	ptp_state->ptp_time);
		/* system_time:			Thu Sep  1 12:58:19 2022 */
		printf("system_time:               %s\n",	ptp_state->sys_time);
	} else {
		/* gmPresent:			false */
		printf("gmPresent:                 false\n");
		/* ptp_stable:			Yes/No/Recovered */
		printf("ptp_stable:                %s\n",	get_stability_string(ptp_state->ptp_stability));
		/* ptp_time:			Thu Sep  1 12:58:19 2022 */
		printf("ptp_time:                  %s\n", ptp_state->ptp_time);
		/* system_time:			Thu Sep  1 12:58:19 2022 */
		printf("system_time:               %s\n", 	ptp_state->sys_time);
	}

	if (ptp_state->error_count > 0) {
		printf("error_count:               %u\n", ptp_state->error_count);
		printf("last_err_time:             %s\n", ptp_state->last_error_time);
	} else
		printf("\n\n");

	/* Maintain the same output length for easy screen formatting */
	if (!ptp_state->gmPresent) {
		for(i = 0; i < 12 ; i++)
			printf("\n");
	}

	/* Flush the output to avoid caching */
	fflush(stdout);

	return DOCA_SUCCESS;
}
