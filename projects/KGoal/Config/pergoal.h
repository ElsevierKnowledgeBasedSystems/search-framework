#ifndef CONFIG_MIN_H
#define CONFIG_MIN_H

#include "common.h"

#define SLB_NODE_DATA NodeBase<SLB_STATE>
#define SLB_OL_PRIORITY GreaterPriority_SmallF_LargeG

#define SLB_GOAL_HANDLER SingleGoalHandler

#define SLB_BASE_HEURISTIC OctileHeuristic
#define SLB_HEURISTIC SimpleHeuristicToGoal_FixedH

#define SLB_ALGORITHM PerGoal
#define SLB_PER_GOAL_BASE_ALGORITHM Astar

#endif