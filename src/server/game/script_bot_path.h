#ifndef SCRIPT_BOT_PATH_H
#define SCRIPT_BOT_PATH_H

#include <stdint.h>

#include "qcommon/q_vector_types.h"

/* The largest shipped waypoint set is mp_hurtgen_tdm at 2157 nodes, and the
 * widest out-degree across every shipped set is 8 once walk links are
 * mirrored. Both bounds hold with margin, so the graph lives in a fixed array
 * and a search needs no allocation. */
#define CODUOMP_BOTPATH_MAX_NODES 2560
#define CODUOMP_BOTPATH_MAX_LINKS 16

typedef struct {
    vec3_t origin;
    /* Penalty is per directed link rather than per node. A mirrored link is a
     * distinct entry with its own counter, so penalising the direction a bot
     * just travelled leaves the return direction free and route diversification
     * pushes away from the exact edge that failed. */
    float linkPenalty[CODUOMP_BOTPATH_MAX_LINKS];
    uint16_t link[CODUOMP_BOTPATH_MAX_LINKS];
    uint8_t linkCount;
    uint8_t mirrorable;
    uint16_t typeKey;
    int32_t stance;
} coduomp_botpath_node_t;

/* Discards every node and every accumulated link penalty. A search against an
 * empty store finds nothing. */
void coduomp_botpath_reset(void);

int32_t coduomp_botpath_node_count(void);

/* Appends a node with no links and returns its index, or -1 when the store is
 * full. `typeKey` is opaque to the search: it is carried through so the caller
 * can hand the authored waypoint type back to script. `mirrorable` marks the
 * type whose links may be traversed in both directions. */
int32_t coduomp_botpath_add(const vec3_t origin, uint16_t typeKey,
                             uint8_t mirrorable, int32_t stance);

/* Attaches a one-way link, returning qfalse when `from` is out of range or is
 * already at its link ceiling. `to` is stored as given and may name a node that
 * has not been added yet, which is the normal shape of a waypoint file. */
qboolean coduomp_botpath_link(int32_t from, int32_t to);

/* Runs the reverse-edge pass over every node added since the last reset and
 * returns the resulting node count. Must be called once after the last add,
 * because a reverse link can only be attached once both of its endpoints
 * exist. */
int32_t coduomp_botpath_finish(void);

/* Reverse edges the last finish() could not attach because the receiving node
 * was already at its link ceiling. Reset by coduomp_botpath_reset() and by
 * finish() itself. */
int32_t coduomp_botpath_dropped_links(void);

/* Links the last finish() removed because they named a node the waypoint file
 * never declared. A non-zero count means the map data itself is inconsistent. */
int32_t coduomp_botpath_invalid_links(void);

/* Index of the node nearest `pos`, or -1 when the store is empty. Ties resolve
 * to the lowest index. */
int32_t coduomp_botpath_nearest(const vec3_t pos);

/* Writes the next route from `start` to `goal` into `outPath`, which must have
 * room for CODUOMP_BOTPATH_MAX_NODES elements, and its length into
 * `outLength`. Every edge the route uses is penalised before returning, so
 * repeated calls yield successively different routes rather than the same one
 * again. The start node is excluded: the caller is already standing on it.
 * Returns 0 when no route exists or when the route just returned would repeat
 * the one before it. */
int32_t coduomp_botpath_next_path(int32_t start, int32_t goal,
                                  int32_t *outPath, int32_t *outLength);

/* Bounds-checked accessor, NULL when `index` is out of range. */
const coduomp_botpath_node_t *coduomp_botpath_node(int32_t index);

/* Script entry points, defined in script_bot_path_builtins.c. They are the
 * only code that touches both this graph and the script VM, and are declared
 * here so the registration tables can name them without pulling in the wider
 * game headers. */
void coduomp_botpath_map_reset(void);
void script_func_waypoint_graph_begin(void);
void script_func_waypoint_add(void);
void script_func_waypoint_link(void);
void script_func_waypoint_graph_end(void);
void PlayerCmd_GetPathsToWaypoint(uint32_t entityNum);

#endif /* SCRIPT_BOT_PATH_H */