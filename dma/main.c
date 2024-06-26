#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include <doca_dma.h>
#include <doca_error.h>
#include <doca_log.h>
#include <doca_mmap.h>
#include <doca_argp.h>
#include <doca_dev.h>
#include <doca_log.h>

#include "dma_common.h"

/* Shared variable to allow for a proper shutdown */
bool is_dma_done_on_dpu;

/*
 * Signal handler
 *
 * @signum [in]: Signal number to handle
 */
static void
signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		DOCA_LOG_INFO("Signal %d received, preparing to exit", signum);
		is_dma_done_on_dpu = true;
	}
}

/*
 * Saves export descriptor and buffer information into two separate files
 *
 * @export_desc [in]: Export descriptor to write into a file
 * @export_desc_len [in]: Export descriptor length
 * @src_buffer [in]: Source buffer
 * @src_buffer_len [in]: Source buffer length
 * @export_desc_file_path [in]: Export descriptor file path
 * @buffer_info_file_path [in]: Buffer information file path
 * @return: DOCA_SUCCESS on success and DOCA_ERROR otherwise
 */
static doca_error_t
save_config_info_to_files(const void *export_desc, size_t export_desc_len, const char *src_buffer, size_t src_buffer_len,
			  char *export_desc_file_path, char *buffer_info_file_path)
{
	FILE *fp;
	uint64_t buffer_addr = (uintptr_t)src_buffer;
	uint64_t buffer_len = (uint64_t)src_buffer_len;

	fp = fopen(export_desc_file_path, "wb");
	if (fp == NULL) {
		DOCA_LOG_ERR("Failed to create the DMA copy file");
		return DOCA_ERROR_IO_FAILED;
	}

	if (fwrite(export_desc, 1, export_desc_len, fp) != export_desc_len) {
		DOCA_LOG_ERR("Failed to write all data into the file");
		fclose(fp);
		return DOCA_ERROR_IO_FAILED;
	}

	fclose(fp);

	fp = fopen(buffer_info_file_path, "w");
	if (fp == NULL) {
		DOCA_LOG_ERR("Failed to create the DMA copy file");
		return DOCA_ERROR_IO_FAILED;
	}

	fprintf(fp, "%" PRIu64 "\n", buffer_addr);
	fprintf(fp, "%" PRIu64 "", buffer_len);

	fclose(fp);

	return DOCA_SUCCESS;
}

/*
 * Run DOCA DMA Host copy sample
 *
 * @pcie_addr [in]: Device PCI address
 * @src_buffer [in]: Source buffer to copy
 * @src_buffer_size [in]: Buffer size
 * @export_desc_file_path [in]: Export descriptor file path
 * @buffer_info_file_name [in]: Buffer info file path
 * @return: DOCA_SUCCESS on success and DOCA_ERROR otherwise
 */
doca_error_t
dma_copy_host(const char *pcie_addr, char *src_buffer, size_t src_buffer_size,
		     char *export_desc_file_path, char *buffer_info_file_name)
{
	struct program_core_objects state = {0};
	doca_error_t result;
	const void *export_desc;
	size_t export_desc_len;

	/* Signal the while loop to stop and destroy the memory map */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* Open the relevant DOCA device */
	result = open_doca_device_with_pci(pcie_addr, &dma_jobs_is_supported, &state.dev);
	if (result != DOCA_SUCCESS)
		return result;

	/* Init all DOCA core objects */
	result = host_init_core_objects(&state);
	if (result != DOCA_SUCCESS) {
		host_destroy_core_objects(&state);
		return result;
	}

	/* Allow exporting the mmap to DPU for read only operations */
	result = doca_mmap_set_permissions(state.src_mmap, DOCA_ACCESS_DPU_READ_ONLY);
	if (result != DOCA_SUCCESS) {
		host_destroy_core_objects(&state);
		return result;
	}

	/* Populate the memory map with the allocated memory */
	result = doca_mmap_set_memrange(state.src_mmap, src_buffer, src_buffer_size);
	if (result != DOCA_SUCCESS) {
		host_destroy_core_objects(&state);
		return result;
	}
	result = doca_mmap_start(state.src_mmap);
	if (result != DOCA_SUCCESS) {
		host_destroy_core_objects(&state);
		return result;
	}

	/* Export DOCA mmap to enable DMA on Host*/
	result = doca_mmap_export_dpu(state.src_mmap, state.dev, &export_desc, &export_desc_len);
	if (result != DOCA_SUCCESS) {
		host_destroy_core_objects(&state);
		return result;
	}

	DOCA_LOG_INFO("Please copy %s and %s to the DPU and run DMA Copy DPU sample before closing", export_desc_file_path, buffer_info_file_name);

	/* Saves the export desc and buffer info to files, it is the user responsibility to transfer them to the dpu */
	result = save_config_info_to_files(export_desc, export_desc_len, src_buffer, src_buffer_size,
					   export_desc_file_path, buffer_info_file_name);
	if (result != DOCA_SUCCESS) {
		host_destroy_core_objects(&state);
		return result;
	}

	/* Wait until DMA copy on the DPU is over */
	while (!is_dma_done_on_dpu)
		sleep(1);

	/* Destroy all relevant DOCA core objects */
	host_destroy_core_objects(&state);

	return result;
}





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
	struct dma_config dma_conf;
	char *src_buffer;
	size_t length;
	doca_error_t result;
	int exit_status = EXIT_FAILURE;

	
	/* Set the default configuration values (Example values) */
	strcpy(dma_conf.pci_address, "b1:00.0");
	strcpy(dma_conf.cpy_txt, "This is a sample piece of text");
	strcpy(dma_conf.export_desc_path, "/tmp/export_desc.txt");
	strcpy(dma_conf.buf_info_path, "/tmp/buffer_info.txt");

	/* Register a logger backend */
	result = doca_log_create_standard_backend();
	if (result != DOCA_SUCCESS)
		goto sample_exit;

	DOCA_LOG_INFO("Starting the sample");

	result = doca_argp_init("doca_dma_copy_host", &dma_conf);
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("Failed to init ARGP resources: %s", doca_get_error_string(result));
		goto sample_exit;
	}
	result = register_dma_params(true);
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("Failed to register DMA sample parameters: %s", doca_get_error_string(result));
		goto argp_cleanup;
	}
	result = doca_argp_start(argc, argv);
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("Failed to parse sample input: %s", doca_get_error_string(result));
		goto argp_cleanup;
	}
	
	length = strlen(dma_conf.cpy_txt) + 1;
	src_buffer = (char *)malloc(length);
	if (src_buffer == NULL) {
		DOCA_LOG_ERR("Source buffer allocation failed");
		goto argp_cleanup;
	}

	memcpy(src_buffer, dma_conf.cpy_txt, length);

	result = dma_copy_host(dma_conf.pci_address, src_buffer, length, dma_conf.export_desc_path, dma_conf.buf_info_path);
	if (result != DOCA_SUCCESS) {
		DOCA_LOG_ERR("dma_copy_host() encountered an error: %s", doca_get_error_string(result));
		goto src_buf_cleanup;
	}

	exit_status = EXIT_SUCCESS;

    src_buf_cleanup:
	if (src_buffer != NULL)
		free(src_buffer);
    argp_cleanup:
	doca_argp_destroy();
    sample_exit:
	if (exit_status == EXIT_SUCCESS)
		DOCA_LOG_INFO("Sample finished successfully");
	else
		DOCA_LOG_INFO("Sample finished with errors");
	return exit_status;
}
