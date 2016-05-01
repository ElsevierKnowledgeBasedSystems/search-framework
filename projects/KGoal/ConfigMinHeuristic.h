#define SLB_VISUALIZATION 50

#define SLB_STATE GridMapState<>
#define SLB_COST_TYPE double
#define SLB_UNIFORM_DOMAIN false
#define SLB_NODE_DATA MyNodeDataT<SLB_STATE>

#define SLB_STATE_SMART_PTR StateSharedPtrT
#define SLB_BUCKET_POSITION_TYPE int
#define SLB_NODE AStarNode<>

#define SLB_ALGORITHM_EVENT AstarEvent<>
// Can't put ALGORITHM_EVENT as default argument in AlgorithmLogger,
// since AlgorithmEvent.h uses AlgorithmLogger.
#define SLB_LOGGER AlgorithmLog<SLB_NODE>

#define SLB_BUILD_GRAPH // Need to build the domain graph
#define SLB_BUILD_GRAPH_GOAL_HANDLER = NoGoalHandler<>

// Can't put defaults in OL, since there are several instantiations
#define SLB_OL_PRIORITY_TYPE DefaultPriority
#define SLB_OL_PRIORITY GreaterPriority_SmallF_LargeG
#define SLB_OL_CONTAINER OLMap
#define SLB_OL OpenList<>

#define SLB_GOAL_HANDLER MinHeuristicGoalHandler

#define SLB_HEURISTIC MinHeuristicToGoals<SLB_STATE, OctileHeuristic>
#define SLB_ALGORITHM Astar<>
#define SLB_RAW_ALGORITHM SLB_ALGORITHM