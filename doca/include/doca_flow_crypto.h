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
 * @file doca_flow_crypto.h
 * @page doca flow crypto
 * @defgroup FLOW_CRYPTO flow net define
 * DOCA HW offload flow cryptonet structure define. For more details please refer to
 * the user guide on DOCA devzone.
 *
 * @{
 */

#ifndef DOCA_FLOW_CRYPTO_H_
#define DOCA_FLOW_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief doca flow crypto operation protocol type
 */
enum doca_flow_crypto_protocol_type {
	DOCA_FLOW_CRYPTO_PROTOCOL_NONE = 0,
	/**< No security protocol engaged */
	DOCA_FLOW_CRYPTO_PROTOCOL_ESP,
	/**< IPsec ESP protocol action */
};

/**
 * @brief doca flow crypto operation action type
 */
enum doca_flow_crypto_action_type {
	DOCA_FLOW_CRYPTO_ACTION_NONE = 0,
	/**< No crypto action performed */
	DOCA_FLOW_CRYPTO_ACTION_ENCRYPT,
	/**< Perform encryption */
	DOCA_FLOW_CRYPTO_ACTION_DECRYPT,
	/**< Perform decryption/authentication */
};

/**
 * @brief doca flow crypto operation reformat type
 */
enum doca_flow_crypto_reformat_type {
	DOCA_FLOW_CRYPTO_REFORMAT_NONE = 0,
	/**< No reformat action performed */
	DOCA_FLOW_CRYPTO_REFORMAT_ENCAP,
	/**< Perform encapsulation action */
	DOCA_FLOW_CRYPTO_REFORMAT_DECAP,
	/**< Perform decapsulation action */
};

/**
 * @brief doca flow crypto operation network mode type
 */
enum doca_flow_crypto_net_type {
	DOCA_FLOW_CRYPTO_NET_NONE = 0,
	/**< No network header involved */
	DOCA_FLOW_CRYPTO_NET_TUNNEL,
	/**< Tunnel network header */
	DOCA_FLOW_CRYPTO_NET_TRANSPORT,
	/**< Tramsport network header */
};

/**
 * @brief doca flow crypto operation encapsulation header type
 */
enum doca_flow_crypto_header_type {
	DOCA_FLOW_CRYPTO_HEADER_NONE = 0,
	/**< No network header involved */
	DOCA_FLOW_CRYPTO_HEADER_IPV4,
	/**< IPv4 network header type */
	DOCA_FLOW_CRYPTO_HEADER_IPV6,
	/**< IPv6 network header type */
	DOCA_FLOW_CRYPTO_HEADER_IPV4_UDP,
	/**< IPv4 + UDP network header type */
	DOCA_FLOW_CRYPTO_HEADER_IPV6_UDP,
	/**< IPv6 + UDP network header type */
};

/**
 * @brief doca flow crypto protocol Integrity Check Value size
 */
enum doca_flow_crypto_icv_size {
	DOCA_FLOW_CRYPTO_ICV_DEFAULT = 0,
	/**< Use default ICV size for specified protocol. */
	DOCA_FLOW_CRYPTO_ICV_8B = 8,
	/**< ICV size is 8B */
	DOCA_FLOW_CRYPTO_ICV_12B = 12,
	/**< ICV size is 12B */
	DOCA_FLOW_CRYPTO_ICV_16B = 16,
	/**< ICV size is 16B */
};

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif /* DOCA_FLOW_CRYPTO_H_ */
