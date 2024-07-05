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

#include <stdlib.h>

#include <doca_log.h>
#include <doca_argp.h>

#include "rdma_common.h"

DOCA_LOG_REGISTER(RDMA_RECEIVE::MAIN);

/* Sample's Logic */
doca_error_t rdma_receive(struct rdma_config *cfg);

/*
 * Sample main function
 *
 * @argc [in]: command line arguments size
 * @argv [in]: array of command line arguments
 * @return: EXIT_SUCCESS on success and EXIT_FAILURE otherwise
 */
int
main(int argc, char **argv)
{
	struct rdma_config cfg;
	doca_error_t result;
	int exit_status = EXIT_FAILURE;

	/* Set the default configuration values (Example values) */
	strcpy(cfg.local_connection_desc_path, DEFAULT_LOCAL_CONNECTION_DESC_PATH);
	strcpy(cfg.remote_connection_desc_path, DEFAULT_REMOTE_CONNECTION_DESC_PATH);
	cfg.is_gid_index_set = false;

	/* No need for send_string in the receiver side */
	cfg.send_string[0] = '\0';

	/* Register a logger backend */
	result = doca_log_create_standard_backend();
	if (result != DOCA_SUCCESS)
		goto sample_exit;

	DOCA_LOG_INFO("Starting the sample");

	/* Initialize argparser */
	result = doca_argp_init("doca_rdma_receive", &cfg);
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("Failed to init ARGP resources: %s", doca_get_error_string(result));
		goto sample_exit;
	}

	/* Register RDMA params */
	result = register_rdma_params();
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("Failed to register sample parameters: %s", doca_get_error_string(result));
		goto argp_cleanup;
	}

	/* Start argparser */
	result = doca_argp_start(argc, argv);
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("Failed to parse sample input: %s", doca_get_error_string(result));
		goto argp_cleanup;
	}

	/* Start sample */
	result = rdma_receive(&cfg);
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("rdma_receive() failed: %s", doca_get_error_string(result));
		goto argp_cleanup;
	}

	exit_status = EXIT_SUCCESS;

argp_cleanup:
	doca_argp_destroy();
sample_exit:
	if (exit_status == EXIT_SUCCESS)
		DOCA_LOG_INFO("Sample finished successfully");
	else
		DOCA_LOG_INFO("Sample finished with errors");
	return exit_status;
}