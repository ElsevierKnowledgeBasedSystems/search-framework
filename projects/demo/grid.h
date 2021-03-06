#ifndef SLB_PROJECTS_DEMO_GRID_H
#define SLB_PROJECTS_DEMO_GRID_H

#define SLB_STATE ext::domain::grid::Grid<>
#define SLB_COST_TYPE double
#define SLB_UNIFORM_DOMAIN false
#define SLB_COST_DIAGONAL 1.5
#define SLB_BASE_HEURISTIC ext::domain::grid::OctileHeuristic<>

#endif
