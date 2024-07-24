#!/usr/bin/python3

#
# Copyright (c) 2022-2023 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
#
# This software product is a proprietary product of NVIDIA CORPORATION &
# AFFILIATES (the "Company") and all right, title, and interest in and to the
# software product, including all associated intellectual property rights, are
# and shall remain exclusively with the Company.
#
# This software product is governed by the End User License Agreement
# provided with the software product.
#

import sys
import click
import logging
import shlex
import concurrent.futures as futures
import grpc

import firefly_monitor_pb2 as gen_pbuf
import firefly_monitor_pb2_grpc as gen_grpc

CLIENT_NAME = 'Firefly-Monitor'
FULL_CLIENT_NAME = CLIENT_NAME + ' gRPC Client'

FIREFLY_MONITOR_VERSION = "1.2.0"

gRPC_PORT = gen_pbuf.eNetworkPort.k_DocaFirefly

stability_string_dict = {
	gen_pbuf.k_STATE_STABLE : "Yes",
	gen_pbuf.k_STATE_FAULTY : "No",
	gen_pbuf.k_STATE_RECOVERED : "Recovered",
}

logger = logging.getLogger(FULL_CLIENT_NAME)
ch = logging.StreamHandler()
ch.setFormatter(logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s'))
logger.addHandler(ch)
logger.setLevel(logging.INFO)

def print_monitor_record(record):
	"""Print the information from an incoming monitor record.

	Args:
		record (grpc): PTP monitor record
	"""
	print("\n")
	if record.gm_present:
		# gmIdentity:			EC:46:70:FF:FE:10:FE:B9 (ec4670.fffe.10feb9)
		print("gmIdentity:                %s" %		record.gm_identity)
		# portIdentity:			EC:46:70:FF:FE:10:FE:B9 (ec4670.fffe.10feb9-1)
		print("portIdentity:              %s" %		record.port_identity)
		# master_offset:		23
		print("master_offset (max):       %d" %		record.master_offset.max)
		print("master_offset (avg):       %d" %		record.master_offset.average)
		# gmPresent:			true
		print("gmPresent:                 true")
		# ptp_stable:			Yes/No/Recovered
		print("ptp_stable:                %s" %		stability_string_dict[record.ptp_stability])
		# currentUtcOffset:		37
		print("UtcOffset:                 %d" %		record.current_utc_offset)
		# timeTraceable:		1
		print("timeTraceable:             %s" %		("1" if record.time_traceable else "0"))
		# frequencyTraceable:		1
		print("frequencyTraceable:        %s" %		("1" if record.frequency_traceable else "0"))
		# grandmasterPriority1:		128
		print("grandmasterPriority1:      %u" %		record.grandmaster_priority1)
		# gmClockClass:			6
		print("gmClockClass:              %u" %		record.gm_clock_class)
		# gmClockAccuracy:		0x21
		print("gmClockAccuracy:           0x%x" %	record.gm_clock_accuracy)
		# grandmasterPriority2:		128
		print("grandmasterPriority2:      %u" %		record.grandmaster_priority2)
		# gmOffsetScaledLogVariance:	0x34fb
		print("gmOffsetScaledLogVariance: 0x%x" %	record.gm_offset_scaled_log_variance)

		# ptp_time:			Thu Sep  1 12:58:19 2022
		print("ptp_time:                  %s" %		record.ptp_time)
		# system_time:			Thu Sep  1 12:58:19 2022
		print("system_time:               %s" %		record.sys_time)
	else:
		# gmPresent:			false
		print("gmPresent:                 false")
		# ptp_stable:			Yes/No/Recovered
		print("ptp_stable:                %s" %		stability_string_dict[record.ptp_stability])
		# ptp_time:			Thu Sep  1 12:58:19 2022
		print("ptp_time:                  %s" %		record.ptp_time)
		# system_time:			Thu Sep  1 12:58:19 2022
		print("system_time:               %s" %		record.sys_time)

	if record.error_count > 0:
		print("error_count:               %u" % record.error_count)
		print("last_err_time:             %s" % record.last_error_time)
	else:
		print("\n")

	# Maintain the same output length for easy screen formatting
	if not record.gm_present:
		print("\n" * 11)

	# Flush the output to avoid caching
	sys.stdout.flush()


@click.group(help='DOCA Firefly Monitor gRPC Client', invoke_without_command=True)
@click.argument('server_address')
def main(server_address):
	if ':' not in server_address:
		server_address = f'{server_address}:{gRPC_PORT}'

	logger.info(f'Connecting to the {CLIENT_NAME} gRPC server on the DPU: {server_address}')
	channel = grpc.insecure_channel(server_address)

	# Print a banner when we start
	print("#########################################")
	print("## DOCA Firefly Monitor 2023 By NVIDIA ##")
	print("##           Version:  %5s           ##" % FIREFLY_MONITOR_VERSION)
	print("#########################################\n")

	try:
		stub = gen_grpc.FireflyMonitorStub(channel)
		monitor_events_stream = stub.Subscribe(gen_pbuf.SubscribeReq())
		# get push notification and print them
		for monitor_record in monitor_events_stream:
			print_monitor_record(monitor_record)
	except RuntimeError as e:
		logger.error('Failed to connect to the gRPC server on the DPU')

	logger.info('Finished Successfully')


if __name__ == '__main__':
	main()
