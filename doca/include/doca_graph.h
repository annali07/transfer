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
 * @file doca_graph.h
 * @page doca doca_graph
 * @defgroup DOCACore Core
 * @defgroup GRAPH DOCA Graph
 * @ingroup DOCACore
 *
 * DOCA graph facilitates submitting an ordered set of jobs and user callbacks.
 * A graph can contain nodes of the following types:
 * - Context node: A node that points to a context and contains a doca_job for that context.
 * -- A graph must contain at least one context node.
 * - User node: A node that points to a callback supplied by the user and contains a user defined doca_job.
 * - Graph node: A node that points to a graph instance and facilitates building a graph of graphs.
 *
 * Graph Instance
 * A graph creates a graph instance (or more)
 * Every node in the graph instance is set with corresponding data (job, callback, etc. depending on the type of the
 * node).
 * Node data can be set during runtime, but it is not recommended. Application should instead change the job content.
 *
 * Usage:
 * - Create a graph by adding nodes and setting dependencies.
 * -- Cyclic graph is not permitted.
 * - Add the graph to a work queue (or to multiple work queues).
 * - Create graph instance (or more).
 * - Set nodes data to every graph instance.
 * -- Setting node doca_event is optional. If set, it will contain the node result and user data (similar to job)
 * - Submit graph instances to the corresponding work queue
 * - Call progress retrieve when applicable.
 * -- event type will be DOCA_GRAPH when the graph ends.
 * -- doca_event.result.u64 shall be DOCA_SUCCESS if the graph progress was successful, any other doca_error_t if graph
 *    failed.
 *
 * Notes
 * - Any node failure shall fail the graph progress. However, the graph progress shall complete only when all in flight
 *   nodes are completed (new nodes shall not be submitted).
 * - A graph instance shall not fail if a context is overloaded (it will continue running once the context is free).
 *
 * Graph example (diamond graph):
 *
 *                         +-------------+
 *                         |    Node A   |
 *                         +-------------+
 *                                |
 *                +---------------+---------------+
 *                |                               |
 *        +-------------+                  +-------------+
 *        |    Node B   |                  |    Node C   |
 *        +-------------+                  +-------------+
 *                |                               |
 *                +---------------+---------------+
 *                                |
 *                         +-------------+
 *                         |    Node D   |
 *                         +-------------+
 *
 * Graph implementation example:
 * This example builds a graph with 2 nodes, creates an instance and submits it to a work queue.
 * node1 -> node2
 * The example is focused on the graph API. It does not include work queue, contexts creation etc. or error handling.
 *
 * Create the graph
 *	struct doca_graph *my_graph;
 *	doca_graph_create(&my_graph);
 *
 * Create the nodes
 *	struct doca_graph_node *node1, node2;
 *	doca_graph_ctx_node_create(my_graph, job_type_1, ctx1, &node1);
 *	doca_graph_ctx_node_create(my_graph, job_type_2, ctx2, &node2);
 *
 * Set dependency (node1 -> node2)
 *	doca_graph_add_dependency(my_graph, node1, node2);
 *
 * Start the graph and add to a work queue
 *	doca_graph_start(my_graph);
 *	doca_graph_workq_add(my_graph, my_workq);
 *
 * Create a graph instance and set nodes data
 *	struct doca_graph_instance *my_graph_instance
 *	doca_graph_instance_create(my_graph, &my_graph_instance);
 *	doca_graph_instance_set_ctx_node_data(my_graph_instance, node1, &node_1_job.base, NULL); // event is optional
 *	doca_graph_instance_set_ctx_node_data(my_graph_instance, node2, &node_2_job.base, NULL); // event is optional
 *
 * Submit the graph instance to a work queue
 *	union doca_data my_user_data;
 *	doca_workq_graph_submit(my_workq, my_graph_instance, my_user_data);
 *
 * Call progress retrieve to tick the work queue until graph is completed.
 *	struct doca_event ev;
 *	doca_workq_progress_retrieve(my_workq, &ev, DOCA_WORKQ_RETRIEVE_FLAGS_NONE);
 *	doca_workq_progress_retrieve(my_workq, &ev, DOCA_WORKQ_RETRIEVE_FLAGS_NONE);
 *	analyze job 2 destination buffer.
 *
 * Resubmit instance
 *	node_1_job.src = new_buf...
 *	doca_workq_graph_submit(my_workq, my_graph_instance, my_user_data);
 * @{
 */

#ifndef DOCA_GRAPH_H_
#define DOCA_GRAPH_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <doca_compat.h>
#include <doca_error.h>
#include <doca_ctx.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Graph instance is submitted in a special work queue API, but is retrieved as a job via progress retrieve.
 */
enum doca_graph_job_types {
	/** DOCA_GRAPH_JOB implies that the event is a graph instance. */
	DOCA_GRAPH_JOB = DOCA_ACTION_GRAPH_FIRST + 1,
};

struct doca_graph;
struct doca_graph_node;
struct doca_graph_instance;

/**
 * @brief Creates a DOCA graph
 *
 * This method creates an empty doca_graph.
 *
 * @param [out] graph
 * The created graph.
 * The application is expected to destroy the graph when it is no longer needed (@see doca_graph_destroy)
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_NO_MEMORY - failed to allocate the graph.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_create(struct doca_graph **graph);

/**
 * @brief Destroys a previously created doca_graph
 *
 * A DOCA graph can be destroyed only after the instances created by the graph were destroyed and it was removed from
 * work queues.
 *
 * @param [in] doca_graph
 * The graph to destroy
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_NOT_PERMITTED - Graph instances are not destroyed or graph is still added to work queues.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_destroy(struct doca_graph *graph);

/**
 * @brief Create a context node
 *
 * This method creates a context node (A node that points to a context and contains a doca_job for the context)
 * A node is automatically added to the graph as a root when it is created
 * A node can only be added before the graph is started.
 *
 * @param [in] graph
 * The graph to add the node to.
 * @param [in] job type
 * A node can only serve one job type (even if the context supports more).
 * @param [in] context
 * Context to run the job.
 * @param [out] graph node
 * Reference to the created graph node.
 * The node shall be used to set dependencies and set node data.
 * A node does not need to be destroyed by the application.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - Graph is already started
 * - DOCA_ERROR_NO_MEMORY - Failed to allocate the node
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_ctx_node_create(struct doca_graph *graph, int job_type, const struct doca_ctx *ctx,
					struct doca_graph_node **node);

/**
 * @brief User node callback
 *
 * Definition of a user node callback. @see doca_graph_user_node_create for more details
 *
 * @param [in] cookie
 * A cookie set to the node (@see doca_graph_instance_set_user_node_data).
 * @param [in] event
 * Reference to an event set to the node (@see doca_graph_instance_set_user_node_data). May be null if the application
 * chose not to set it.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - Any doca_error_t (depends on the callback implementation)
 */
typedef doca_error_t (*doca_graph_user_node_cb_t)(void *cookie, struct doca_event *ev);

/**
 * @brief Create a user node
 *
 * This method creates a user node (A node that points to a callback supplied by the user and contains a user
 * defined doca_job.)
 * A node is automatically added to the graph as a root when it is created
 * A node can only be added before the graph is started.
 *
 * @param [in] graph
 * The graph to add the node to.
 * @param [in] callback
 * Callback to be called when the node is executed
 * @param [out] graph node
 * Reference to the created graph node.
 * The node shall be used to set dependencies and set node data.
 * A node does not need to be destroyed by the application.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - Graph is already started
 * - DOCA_ERROR_NO_MEMORY - Failed to allocate the node
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_user_node_create(struct doca_graph *graph, doca_graph_user_node_cb_t cb,
					 struct doca_graph_node **node);

/**
 * @brief Create a sub graph node
 *
 * This method creates a sub graph node (a node that points to a doca_graph).
 * A node is automatically added to the graph as a root when it is created
 * A node can only be added before the graph is started.
 * Sub graph must not form a circle with the graph that it is added to (e.g. Graph A -> Graph B -> Graph A)
 *
 * @param [in] graph
 * The graph to add the node to.
 * @param [in] sub graph
 * Graph to be executed as a sub graph.
 * @param [out] graph node
 * Reference to the created graph node.
 * The node shall be used to set dependencies and set node data.
 * A node does not need to be destroyed by the application.
 *
 * @param [in] sub graph
 *
 * @param [out] node
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - Graph is already started or sub graph is not started.
 * - DOCA_ERROR_NO_MEMORY - Failed to allocate the node
 * - DOCA_ERROR_NOT_PERMITTED - Sub graph forms a circle (e.g. pointing to the graph or forming a circle with one of the
 *   nodes).
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_sub_graph_node_create(struct doca_graph *graph, struct doca_graph *sub_graph,
					      struct doca_graph_node **node);

/**
 * @brief Set dependencies
 *
 * This method adds a dependent node to a node.
 * Node dependency can only be set before the graph is started.
 * Setting dependency must not form a circle in the graph
 *
 * @param [in] doca_graph
 * The graph that both from node and to node reside in.
 * @param [in] from
 * Node to depend on
 * @param [in] to
 * Node that depends on the from node
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - Graph is already started.
 * - DOCA_ERROR_NO_MEMORY - Failed to allocate dependency.
 * - DOCA_ERROR_NOT_PERMITTED - Dependency forms a circle.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_add_dependency(struct doca_graph *graph, struct doca_graph_node *from,
				       struct doca_graph_node *to);

/**
 * @brief Start a graph
 *
 * This method starts a graph.
 * A doca_graph can only be used after it was started (@see details and pseudo code example at the top of the header
 * file).
 * A doca_graph can only be started if all contexts (in the context nodes) were started.
 *
 * @param [in] graph
 * Graph to start
 *
 * @return
 * DOCA_SUCCESS - in case of success or if the graph is already started.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - Graph does not contain a context node (graph must contain at least one context node) or
 *			    graph is already started.
 * - DOCA_ERROR_NO_MEMORY - Failed to allocate run graph time data.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_start(struct doca_graph *graph);

/**
 * @brief Stop a graph
 *
 * This method stops a graph. A graph can be stopped only after all the instances created by it were destroyed and
 * after it was removed from all work queues it was added to.
 *
 * @param [in] graph
 * Graph to stop
 *
 * @return
 * DOCA_SUCCESS - in case of success or if the graph is already stopped.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_NOT_PERMITTED - graph instances are not destroyed or graph is still added to work queues.
 * - DOCA_ERROR_BAD_STATE - graph is already stopped.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_stop(struct doca_graph *graph);

/**
 * @brief Add graph to a work queue
 *
 * This method adds a graph to the work queue.
 * A graph can be added to one or more work queues.
 * A graph must be started before it is added to the work queue.
 * A work queue must contain all contexts that the graph uses before the graph is added. These contexts can not be
 * removed from the work queue as long as the graph is added to the work queue.
 * Adding a graph to a work queue is thread safe.
 *
 * @param [in] graph
 * Graph to add to the work queue.
 * @param [in] work queue
 * Work queue to add the graph to.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - Graph is not started or one of the contexts it requires is not in the work queue.
 * - DOCA_ERROR_NO_MEMORY - Failed to allocate memory for the graph in the work queue.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_workq_add(struct doca_graph *graph, struct doca_workq *workq);

/**
 * @brief Remove graph from a work queue
 *
 * This method removes a graph from a work queue
 * A graph can only be removed when the work queue is empty
 *
 * @param [in] graph
 * Graph to remove from the work queue
 * @param [in] work queue
 * Work queue to remove the graph from
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_IN_USE - Work queue contains in-flight jobs.
 * - DOCA_ERROR_BAD_STATE
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_workq_rm(struct doca_graph *graph, struct doca_workq *workq);

/**
 * @brief Create a graph instance
 *
 * This method creates a graph instance.
 * Graph instance contains the nodes data (jobs, callbacks, sub graphs, etc.) and is submitted to a work queue to be
 * executed. A graph must be started before it can create an instance.
 * Removing a graph from a work queue is thread safe.
 *
 * @param [in] graph
 * Graph to create the instance from.
 * @param [out] graph_instance
 * Instance created by the graph.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - Graph is not started.
 * - DOCA_ERROR_NO_MEMORY - Failed to allocate memory for the graph instance.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_create(const struct doca_graph *graph, struct doca_graph_instance **graph_instance);

/**
 * @brief Destroy graph instance
 *
 * This method destroys a graph instance
 * A graph instance can not be destroyed if it is submitted to a work queue or if it is set as a sub graph node data.
 *
 * @param [in] graph instance
 * Graph instance to destroy
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_NOT_PERMITTED - graph instance is submitted to a work queue.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_destroy(struct doca_graph_instance *graph_instance);

/**
 * @brief Set context node data
 *
 * This method sets context node data (job and event).
 * It is recommended to set the node data once and change the job content (if required) every instance run.
 *
 * @param [in] graph instance
 * Graph instance to set the node data to
 * @param [in] graph node
 * Graph node that facilitates setting the data to the correct node in the instance.
 * - Node must belong to the graph that created the instance
 * - Node must be a context node.
 * created the instance.
 * @param [in] doca job.
 * doca_job to set to the node. The job type and context must match the type and context of the graph node.
 * doca_job lifespan must be >= to the lifespan of the graph instance.
 * @param [in][optional] doca event
 * event for job result. This parameter is optional.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - instance is submitted to a work queue.
 * - DOCA_ERROR_NOT_PERMITTED - node does not belong to the graph that created the instance, job type mismatch,
 *				invalid context, etc.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_set_ctx_node_data(struct doca_graph_instance *graph_instance,
						   struct doca_graph_node *node, struct doca_job *job,
						   struct doca_event *ev);

/**
 * @brief Set user node data
 *
 * This method sets user node data
 * It is recommended to set the node data once and change the job content (if required) every instance run.
 *
 * @param [in] graph instance
 * Graph instance to set the node data to
 * @param [in] graph node
 * Graph node that facilitates setting the data to the correct node in the instance.
 * - Node must belong to the graph that created the instance
 * - Node must be a user node.
 * @param [in] cookie
 * cookie supplied by the application (passed to the callback when it is executes).
 * @param [in][optional] doca event
 * event for job result. This parameter is optional.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - instance is submitted to a work queue.
 * - DOCA_ERROR_NOT_PERMITTED - node does not belong to the graph that created the instance
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_set_user_node_data(struct doca_graph_instance *graph_instance,
						    struct doca_graph_node *node, void *cookie, struct doca_event *ev);

/**
 * @brief Set sub graph node data
 *
 * This method sets sub graph node data
 * It is recommended to set the node data once and change the job content (if required) every instance run.
 *
 * @param [in] graph instance
 * Graph instance to set the node data to
 * @param [in] graph node
 * Graph node that facilitates setting the data to the correct node in the instance.
 * - Node must belong to the graph that created the instance
 * - Node must be a sub graph node.
 * @param [in] sub graph instance.
 * Graph instance to be run by the node.
 * -- Instance must be created by the graph that the sub graph node was created with.
 * -- Instance must not be submitted to a work queue.
 * @param [in][optional] doca event
 * event for job result. This parameter is optional.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_BAD_STATE - instance is submitted to a work queue.
 * - DOCA_ERROR_NOT_PERMITTED - node does not belong to the graph that created the instance, sub graph instance is
 *   submitted to a work queue, etc.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_set_sub_graph_node_data(struct doca_graph_instance *graph_instance,
							 struct doca_graph_node *node,
							 struct doca_graph_instance *sub_graph_instance,
							 struct doca_event *ev);

/**
 * @brief Submit graph instance to a work queue
 *
 * This method submits a graph instance to a work queue
 * Graph submission executes the graph root nodes.
 * A submitted graph can't be aborted or flushed.
 *
 * @param [in] work queue
 * The work queue to submit the graph instance to
 * @param [in] graph instance
 * The graph instance to submit
 * @param [in] user data
 * User data that shall be received in progress retrieve event when the graph instance execution is completed.
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 * - DOCA_ERROR_INVALID_VALUE - invalid input received.
 * - DOCA_ERROR_IN_USE - The graph instance is already submitted
 * - DOCA_ERROR_NO_MEMORY - Work queue is overloaded and does not have enough room to run the graph.
 * - other doca_error_t statuses may be popped up from root jobs submission.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_workq_graph_submit(struct doca_workq *workq, struct doca_graph_instance *graph_instance,
				     union doca_data user_data);


/**
 * Status APIs
 *
 * The APIs below can be used to query graph instance or nodes status
 * These APIs can be used during graph instance execution or after it has completed.
 */

/**
 * @brief This method retrieves the number of failed nodes in a graph instance
 *
 * @param [in] graph_instance
 * Graph instance to query
 * @param [out] num_failed_nodes
 * Number of failed nodes
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 *  DOCA_ERROR_INVALID_VALUE - invalid input received.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_get_num_failed_nodes(const struct doca_graph_instance *graph_instance,
						      uint32_t *num_failed_nodes);

/**
 * @brief This method retrieves the failed nodes of a graph instance
 *
 * @param [in] graph_instance
 * Graph instance to query
 * @param [out] failed_nodes
 * Array of the failed nodes
 * @param [in][out] num_failed_nodes
 * Failed nodes array size as input, num filled nodes as output (may be smaller than actual num failed nodes if the
 * array size is too small).
 *
 * * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 *  DOCA_ERROR_INVALID_VALUE - invalid input received.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_get_failed_nodes(const struct doca_graph_instance *graph_instance,
						  const struct doca_graph_node **failed_nodes, uint32_t *num_failed_nodes);

/**
 * @brief graph instance node state
 * This enum defines the states that a graph node can be in
 */
enum doca_graph_instance_node_state {
	DOCA_GRAPH_INSTANCE_NODE_STATE_NOT_STARTED, /** One or more node parents are incomplete */
	DOCA_GRAPH_INSTANCE_NODE_STATE_STARTED,	    /** Node is being executed */
	DOCA_GRAPH_INSTANCE_NODE_STATE_COMPLETED,   /** Node execution completed */
	DOCA_GRAPH_INSTANCE_NODE_STATE_ERROR,	    /** Node submission or execution failed */
};

/**
 * @brief Get graph instance node state
 * This method retrieves a graph instance node state
 *
 * @param [in] graph_instance
 * Graph instance to get the node state from.
 * @param [in] node
 * Graph node to get the state from
 * @param [out] state
 * Node state
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 *  DOCA_ERROR_INVALID_VALUE - invalid input received.
 *  DOCA_ERROR_NOT_PERMITTED - graph instance and node mismatch
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_node_get_state(const struct doca_graph_instance *graph_instance,
						const struct doca_graph_node *node,
						enum doca_graph_instance_node_state *state);

/**
 * @brief Get graph instance node result
 * This method retrieves a graph instance node result. A result can only be retrieved after the node was completed.
 *
 * @param [in] graph_instance
 * Graph instance to get the node result from.
 * @param [in] node
 * Graph node to get the result from
 * @param [out] result
 * Node result
 *
 * @return
 * DOCA_SUCCESS - in case of success.
 * Error code - in case of failure:
 *  DOCA_ERROR_INVALID_VALUE - invalid input received.
 *  DOCA_ERROR_NOT_PERMITTED - graph instance and node mismatch.
 *  DOCA_ERROR_BAD_STATE - node is not completed yet.
 */
__DOCA_EXPERIMENTAL
doca_error_t doca_graph_instance_node_get_result(const struct doca_graph_instance *graph_instance,
						 const struct doca_graph_node *node, union doca_data *result);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* DOCA_GRAPH_H_ */



