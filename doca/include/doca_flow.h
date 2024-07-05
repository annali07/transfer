/*
 * Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
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
 * @file doca_flow.h
 * @page doca flow
 * @defgroup Flow flow
 * DOCA HW offload flow library. For more details please refer to the user guide
 * on DOCA devzone.
 *
 * @{
 */

#ifndef DOCA_FLOW_H_
#define DOCA_FLOW_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <doca_compat.h>
#include <doca_error.h>

#include <doca_flow_net.h>
#include <doca_flow_crypto.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief doca flow port struct
 */
struct doca_flow_port;

/**
 * @brief doca flow pipeline struct
 */
struct doca_flow_pipe;

/**
 * @brief doca flow pipeline entry struct
 */
struct doca_flow_pipe_entry;

/**
 * @brief doca flow target struct
 */
struct doca_flow_target;

/**
 * @brief doca flow parser struct
 */
struct doca_flow_parser;

/**
 * @brief Shared resource supported types
 */
enum doca_flow_shared_resource_type {
	DOCA_FLOW_SHARED_RESOURCE_METER,
	/**< Shared meter type */
	DOCA_FLOW_SHARED_RESOURCE_COUNT,
	/**< Shared counter type */
	DOCA_FLOW_SHARED_RESOURCE_RSS,
	/**< Shared rss type */
	DOCA_FLOW_SHARED_RESOURCE_CRYPTO,
	/**< Shared crypto action type */
	DOCA_FLOW_SHARED_RESOURCE_MIRROR,
	/**< Shared mirror type */
	DOCA_FLOW_SHARED_RESOURCE_MAX,
	/**< Shared max supported types */
};

/**
 * @brief doca flow flags type
 */
enum doca_flow_flags_type {
	DOCA_FLOW_NO_WAIT = 0,
	/**< entry will not be buffered */
	DOCA_FLOW_WAIT_FOR_BATCH = (1 << 0),
	/**< entry will be buffered */
};

/**
 * @brief doca flow resource quota
 */
struct doca_flow_resources {
	uint32_t nb_counters;
	/**< Number of counters to configure */
	uint32_t nb_meters;
	/**< Number of traffic meters to configure */
};

/**
 * @brief doca flow entry operation
 */
enum doca_flow_entry_op {
	DOCA_FLOW_ENTRY_OP_ADD,
	/**< Add entry */
	DOCA_FLOW_ENTRY_OP_DEL,
	/**< Delete entry */
	DOCA_FLOW_ENTRY_OP_UPD,
	/**< Update entry */
	DOCA_FLOW_ENTRY_OP_AGED,
	/**< Aged entry */
};

/**
 * @brief doca flow entry status
 */
enum doca_flow_entry_status {
	DOCA_FLOW_ENTRY_STATUS_IN_PROCESS,
	/* The operation is in progress. */
	DOCA_FLOW_ENTRY_STATUS_SUCCESS,
	/* The operation was completed successfully. */
	DOCA_FLOW_ENTRY_STATUS_ERROR,
	/* The operation failed. */
};

/**
 * @brief doca flow entry process callback
 */
typedef void (*doca_flow_entry_process_cb)(struct doca_flow_pipe_entry *entry,
	uint16_t pipe_queue, enum doca_flow_entry_status status,
	enum doca_flow_entry_op op, void *user_ctx);

/**
 * @brief doca flow shared resource unbind callback
 */
typedef void (*doca_flow_shared_resource_unbind_cb)(enum doca_flow_shared_resource_type,
						    uint32_t shared_resource_id,
						    void *bindable_obj);

/**
 * @brief doca flow configuration flags
 */
enum doca_flow_cfg_flags {
	DOCA_FLOW_CFG_PIPE_MISS_MON = (1 << 0),
	 /**< Counter pipe miss flow and query with doca_flow_query_pipe_miss() */
};

/**
 * @brief doca flow global configuration
 */
struct doca_flow_cfg {
	uint64_t flags;
	/**< configuraton flags */
	uint16_t queues;
	/**< queue id for each offload thread */
	struct doca_flow_resources resource;
	/**< resource quota */
	uint8_t nr_acl_collisions;
	/**< number of pre-configured collisions for the acl module, default to 3, max to 8 */
	const char *mode_args;
	/**< set doca flow architecture mode switch, vnf */
	uint32_t nr_shared_resources[DOCA_FLOW_SHARED_RESOURCE_MAX];
	/**< total shared resource per type */
	uint32_t queue_depth;
	/**< Number of pre-configured queue_size, default to 128 */
	doca_flow_entry_process_cb cb;
	/**< callback for entry create/destroy */
	doca_flow_shared_resource_unbind_cb unbind_cb;
	/**< callback for unbinding of a shared resource */
	const uint8_t *rss_key;
	/**< RSS optional hash key. */
	uint32_t rss_key_len;
	/**< RSS hash key length in bytes. */
};

/**
 * @brief doca flow port type
 */
enum doca_flow_port_type {
	DOCA_FLOW_PORT_DPDK_BY_ID,
	/**< dpdk port by mapping id */
};

/**
 * @brief doca flow pipe type
 */
enum doca_flow_pipe_type {
	DOCA_FLOW_PIPE_BASIC,
	/**< Flow pipe */
	DOCA_FLOW_PIPE_CONTROL,
	/**< Control pipe */
	DOCA_FLOW_PIPE_LPM,
	/**< longest prefix match (LPM) pipe */
	DOCA_FLOW_PIPE_CT,
	/**< Connection Tracking pipe */
	DOCA_FLOW_PIPE_ACL,
	/**< ACL pipe */
	DOCA_FLOW_PIPE_ORDERED_LIST,
	/**< Ordered list pipe */
	DOCA_FLOW_PIPE_HASH,
	/**< Hash pipe */
};

/**
 * @brief doca flow pipe domain
 */
enum doca_flow_pipe_domain {
	DOCA_FLOW_PIPE_DOMAIN_DEFAULT = 0,
	/**< Default pipe domain for actions on ingress traffic */
	DOCA_FLOW_PIPE_DOMAIN_SECURE_INGRESS,
	/**< Pipe domain for secure actions on ingress traffic */
	DOCA_FLOW_PIPE_DOMAIN_EGRESS,
	/**< Pipe domain for actions on egress traffic */
	DOCA_FLOW_PIPE_DOMAIN_SECURE_EGRESS,
	/**< Pipe domain for actions on egress traffic */
};

/**
 * @brief doca flow port configuration
 */
struct doca_flow_port_cfg {
	uint16_t port_id;
	/**< dpdk port id */
	enum doca_flow_port_type type;
	/**< mapping type of port */
	const char *devargs;
	/**< specific per port type cfg */
	uint16_t priv_data_size;
	/**< user private data */
	void *dev;
	/**< port's dev */
};

/**
 * Max meta data size in bytes.
 */
#define DOCA_FLOW_META_MAX 20

/**
 * External meta data size in bytes.
 */
#define DOCA_FLOW_META_EXT 12

/**< @brief meter mark color */
enum __attribute__ ((__packed__)) doca_flow_meter_color {
	DOCA_FLOW_METER_COLOR_GREEN = 0,
	/**< Meter marking packet color as green */
	DOCA_FLOW_METER_COLOR_YELLOW,
	/**< Meter marking packet color as yellow */
	DOCA_FLOW_METER_COLOR_RED,
	/**< Meter marking packet color as red */
};

/**
 * @brief doca flow meta data
 *
 * Meta data known as scratch data can be used to match or modify within pipes.
 * Meta data can be set with value in previous pipes and match in later pipes.
 * User can customize meta data structure as long as overall size doens't exceed limit.
 * To match meta data, mask must be specified when creating pipe.
 * Struct must be aligned to 32 bits.
 * No initial value for Meta data, must match after setting value.
 */
struct doca_flow_meta {
	uint32_t pkt_meta; /**< Shared with application via packet. */
	uint32_t u32[DOCA_FLOW_META_MAX / 4 - 1]; /**< Programmable user data. */
	uint32_t port_meta; /**< Programmable source vport. */
	uint32_t mark; /**< Mark id. */
	/**
	 * Matches a random value.
	 * This value is not based on the packet data/headers.
	 * Application shouldn't assume that this value is kept during the packet lifetime.
	 */
	uint16_t random;
	uint8_t ipsec_syndrome; /**< IPsec decrypt/authentication syndrome. */
	enum doca_flow_meter_color meter_color; /**< Meter colors: Green, Yellow, Red. */
};

/**
 * @brief doca flow match flags
 */
enum doca_flow_match_tcp_flags {
	DOCA_FLOW_MATCH_TCP_FLAG_FIN = (1 << 0),
	/**< match tcp packet with Fin flag */
	DOCA_FLOW_MATCH_TCP_FLAG_SYN = (1 << 1),
	/**< match tcp packet with Syn flag */
	DOCA_FLOW_MATCH_TCP_FLAG_RST = (1 << 2),
	/**< match tcp packet with Rst flag */
	DOCA_FLOW_MATCH_TCP_FLAG_PSH = (1 << 3),
	/**< match tcp packet with Psh flag */
	DOCA_FLOW_MATCH_TCP_FLAG_ACK = (1 << 4),
	/**< match tcp packet with Ack flag */
	DOCA_FLOW_MATCH_TCP_FLAG_URG = (1 << 5),
	/**< match tcp packet with Urg flag */
	DOCA_FLOW_MATCH_TCP_FLAG_ECE = (1 << 6),
	/**< match tcp packet with Urg flag */
	DOCA_FLOW_MATCH_TCP_FLAG_CWR = (1 << 7),
	/**< match tcp packet with Urg flag */
};

/**
 * Max number of vlan headers.
 */
#define DOCA_FLOW_VLAN_MAX 2

/**
 * @brief doca flow l2 valid headers
 */
enum doca_flow_l2_valid_header {
	DOCA_FLOW_L2_VALID_HEADER_VLAN_0 = (1 << 0),
	/**< first vlan */
	DOCA_FLOW_L2_VALID_HEADER_VLAN_1 = (1 << 1),
	/**< second vlan */
};

/**
 * @brief doca flow packet format
 */
struct doca_flow_header_format {
	struct doca_flow_header_eth eth;
	/**< ether head */
	uint16_t l2_valid_headers;
	/**< indicate which headers are valid */
	struct doca_flow_header_eth_vlan eth_vlan[DOCA_FLOW_VLAN_MAX];
	/**< vlan header array*/
	enum doca_flow_l3_type l3_type;
	/**< layer 3 protocol type */
	union {
		struct doca_flow_header_ip4 ip4;
		/**< ipv4 head */
		struct doca_flow_header_ip6 ip6;
		/**< ipv6 head */
	};
	enum doca_flow_l4_type_ext l4_type_ext;
	/**< l4 layer extend type */
	union {
		struct doca_flow_header_icmp icmp;
		/**< icmp header */
		struct doca_flow_header_udp udp;
		/**< udp header */
		struct doca_flow_header_tcp tcp;
		/**< tcp header */
	};
};

/**
 * @brief doca flow matcher information
 */
struct doca_flow_match {
	uint32_t flags;
	/**< match items which are no value */
	struct doca_flow_meta meta;
	/**< Programmable meta data. */
	struct doca_flow_header_format outer;
	/**< outer layer header format */
	struct doca_flow_tun tun;
	/**< tunnel info */
	struct doca_flow_header_format inner;
	/**< inner layer header format */
};

/**
 * @brief doca flow encap data information
 */
struct doca_flow_encap_action {
	struct doca_flow_header_format outer;
	/**< outer header format */
	struct doca_flow_tun tun;
	/**< tunnel info */
};

/**
 * @brief doca flow push action type
 */
enum doca_flow_push_action_type {
	DOCA_FLOW_PUSH_ACTION_VLAN,
};

/**
 * @brief doca flow push data information
 */
struct doca_flow_push_action {
	enum doca_flow_push_action_type type;
	/**< header type to push */
	union {
		struct doca_flow_header_eth_vlan vlan;
	};
};

/**
 * @brief doca flow actions information
 */
struct doca_flow_actions {
	uint8_t action_idx;
	/**< index according to place provided on creation */
	uint32_t flags;
	/**< action flags */
	bool decap;
	/**< when true, will do decap */
	bool pop;
	/**< when true, pop header */
	struct doca_flow_meta meta;
	/**< modify meta data, pipe action as mask */
	struct doca_flow_header_format outer;
	/**< modify outer headers */
	struct doca_flow_tun tun;
	/**< modify tunnel headers*/
	bool has_encap;
	/**< when true, will do encap */
	struct doca_flow_encap_action encap;
	/**< encap data information */
	bool has_push;
	/**< when true, push header */
	struct doca_flow_push_action push;
	/**< push header data information */
	struct {
		enum doca_flow_crypto_protocol_type proto_type;
		/**< Crypto shared action type */
		uint32_t crypto_id;
		/**< Crypto shared action id */
	} security;
	/**< security shared action */
};

/**
 * @brief doca flow target type
 */
enum doca_flow_target_type {
	DOCA_FLOW_TARGET_KERNEL,
};

/**
 * @brief forwarding action type
 */
enum doca_flow_fwd_type {
	DOCA_FLOW_FWD_NONE = 0,
	/**< No forward action be set */
	DOCA_FLOW_FWD_RSS,
	/**< Forwards packets to rss */
	DOCA_FLOW_FWD_PORT,
	/**< Forwards packets to one port */
	DOCA_FLOW_FWD_PIPE,
	/**< Forwards packets to another pipe */
	DOCA_FLOW_FWD_DROP,
	/**< Drops packets */
	DOCA_FLOW_FWD_TARGET,
	/**< Forwards packets to target */
	DOCA_FLOW_FWD_ORDERED_LIST_PIPE,
	/**< Forwards packet to a specific entry in an ordered list pipe. */
};

/**
 * @brief rss offload types
 */
enum doca_rss_type {
	DOCA_FLOW_RSS_IPV4 = (1 << 0),
	/**< rss by ipv4 head */
	DOCA_FLOW_RSS_IPV6 = (1 << 1),
	/**< rss by ipv6 head */
	DOCA_FLOW_RSS_UDP = (1 << 2),
	/**< rss by udp head */
	DOCA_FLOW_RSS_TCP = (1 << 3),
	/**< rss by tcp head */
};

/**
 * @brief rss hash function type
 */
enum doca_flow_rss_hash_function {
	DOCA_FLOW_RSS_HASH_FUNCTION_TOEPLITZ,		/**< Toeplitz */
	DOCA_FLOW_RSS_HASH_FUNCTION_SYMMETRIC_TOEPLITZ, /**< Toeplitz with sorted source and destination */
};

/**
 * @brief forwarding configuration
 */
struct doca_flow_fwd {
	enum doca_flow_fwd_type type;
	/**< indicate the forwarding type */
	union {
		struct {
			uint32_t rss_outer_flags;
			/**< rss offload outer types */
			uint32_t rss_inner_flags;
			/**< rss offload outer types */
			uint16_t *rss_queues;
			/**< rss queues array */
			int num_of_queues;
			/**< number of queues */
			uint32_t shared_rss_id;
			/**< shared rss id, only for pipe's fwd is NULL */
			enum doca_flow_rss_hash_function rss_hash_func;
			/**< hash function */
		};
		/**< rss configuration information */
		struct {
			uint16_t port_id;
			/**< destination port id */
		};
		/**< port configuration information */
		struct {
			struct doca_flow_pipe *next_pipe;
			/**< next pipe pointer */
		};
		/**< next pipe configuration information */
		struct {
			/** Ordered list pipe to select an entry from. */
			struct doca_flow_pipe *pipe;
			/** Index of the ordered list pipe entry. */
			uint32_t idx;
		} ordered_list_pipe;
		/**< next ordered list pipe configuration */
		struct {
			struct doca_flow_target *target;
			/**< pointer to target handler */
		};
		/**< target configuration information */
	};
};


/**
 * @brief doca flow rss resource configuration
 */
struct doca_flow_resource_rss_cfg {
	uint32_t outer_flags;
	/**< rss offload outer types */
	uint32_t inner_flags;
	/**< rss offload inner types */
	uint16_t *queues_array;
	/**< rss queues array */
	int nr_queues;
	/**< number of queues */
	enum doca_flow_rss_hash_function rss_hash_func;
	/**< hash function */
};

/**
 * @brief Traffic meter algorithms
 */
enum doca_flow_meter_algorithm_type {
	DOCA_FLOW_METER_ALGORITHM_TYPE_RFC2697,
	/**< Single Rate Three Color Marker - IETF RFC 2697. */
	DOCA_FLOW_METER_ALGORITHM_TYPE_RFC2698,
	/**< Two Rate Three Color Marker - IETF RFC 2698. */
	DOCA_FLOW_METER_ALGORITHM_TYPE_RFC4115,
	/**< Two Rate Three Color Marker - IETF RFC 4115. */
};

/**
 * @brief Traffic meter limit type: per bytes or per packets for all
 * meter parameters: cir, cbs, eir, ebs.
 */
enum doca_flow_meter_limit_type {
	DOCA_FLOW_METER_LIMIT_TYPE_BYTES = 0,
	/**< Meter parameters per bytes */
	DOCA_FLOW_METER_LIMIT_TYPE_PACKETS,
	/**< Meter parameters packets */
};

/**
 * @brief Traffic meter init color mode when creating a pipe or entry: blind
 * (fixed as green) or aware (configurable value).
 */
enum doca_flow_meter_color_mode {
	DOCA_FLOW_METER_COLOR_MODE_BLIND = 0,
	/**< Meter action init color is green. */
	DOCA_FLOW_METER_COLOR_MODE_AWARE,
	/**< Meter action init color is configured. */
};

/**
 * @brief doca flow meter resource configuration
 */
struct doca_flow_resource_meter_cfg {
	enum doca_flow_meter_limit_type limit_type;
	/**< Meter rate limit type: bytes / packets per second */
	enum doca_flow_meter_color_mode color_mode;
	/**< Meter color mode: blind / aware */
	enum doca_flow_meter_algorithm_type alg;
	/**< Meter algorithm by RFCs */
	uint64_t cir;
	/**< Committed Information Rate (bytes or packets per second). */
	uint64_t cbs;
	/**< Committed Burst Size (bytes or packets). */
	union {
		struct {
			uint64_t ebs;
			/** Excess Burst Size (EBS) (bytes or packets). */
		} rfc2697;
		struct {
			uint64_t pir;
			/**< Peak Information Rate (bytes or packets per seconds). */
			uint64_t pbs;
			/**< Peak Burst Size (bytes or packets). */
		} rfc2698;
		struct {
			uint64_t eir;
			/**< Excess Information Rate (bytes or packets per seconds). */
			uint64_t ebs;
			/**< Excess Burst Size (EBS) (bytes or packets). */
		} rfc4115;
	};
};

/**
 * @brief doca flow crypto resource configuration
 */
struct doca_flow_resource_crypto_cfg {
	enum doca_flow_crypto_protocol_type proto_type;
	/**< packet reformat action */
	enum doca_flow_crypto_action_type action_type;
	/**< crypto action */
	enum doca_flow_crypto_reformat_type reformat_type;
	/**< packet reformat action */
	enum doca_flow_crypto_net_type net_type;
	/**< packet network mode type */
	enum doca_flow_crypto_header_type header_type;
	/**< packet header type */
	uint16_t reformat_data_sz;
	/**< reformat header length in bytes */
	uint8_t reformat_data[DOCA_FLOW_CRYPTO_REFORMAT_LEN_MAX];
	/**< reformat header buffer */
	enum doca_flow_crypto_icv_size reformat_icv_sz;
	/**< Intergtity Check Value in reformat action */
	union {
		struct {
			uint16_t key_sz;
			/**< key size in bytes */
			uint8_t key[DOCA_FLOW_CRYPTO_KEY_LEN_MAX];
			/**< Crypto key buffer */
		};
		void *security_ctx;
		/**< crypto object handle */
	};
	struct doca_flow_fwd fwd;
	/**< Crypto action continuation */
};

/**
 * @brief doca flow mirror target
 */
struct doca_flow_mirror_target {
	bool has_encap;
	/**< Encap mirrored packets. */
	struct doca_flow_encap_action encap;
	/**< Encap data. */
	struct doca_flow_fwd fwd;
	/**< Mirror target, must be filled. */
};

/**
 * @brief doca flow mirror resource configuration
 */
struct doca_flow_resource_mirror_cfg {
	int nr_targets;
	/**< Mirror target number. */
	struct doca_flow_mirror_target *target;
	/**< Mirror target pointer. */
	struct doca_flow_fwd fwd;
	/**< Original packet dst, can be filled optional. */
};

/**
 * @brief doca flow shared resource configuration
 */
struct doca_flow_shared_resource_cfg {
	enum doca_flow_pipe_domain domain;
	/**< Shared resource steering domain */
	union {
		struct doca_flow_resource_meter_cfg meter_cfg;
		struct doca_flow_resource_rss_cfg rss_cfg;
		struct doca_flow_resource_crypto_cfg crypto_cfg;
		struct doca_flow_resource_mirror_cfg mirror_cfg;
	};
};

/**
 * @brief doca monitor action flags
 */
enum {
	DOCA_FLOW_MONITOR_NONE = 0,
	/**< No monitor action be set */
	DOCA_FLOW_MONITOR_METER = (1 << 1),
	/**< set monitor with meter action */
	DOCA_FLOW_MONITOR_COUNT = (1 << 2),
	/**< set monitor with counter action */
	DOCA_FLOW_MONITOR_AGING = (1 << 3),
	/**< set monitor with aging action */
	DOCA_FLOW_MONITOR_MIRROR = (1 << 4),
	/**< set monitor with mirror action */
};

/**
 * @brief doca monitor action configuration
 */
struct doca_flow_monitor {
	uint8_t flags;
	/**< indicate which actions be included */
	struct {
		enum doca_flow_meter_limit_type limit_type;
		/**< Meter rate limit type: bytes / packets per second */
		uint64_t cir;
		/**< Committed Information Rate (bytes/second). */
		uint64_t cbs;
		/**< Committed Burst Size (bytes). */
	};
	/**< meter action configuration */
	struct {
		uint32_t shared_meter_id;
		/**< shared meter id */
		enum doca_flow_meter_color meter_init_color;
		/**< meter initial color */
	};
	uint32_t shared_counter_id;
	/**< shared counter id */
	uint32_t shared_mirror_id;
	/**< shared mirror id. */
	uint32_t aging_sec;
	/**< aging time in seconds.*/
};

/**
 * @brief action type enumeration
 */
enum doca_flow_action_type {
	DOCA_FLOW_ACTION_AUTO = 0, /* Derived from pipe actions. */
	DOCA_FLOW_ACTION_ADD, /* Add field value from pipe actions or flow entry. */
	DOCA_FLOW_ACTION_COPY, /* Copy field to another field. */
	DOCA_FLOW_ACTION_MAX, /* End of action type list. */
};

/**
 * @brief Action descriptor field
 *
 * Field based on a string that is composed out of struct members separated by a dot.
 *
 * The 1st segment determines the field location in packet "outer", "inner", "tunnel".
 * The 2nd segment determines the protocol.
 * The 3rd segment determines the field.
 *
 * E.g.
 * "outer.eth.src_mac"
 * "tunnel.gre.protocol"
 * "inner.ipv4.next_proto"
 */
struct doca_flow_action_desc_field {
	const char *field_string;
	/**< Field selection by string. */
	uint32_t bit_offset;
	/**< Field bit offset. */
};

/**
 * @brief action description
 */
struct doca_flow_action_desc {
	enum doca_flow_action_type type; /**< type */
	union {
		struct {
			struct doca_flow_action_desc_field src; /* Source info to copy from. */
			struct doca_flow_action_desc_field dst; /* Or destination info to copy to. */
			uint32_t width; /* Bit width to copy */
		} copy;
		struct {
			struct doca_flow_action_desc_field dst; /* destination info. */
			uint32_t width; /* Bit width to add */
		} add;
	};
};

/**
 * @brief action descriptor array
 */
struct doca_flow_action_descs {
	uint8_t nb_action_desc;
	/**< maximum number of action descriptor array. */
	struct doca_flow_action_desc *desc_array;
	/**< action descriptor array pointer. */
};

/** Type of an ordered list element. */
enum doca_flow_ordered_list_element_type {
	/**
	 * Ordered list element is struct doca_flow_actions,
	 * the next element is struct doca_flow_action_descs
	 * or actions mask associated with the current element.
	 */
	DOCA_FLOW_ORDERED_LIST_ELEMENT_ACTIONS,
	/**
	 * Ordered list element is struct doca_flow_actions,
	 * the next element is struct doca_flow_action_descs
	 * associated with the current element.
	 */
	DOCA_FLOW_ORDERED_LIST_ELEMENT_ACTIONS_MASK,
	/**
	 * Ordered list element is struct doca_flow_action_descs.
	 * If the previous element type is ACTIONS, the current element is associated with it.
	 * Otherwise the current element is ordered w.r.t. the previous one.
	 */
	DOCA_FLOW_ORDERED_LIST_ELEMENT_ACTION_DESCS,
	/**
	 * Ordered list element is struct doca_flow_monitor.
	 */
	DOCA_FLOW_ORDERED_LIST_ELEMENT_MONITOR,
};

/** Ordered list configuration. */
struct doca_flow_ordered_list {
	/**
	 * List index among the lists of the pipe.
	 * At pipe creation, it must match the list position in the array of lists.
	 * At entry insertion, it determines which list to use.
	 */
	uint32_t idx;
	/** Number of elements in the list. */
	uint32_t size;
	/** An array of DOCA flow structure pointers, depending on types. */
	const void **elements;
	/** Types of DOCA Flow structures each of the elements is pointing to. */
	enum doca_flow_ordered_list_element_type *types;
};

/**
 * @brief doca flow direction info
 */
enum doca_flow_direction_info {
	DOCA_FLOW_DIRECTION_BIDIRECTIONAL = 0,
	DOCA_FLOW_DIRECTION_NETWORK_TO_HOST,
	DOCA_FLOW_DIRECTION_HOST_TO_NETWORK,
};

/**
 * @brief pipe attributes
 */
struct doca_flow_pipe_attr {
	const char *name;
	/**< name for the pipeline */
	enum doca_flow_pipe_type type;
	/**< type of pipe. enum doca_flow_pipe_type */
	enum doca_flow_pipe_domain domain;
	/**< pipe steering domain. */
	bool is_root;
	/**< pipeline is root or not. If true it means the pipe is a root pipe executed on packet arrival. */
	uint32_t nb_flows;
	/**< maximum number of flow rules, default is 8k if not set */
	uint8_t nb_actions;
	/**< maximum number of doca flow action array, default is 1 if not set */
	uint8_t nb_ordered_lists;
	/**< number of ordered lists in the array, default 0, mutually exclusive with nb_actions */
	enum doca_flow_direction_info dir_info;
	/**< Optional direction hint for driver optimization, supported in switch mode only */
};

/**
 * @brief pipeline configuration
 */
struct doca_flow_pipe_cfg {
	struct doca_flow_pipe_attr attr;
	/**< attributes of pipe */
	struct doca_flow_port *port;
	/**< port for the pipeline */
	struct doca_flow_match *match;
	/**< matcher for the pipeline */
	struct doca_flow_match *match_mask;
	/**< match mask for the pipeline */
	struct doca_flow_actions **actions;
	/**< actions array for the pipeline */
	struct doca_flow_actions **actions_masks;
	/**< actions mask array for the pipeline */
	struct doca_flow_action_descs **action_descs;
	/**< action array descriptions */
	struct doca_flow_monitor *monitor;
	/**< monitor for the pipeline */
	struct doca_flow_ordered_list **ordered_lists;
	/**< array of ordered list types */
};

/**
 * @brief flow query result
 */
struct doca_flow_query {
	uint64_t total_bytes;
	/**< total bytes hit this flow */
	uint64_t total_pkts;
	/**< total packets hit this flow */
};

/**
 * @brief flow shared resources query result
 */
struct doca_flow_shared_resource_result {
	union {
		struct doca_flow_query counter;
	};
};

/**
 * @brief Geneve TLV option class mode
 */
enum doca_flow_parser_geneve_opt_mode {
	DOCA_FLOW_PARSER_GENEVE_OPT_MODE_IGNORE,
	/**< class is ignored. */
	DOCA_FLOW_PARSER_GENEVE_OPT_MODE_FIXED,
	/**< class is fixed (the class defines the option along with the type). */
	DOCA_FLOW_PARSER_GENEVE_OPT_MODE_MATCHABLE,
	/**< class is matching per flow. */
};

/**
 * @brief User configuration structure using to create parser for single GENEVE TLV option.
 */
struct doca_flow_parser_geneve_opt_cfg {
	enum doca_flow_parser_geneve_opt_mode match_on_class_mode;
	/**< Indicator about class field role in this option. */
	doca_be16_t option_class;
	/**< The class of the GENEVE TLV option. */
	uint8_t option_type;
	/**< The type of the GENEVE TLV option. */
	uint8_t option_len;
	/**< The length of the GENEVE TLV option data in DW granularity. */
	doca_be32_t data_mask[DOCA_FLOW_GENEVE_DATA_OPTION_LEN_MAX];
	/**< Data mask describing which DWs should be sampled. */
};

/**
 * @brief Initialize the doca flow.
 *
 * This is the global initialization function for doca flow. It
 * initializes all resources used by doca flow.
 *
 * Must be invoked first before any other function in this API.
 * this is a one time call, used for doca flow initialization and
 * global configurations.
 *
 * @param [in] cfg
 * Port configuration, see doca_flow_cfg for details.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported configuration.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_init(const struct doca_flow_cfg *cfg);

/**
 * @brief Destroy the doca flow.
 *
 * Release all the resources used by doca flow.
 *
 * Must be invoked at the end of the application, before it exits.
 */
__DOCA_EXPERIMENTAL
void
doca_flow_destroy(void);

/**
 * @brief Start a doca port.
 *
 * Start a port with the given configuration. Will create one port in
 * the doca flow layer, allocate all resources used by this port, and
 * create the default offload logic for traffic.
 *
 * @param [in] cfg
 * Port configuration, see doca_flow_cfg for details.
 * @param [out] port
 * Port handler on success.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported port type.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_port_start(const struct doca_flow_port_cfg *cfg,
		     struct doca_flow_port **port);

/**
 * @brief Stop a doca port.
 *
 * Stop the port, disable the traffic, destroy the doca port,
 * free all resources of the port.
 *
 * @param [in] port
 * Port struct.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_port_stop(struct doca_flow_port *port);

/**
 * @brief pair two doca flow ports.
 *
 * This API should be used to pair two doca ports. This pair should be the
 * same as the actual physical layer paired information. Those two pair
 * ports have no order, a port cannot be paired with itself.
 *
 * In this API, default behavior will be handled according to each modes.
 * In VNF mode, pair information will be translated to queue action to
 * redirect packets to it's pair port. In REMOTE_VNF mode,
 * default rules will be created to redirect packets between 2 pair ports.
 *
 * @param [in] port
 * Pointer to doca flow port.
 * @param [in] pair_port
 * Pointer to the pair port.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_NOT_SUPPORTED - not supported in the current run mode.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */

__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_port_pair(struct doca_flow_port *port, struct doca_flow_port *pair_port);

/**
 * @brief Get pointer of user private data.
 *
 * User can manage specific data structure in port structure.
 * The size of the data structure is given on port configuration.
 * See doca_flow_cfg for more details.
 *
 * @param [in] port
 * Port struct.
 * @return
 * Private data head pointer.
 */
__DOCA_EXPERIMENTAL
uint8_t*
doca_flow_port_priv_data(struct doca_flow_port *port);

/**
 * @brief Configure a single shared resource.
 *
 * This API can be used by bounded and unbounded resources.
 *
 * @param [in] type
 * Shared resource type.
 * @param [in] id
 * Shared resource id.
 * @param [in] cfg
 * Pointer to a shared resource configuration.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported shared resource type.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_shared_resource_cfg(enum doca_flow_shared_resource_type type, uint32_t id,
			      struct doca_flow_shared_resource_cfg *cfg);

/**
 * @brief Binds a bulk of shared resources to a bindable object.
 *
 * Binds a bulk of shared resources from the same type to a bindable object.
 * Currently the bindable objects are ports and pipes.
 *
 * @param [in] type
 * Shared resource type.
 * @param [in] res_array
 * Array of shared resource IDs.
 * @param [in] res_array_len
 * Shared resource IDs array length.
 * @param [in] bindable_obj
 * Pointer to an allowed bindable object, use NULL to bind globally.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported shared resource type.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_shared_resources_bind(enum doca_flow_shared_resource_type type, uint32_t *res_array,
				uint32_t res_array_len, void *bindable_obj);

/**
 * @brief Extract information about shared counter
 *
 * Query an array of shared objects of a specific type.
 *
 * @param [in] type
 * Shared object type.
 * @param [in] res_array
 * Array of shared objects IDs to query.
 * @param [in] query_results_array
 * Data array retrieved by the query.
 * @param [in] array_len
 * Number of objects and their query results in their arrays (same number).
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported shared resource type.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_shared_resources_query(enum doca_flow_shared_resource_type type,
				 uint32_t *res_array,
				 struct doca_flow_shared_resource_result *query_results_array,
				 uint32_t array_len);

/**
 * @brief Create one new pipe.
 *
 * Create new pipeline to match and offload specific packets, the pipe
 * configuration includes the following components:
 *
 *     match: Match one packet by inner or outer fields.
 *     match_mask: The mask for the matched items.
 *     actions: Includes the modify specific packets fields, Encap and
 *                  Decap actions.
 *     monitor: Includes Count, Age, and Meter actions.
 *     fwd: The destination of the matched action, include RSS, Hairpin,
 *             Port, and Drop actions.
 *
 * This API will create the pipe, but would not start the HW offload.
 *
 * @param [in] cfg
 * Pipe configuration.
 * @param [in] fwd
 * Fwd configuration for the pipe.
 * @param [in] fwd_miss
 * Fwd_miss configuration for the pipe. NULL for no fwd_miss.
 * When creating a pipe if there is a miss and fwd_miss configured,
 * packet steering should jump to it.
 * @param [out] pipe
 * Pipe handler on success.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported pipe type.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_create(const struct doca_flow_pipe_cfg *cfg,
		const struct doca_flow_fwd *fwd,
		const struct doca_flow_fwd *fwd_miss,
		struct doca_flow_pipe **pipe);

/**
 * @brief Add one new entry to a pipe.
 *
 * When a packet matches a single pipe, will start HW offload. The pipe only
 * defines which fields to match. When offloading, we need detailed information
 * from packets, or we need to set some specific actions that the pipe did not
 * define. The parameters include:
 *
 *    match: The packet detail fields according to the pipe definition.
 *    actions: The real actions according to the pipe definition.
 *    monitor: Defines the monitor actions if the pipe did not define it.
 *    fwd: Define the forward action if the pipe did not define it.
 *
 * This API will do the actual HW offload, with the information from the fields
 * of the input packets.
 *
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] pipe
 * Pointer to pipe.
 * @param [in] match
 * Pointer to match, indicate specific packet match information.
 * @param [in] actions
 * Pointer to modify actions, indicate specific modify information.
 * @param [in] monitor
 * Pointer to monitor actions.
 * @param [in] fwd
 * Pointer to fwd actions.
 * @param [in] flags
 * Flow entry will be pushed to hw immediately or not. enum doca_flow_flags_type.
 * @param [in] usr_ctx
 * Pointer to user context.
 * @param [out] entry
 * Pipe entry handler on success.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_add_entry(uint16_t pipe_queue,
			struct doca_flow_pipe *pipe,
			const struct doca_flow_match *match,
			const struct doca_flow_actions *actions,
			const struct doca_flow_monitor *monitor,
			const struct doca_flow_fwd *fwd,
			uint32_t flags,
			void *usr_ctx,
			struct doca_flow_pipe_entry **entry);

/**
 * @brief Update the pipe entry with new actions.
 *
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] pipe
 * Pointer to pipe.
 * @param [in] actions
 * Pointer to modify actions, indicate specific modify information.
 * @param [in] monitor
 * Pointer to monitor actions.
 * @param [in] fwd
 * Pointer to fwd actions.
 * @param [in] flags
 * Flow entry will be pushed to hw immediately or not. enum doca_flow_flags_type.
 * @param [in] entry
 * The pipe entry to be updated.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_update_entry(uint16_t pipe_queue,
			struct doca_flow_pipe *pipe,
			const struct doca_flow_actions *actions,
			const struct doca_flow_monitor *monitor,
			const struct doca_flow_fwd *fwd,
			const enum doca_flow_flags_type flags,
			struct doca_flow_pipe_entry *entry);

/**
 * @brief Add one new entry to a control pipe.
 *
 * Refer to doca_flow_pipe_add_entry.
 *
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] priority
 * Priority value.
 * @param [in] pipe
 * Pointer to pipe.
 * @param [in] match
 * Pointer to match, indicate specific packet match information.
 * @param [in] match_mask
 * Pointer to match mask information.
 * @param [in] actions
 * Pointer to modify actions, indicate specific modify information.
 * @param [in] actions_mask
 * Pointer to modify actions' mask, indicate specific modify information.
 * @param [in] action_descs
 * action descriptions
 * @param [in] monitor
 * Pointer to monitor actions.
 * @param [in] fwd
 * Pointer to fwd actions.
 * @param [in] usr_ctx
 * Pointer to user context.
 * @param [out] entry
 * Pipe entry handler on success.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_control_add_entry(uint16_t pipe_queue,
			uint32_t priority,
			struct doca_flow_pipe *pipe,
			const struct doca_flow_match *match,
			const struct doca_flow_match *match_mask,
			const struct doca_flow_actions *actions,
			const struct doca_flow_actions *actions_mask,
			const struct doca_flow_action_descs *action_descs,
			const struct doca_flow_monitor *monitor,
			const struct doca_flow_fwd *fwd,
			void *usr_ctx,
			struct doca_flow_pipe_entry **entry);

/**
 * @brief Add one new entry to a lpm pipe.
 *
 * This API will populate the lpm entries
 *
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] pipe
 * Pointer to pipe.
 * @param [in] match
 * Pointer to match, indicate specific packet match information.
 * @param [in] match_mask
 * Pointer to match mask information.
 * @param [in] actions
 * Pointer to modify actions, indicate specific modify information.
 * @param [in] monitor
 * Pointer to monitor actions.
 * @param [in] fwd
 * Pointer to fwd actions.
 * @param [in] flag
 * Flow entry will be pushed to hw immediately or not. enum doca_flow_flags_type.
 * @param [in] usr_ctx
 * Pointer to user context.
 * @param [out] entry
 * Pipe entry handler on success.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_lpm_add_entry(uint16_t pipe_queue,
			 struct doca_flow_pipe *pipe,
			 const struct doca_flow_match *match,
			 const struct doca_flow_match *match_mask,
			 const struct doca_flow_actions *actions,
			 const struct doca_flow_monitor *monitor,
			 const struct doca_flow_fwd *fwd,
			 const enum doca_flow_flags_type flag,
			 void *usr_ctx,
			 struct doca_flow_pipe_entry **entry);

/**
 * @brief Update the lpm pipe entry with new actions.
 *
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] pipe
 * Pointer to pipe.
 * @param [in] actions
 * Pointer to modify actions, indicate specific modify information.
 * @param [in] monitor
 * Pointer to monitor actions.
 * @param [in] fwd
 * Pointer to fwd actions.
 * @param [in] flags
 * Flow entry will be pushed to hw immediately or not. enum doca_flow_flags_type.
 * @param [in] entry
 * The pipe entry to be updated.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_lpm_update_entry(uint16_t pipe_queue,
			struct doca_flow_pipe *pipe,
			const struct doca_flow_actions *actions,
			const struct doca_flow_monitor *monitor,
			const struct doca_flow_fwd *fwd,
			const enum doca_flow_flags_type flags,
			struct doca_flow_pipe_entry *entry);

/**
 * Add an entry to the ordered list pipe.
 *
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] pipe
 * Pipe handle.
 * @param [in] idx
 * Unique entry index. It is the user's responsibility to ensure uniqueness.
 * @param [in] ordered_list
 * Ordered list with pointers to struct doca_flow_actions and struct doca_flow_monitor
 * at the same indices as they were at the pipe creation time.
 * If the configuration contained an element of struct doca_flow_action_descs,
 * the corresponding array element is ignored and can be NULL.
 * @param [in] fwd
 * Entry forward configuration.
 * @param [in] flags
 * Entry insertion flags.
 * @param [in] user_ctx
 * Opaque context for the completion callback.
 * @param[out] entry
 * The entry inserted.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_ordered_list_add_entry(uint16_t pipe_queue,
				      struct doca_flow_pipe *pipe,
				      uint32_t idx,
				      const struct doca_flow_ordered_list *ordered_list,
				      const struct doca_flow_fwd *fwd,
				      enum doca_flow_flags_type flags,
				      void *user_ctx,
				      struct doca_flow_pipe_entry **entry);
/**
 * @brief Add one new entry to a acl pipe.
 *
 * This API will populate the acl entries
 *
 * @param pipe_queue
 * Queue identifier.
 * @param pipe
 * Pointer to pipe.
 * @param match
 * Pointer to match, indicate specific packet match information.
 * @param match_mask
 * Pointer to match mask information.
 * @param priority
 * Priority value
 * @param fwd
 * Pointer to fwd actions.
 * @param flag
 * Flow entry will be pushed to hw immediately or not. enum doca_flow_flags_type.
 * @param usr_ctx
 * Pointer to user context.
 * @param[out] entry
 * The entry inserted.
 * @return
 * Pipe entry handler on success, NULL otherwise and error is set.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_acl_add_entry(uint16_t pipe_queue, struct doca_flow_pipe *pipe,
			     const struct doca_flow_match *match,
			     const struct doca_flow_match *match_mask,
			     const uint32_t priority, const struct doca_flow_fwd *fwd,
			     const enum doca_flow_flags_type flag, void *usr_ctx,
			     struct doca_flow_pipe_entry **entry);

/**
 * @brief Add one new entry to an hash pipe.
 *
 * Refer to doca_flow_pipe_add_entry.
 *
 * @param pipe_queue
 * Queue identifier.
 * @param pipe
 * Pointer to pipe.
 * @param entry_index
 * Static index in pipe for this entry.
 * @param actions
 * Pointer to modify actions, indicate specific modify information.
 * @param monitor
 * Pointer to monitor actions.
 * @param fwd
 * Pointer to forward actions.
 * @param flags
 * Flow entry will be pushed to HW immediately or not. enum doca_flow_flags_type.
 * @param usr_ctx
 * Pointer to user context.
 * @param [out] entry
 * Pipe entry handler on success.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_hash_add_entry(uint16_t pipe_queue,
			      struct doca_flow_pipe *pipe,
			      uint32_t entry_index,
			      const struct doca_flow_actions *actions,
			      const struct doca_flow_monitor *monitor,
			      const struct doca_flow_fwd *fwd,
			      const enum doca_flow_flags_type flags,
			      void *usr_ctx,
			      struct doca_flow_pipe_entry **entry);

/**
 * @brief Free one pipe entry.
 *
 * This API will free the pipe entry and cancel HW offload. The
 * Application receives the entry pointer upon creation and if can
 * call this function when there is no more need for this offload.
 * For example, if the entry aged, use this API to free it.
 *
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] flags
 * Flow entry will be removed from hw immediately or not. enum doca_flow_flags_type.
 * @param [in] entry
 * The pipe entry to be removed.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported pipe type.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_pipe_rm_entry(uint16_t pipe_queue,
			uint32_t flags,
			struct doca_flow_pipe_entry *entry);

/**
 * @brief Destroy one pipe
 *
 * Destroy the pipe, and the pipe entries that match this pipe.
 *
 * @param [in] pipe
 * Pointer to pipe.
 */
__DOCA_EXPERIMENTAL
void
doca_flow_pipe_destroy(struct doca_flow_pipe *pipe);

/**
 * @brief Flush pipes of one port
 *
 * Destroy all pipes and all pipe entries belonging to the port.
 *
 * @param [in] port
 * Pointer to doca flow port.
 */
__DOCA_EXPERIMENTAL
void
doca_flow_port_pipes_flush(struct doca_flow_port *port);

/**
 * @brief Dump pipes of one port
 *
 * Dump all pipes information belong to this port.
 *
 * @param [in] port
 * Pointer to doca flow port.
 * @param [in] f
 * The output file of the pipe information.
 */
__DOCA_EXPERIMENTAL
void
doca_flow_port_pipes_dump(struct doca_flow_port *port, FILE *f);

/**
 * @brief Dump pipe information
 *
 * @param [in] pipe
 * Pointer to doca flow pipe.
 * @param [in] f
 * The output file of the pipe information.
 */
__DOCA_EXPERIMENTAL
void
doca_flow_pipe_dump(struct doca_flow_pipe *pipe, FILE *f);

/**
 * @brief Extract information about specific entry
 *
 * Query the packet statistics about specific pipe entry
 *
 * @param [in] entry
 * The pipe entry to query.
 * @param [in] query_stats
 * Data retrieved by the query.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_query_entry(struct doca_flow_pipe_entry *entry,
		      struct doca_flow_query *query_stats);

/**
 * @brief Extract information about pipe miss entry
 *
 * Query the packet statistics about specific pipe miss entry
 *
 * @param [in] pipe
 * The pipe to query.
 * @param [in] query_stats
 * Data retrieved by the query.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_query_pipe_miss(struct doca_flow_pipe *pipe,
			  struct doca_flow_query *query_stats);

/**
 * @brief Handle aging of entries.
 *
 * Process aged entries, the user will get a notification in
 * the callback.
 *
 * Handling of aged entries can take too much time, so we split each cycle
 * to small chunks that are limited by some time quota.
 *
 * As long as the function doesn't return -1, more entries
 * are pending processing for this cycle.
 *
 * @param [in] port
 * Port to handle aging
 * @param [in] queue
 * Queue identifier.
 * @param [in] quota
 * Max time quota in micro seconds handle aging, 0: no limit.
 * @param [in] max_entries
 * Max entries for this function to handle aging, 0: no limit.
 * @return
 * > 0 the number of aged entries.
 * 0 no aged entries in current call.
 * -1 full cycle done.
 */
__DOCA_EXPERIMENTAL
int
doca_flow_aging_handle(struct doca_flow_port *port, uint16_t queue, uint64_t quota, uint64_t max_entries);

/**
 * @brief Process entries in queue.
 *
 * The application must invoke this function in order to complete
 * the flow rule offloading and to receive the flow rule operation status.
 *
 * @param [in] port
 * Port
 * @param [in] pipe_queue
 * Queue identifier.
 * @param [in] timeout
 * Max time in micro seconds for this function to process entries.
 * Process once if timeout is 0
 * @param [in] max_processed_entries
 * Flow entries number to process
 * If it is 0, it will proceed until timeout.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_DRIVER - driver error.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_entries_process(struct doca_flow_port *port, uint16_t pipe_queue, uint64_t timeout,
			  uint32_t max_processed_entries);

/**
 * @brief Get entry's status
 *
 * @param [in] entry
 * pipe entry
 * @return
 * entry's status
 */
__DOCA_EXPERIMENTAL
enum doca_flow_entry_status
doca_flow_pipe_entry_get_status(struct doca_flow_pipe_entry *entry);

/**
 * @brief Get doca flow switch port
 *
 * @param [in] port
 * The port for which to get the switch port. If NULL, get the first switch
 * port created.
 * The application could use this function to get the doca switch port, then
 * create pipes and pipe entries on this port.
 * @return
 * The parent switch port number or NULL if none found
 *
 */
__DOCA_EXPERIMENTAL
struct doca_flow_port *
doca_flow_port_switch_get(const struct doca_flow_port *port);

/**
 * @brief Prepare an MPLS label header in big-endian.
 *
 * @note: All input variables are in cpu-endian.
 *
 * @param [in] label
 * The label value - 20 bits.
 * @param [in] traffic_class
 * Traffic class - 3 bits.
 * @param [in] ttl
 * Time to live - 8 bits
 * @param [in] bottom_of_stack
 * Whether this MPLS is bottom of stack.
 * @param [out] mpls
 * Pointer to MPLS structure to fill.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_mpls_label_encode(uint32_t label, uint8_t traffic_class, uint8_t ttl, bool bottom_of_stack,
			    struct doca_flow_header_mpls *mpls);

/**
 * @brief Decode an MPLS label header.
 *
 * @note: All output variables are in cpu-endian.
 *
 * @param [in] mpls
 * Pointer to MPLS structure to decode.
 * @param [out] label
 * Pointer to fill MPLS label value.
 * @param [out] traffic_class
 * Pointer to fill MPLS traffic class value.
 * @param [out] ttl
 * Pointer to fill MPLS TTL value.
 * @param [out] bottom_of_stack
 * Pointer to fill whether this MPLS is bottom of stack.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_mpls_label_decode(const struct doca_flow_header_mpls *mpls, uint32_t *label,
			    uint8_t *traffic_class, uint8_t *ttl, bool *bottom_of_stack);

/**
 * @brief Creates GENEVE TLV parser for the selected port.
 *
 * This function must be called before creation of any pipe using GENEVE option.
 *
 * This API is port oriented, but the configuration is done once for all ports under the same
 * physical device. Each port should call this API before using GENEVE options, but it must use
 * the same options in the same order inside the list.
 *
 * Each physical device has 8 DWs for GENEVE TLV options. Each nonzero element in 'data_mask'
 * array consumes one DW, and choosing matchable mode for class consumes additional one.
 * Calling this API for second port under same physical device doesn't consume more DW, it uses
 * same configuration.
 *
 * @param [in] port
 * Pointer to doca flow port.
 * @param [in] tlv_list
 * A list of GENEVE TLV options to create parser for them.
 * @param [in] nb_options
 * The number of options in TLV list.
 * @param [out] parser
 * Parser handler on success.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NO_MEMORY - memory allocation failed.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported configuration.
 * - DOCA_ERROR_ALREADY_EXIST - physical device already has parser, by either same or another port.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_parser_geneve_opt_create(const struct doca_flow_port *port,
				   const struct doca_flow_parser_geneve_opt_cfg tlv_list[],
				   uint8_t nb_options, struct doca_flow_parser **parser);

/**
 * @brief Destroy GENEVE TLV parser.
 *
 * This function must be called after last use of GENEVE option and before port closing.
 *
 * @param [in] parser
 * Pointer to parser to be destroyed.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_IN_USE - one of options is in used by a pipe.
 * - DOCA_ERROR_DRIVER - there is no valid GENEVE TLV parser in this handle.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_parser_geneve_opt_destroy(struct doca_flow_parser *parser);

/**
 * @brief Get doca flow forward target.
 *
 * @param [in] type
 * Target type.
 * @param [out] target
 * Target handler on success
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_NOT_SUPPORTED - unsupported type.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_get_target(enum doca_flow_target_type type, struct doca_flow_target **target);

/**
 * @brief doca flow CT IPv4 match pattern
 */
struct doca_flow_ct_match4 {
	struct doca_flow_header_l4_port l4_port;
	/**< UDP or TCP source and destination port */
	doca_be32_t src_ip;
	/**< ip src address */
	doca_be32_t dst_ip;
	/**< ip dst address */
        uint32_t zone;
	/**< meta to match */
	uint8_t next_proto;
	/**< ip next protocol */
};

/**
 * @brief doca flow CT IPv6 match pattern
 */
struct doca_flow_ct_match6 {
	struct doca_flow_header_l4_port l4_port;
	/**< UDP or TCP source and destination port */
	doca_be32_t src_ip[4];
	/**< ip src address */
	doca_be32_t dst_ip[4];
	/**< ip dst address */
        uint32_t zone;
	/**< zone ID */
	uint8_t next_proto;
	/**< ip next protocol */
};

/**
 * @brief doca flow CT match pattern
 */
struct doca_flow_ct_match {
	union {
		struct doca_flow_ct_match4 ipv4;
		struct doca_flow_ct_match6 ipv6;
	};
};

/**
 * @brief doca flow CT entry operation flags
 */
enum doca_flow_ct_entry_flags {
	DOCA_FLOW_CT_ENTRY_FLAGS_NO_WAIT = (1 << 0),
	/**< entry will not be buffered, send to hardware immediately */
	DOCA_FLOW_CT_ENTRY_FLAGS_DIR_ORIGIN = (1 << 1),
	/**< apply to origin direction */
	DOCA_FLOW_CT_ENTRY_FLAGS_DIR_REPLY = (1 << 2),
	/**< apply to reply direction */
	DOCA_FLOW_CT_ENTRY_FLAGS_IPV6 = (1 << 3),
	/**< entry is IPv6, union in struct doca_flow_ct_match is ipv6 */
	DOCA_FLOW_CT_ENTRY_FLAGS_AGING = (1 << 4),
	/**< enable aging check */
	DOCA_FLOW_CT_ENTRY_FLAGS_COUNTER_ORIGIN = (1 << 5),
	/**< Apply counter to origin direction */
	DOCA_FLOW_CT_ENTRY_FLAGS_COUNTER_REPLY = (1 << 6),
	/**< Apply counter to reply direction */
	DOCA_FLOW_CT_ENTRY_FLAGS_COUNTER_SHARED = (1 << 7),
	/**< Counter is shared for both direction */
};

/**
 * @brief Add new entry to doca flow CT table.
 *
 * @param [in] port
 * port of CT pipe.
 * @param [in] queue
 * queue ID, offset from doca_flow.nb_queues.
 * @param [in] flags
 * operation flags, see DOCA_FLOW_CT_FLAGS_xxx.
 * @param [in] match_origin
 * match pattern in origin direction.
 * @param [in] match_reply
 * match pattern in reply direction, default to reverse of origin pattern.
 * @param [in] meta_origin
 * meta to set on origin direction
 * @param [in] meta_reply
 * meta to set on reply direction
 * @param [in] usr_ctx
 * user context data to associate to entry
 * @param [out] entry
 * pointer to save the new entry
 * @return
 * DOCA_SUCCESS - in case of success.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_ct_add_entry(struct doca_flow_port *port, uint16_t queue, uint32_t flags,
		       struct doca_flow_ct_match *match_origin, struct doca_flow_ct_match *match_reply,
		       uint32_t meta_origin, uint32_t meta_reply, void *usr_ctx, struct doca_flow_pipe_entry  **entry);

/**
 * @brief Update CT entry meta or couter.
 *
 * @param [in] port
 * port of CT pipe.
 * @param [in] queue
 * queue ID, offset from doca_flow.nb_queues.
 * @param [in] flags
 * operation flags, see DOCA_FLOW_CT_FLAGS_xxx.
 * @param [in] entry
 * The CT pipe entry to query.
 * @param [in] meta_origin
 * meta to set on origin direction
 * @param [in] meta_reply
 * meta to set on reply direction
 * @return
 * DOCA_SUCCESS - in case of success.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_ct_update_entry(struct doca_flow_port *port, uint16_t queue, uint32_t flags,
			  struct doca_flow_pipe_entry *entry, uint32_t meta_origin, uint32_t meta_reply);

/**
 * @brief remove CT entry.
 *
 * @param [in] port
 * port of CT pipe.
 * @param [in] queue
 * queue ID, offset from doca_flow.nb_queues.
 * @param [in] flags
 * operation flags, see DOCA_FLOW_CT_FLAGS_xxx.
 * @param [in] entry
 * The CT pipe entry to query.
 * @return
 * DOCA_SUCCESS - in case of success.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_ct_rm_entry(struct doca_flow_port *port, uint16_t queue, uint32_t flags, struct doca_flow_pipe_entry *entry);

/**
 * @brief Get CT entry match pattern.
 *
 * @param [in] port
 * port of CT pipe.
 * @param [in] queue
 * queue ID, offset from doca_flow.nb_queues.
 * @param [in] flags
 * operation flags, see DOCA_FLOW_CT_FLAGS_xxx.
 * @param [in] entry
 * CT entry.
 * @param [out] match_origin
 * Pointer to match pattern of origin direction
 * @param [out] match_reply
 * Pointer to match pattern of reply direction
 * @return
 * DOCA_SUCCESS - in case of success.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_ct_get_entry(struct doca_flow_port *port, uint16_t queue, uint32_t flags, struct doca_flow_pipe_entry *entry,
		       struct doca_flow_ct_match **match_origin, struct doca_flow_ct_match **match_reply);

/**
 * @brief Extract information about specific entry
 *
 * Query the packet statistics about specific CT pipe entry
 *
 * @param [in] port
 * port of CT pipe.
 * @param [in] queue
 * queue ID, offset from doca_flow.nb_queues.
 * @param [in] flags
 * operation flags, see DOCA_FLOW_CT_FLAGS_xxx.
 * @param [in] entry
 * The CT pipe entry to query.
 * @param [in] stats_origin
 * Data of origin direction retrieved by the query.
 * @param [in] stats_reply
 * Data of reply direction retrieved by the query.
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - received invalid input.
 * - DOCA_ERROR_UNKNOWN - otherwise.
 */
__DOCA_EXPERIMENTAL
doca_error_t
doca_flow_ct_query_entry(struct doca_flow_port *port, uint16_t queue, uint32_t flags,
			 struct doca_flow_pipe_entry *entry, struct doca_flow_query *stats_origin,
			 struct doca_flow_query *stats_reply);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif /* DOCA_FLOW_H_ */
