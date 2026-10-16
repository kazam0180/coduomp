/* NOT_FROM_ORIGINAL_SOURCE: script-facing entry points for the bot waypoint
 * pathfinder. The graph itself lives in script_bot_path.c, which has no script
 * VM dependency; this file is the only place that crosses that boundary.
 *
 * Script contract:
 *   waypoint_graph_begin()
 *   waypoint_add(originX, originY, originZ, type, stance) -> index
 *   waypoint_link(fromIndex, toIndex) -> bool
 *   waypoint_graph_end() -> node count
 *   self get_paths_to_waypoint(goalIndex, numPaths)
 *       -> [ [ {index, origin, type, stance}, ... ], ... ]
 */

#include <stdint.h>
#include <string.h>

#include "recovered_game.h"
#include "game_functions.h"
#include "game_globals.h"
#include "scr_vm.h"
#include "script_bot_path.h"

/* The only waypoint type whose links may be walked in both directions. Every
 * other type (jump, ladder, fall, camp, goal) records one-way traversal that a
 * reverse edge would turn into a bot stepping off a ledge. */
#define CODUOMP_BOTPATH_WALK_TYPE "w"

/* Upper bound on the routes one call may return, so a caller passing a large
 * count cannot make the search run away. */
#define CODUOMP_BOTPATH_MAX_PATHS 8

/* One route's worth of waypoint indices, times the route ceiling. Held at file
 * scope rather than on the stack: at the node ceiling this is 80 KB, and a bot
 * repathing inside a script thread must not push that deep. */
static int32_t botpathRoutes[CODUOMP_BOTPATH_MAX_PATHS *
                             CODUOMP_BOTPATH_MAX_NODES];
static int32_t botpathRouteLengths[CODUOMP_BOTPATH_MAX_PATHS];

typedef struct {
    uint16_t keyIndex;
    uint16_t keyOrigin;
    uint16_t keyType;
    uint16_t keyStance;
    uint16_t walkType;
    uint8_t initialized;
} coduomp_botpath_keys_t;

static coduomp_botpath_keys_t botpathKeys;

/* Interns the emitted field names and the walk type once. The scr_const table
 * is full at 159 of its 160 slots, so these are interned directly rather than
 * claiming pre-interned slots the original table does not leave free. */
static const coduomp_botpath_keys_t *botpath_get_keys(void)
{
    if (!botpathKeys.initialized) {
        botpathKeys.keyIndex = Scr_AllocString("index", 1);
        botpathKeys.keyOrigin = Scr_AllocString("origin", 1);
        botpathKeys.keyType = Scr_AllocString("type", 1);
        botpathKeys.keyStance = Scr_AllocString("stance", 1);
        botpathKeys.walkType = Scr_AllocString(CODUOMP_BOTPATH_WALK_TYPE, 1);
        botpathKeys.initialized = 1;
    }

    return &botpathKeys;
}

/* Map change invalidates every node and every accumulated link penalty. Called
 * before the level script loads, so a graph left over from the previous map can
 * never be searched. */
void coduomp_botpath_map_reset(void)
{
    coduomp_botpath_reset();
}

/* NOT_FROM_ORIGINAL_SOURCE */
void script_func_waypoint_graph_begin(void)
{
    coduomp_botpath_reset();
    Scr_AddInt(0);
}

/* NOT_FROM_ORIGINAL_SOURCE: appends one waypoint with no links and returns its
 * index. Links are attached separately by waypoint_link, so script never has to
 * pad a call out to a fixed neighbour count. */
void script_func_waypoint_add(void)
{
    const coduomp_botpath_keys_t *keys = botpath_get_keys();
    vec3_t origin;
    const char *type;
    uint16_t typeKey;
    int32_t index;

    if (Scr_GetNumParam() != 5 || Scr_GetType(3) != SCRIPT_VAR_STRING ||
        Scr_GetType(4) != SCRIPT_VAR_INT) {
        Com_Printf("waypoint_add: USAGE: waypoint_add(x, y, z, type, stance)\n");
        Scr_AddInt(-1);
        return;
    }

    origin[0] = Scr_GetFloat(0);
    origin[1] = Scr_GetFloat(1);
    origin[2] = Scr_GetFloat(2);
    type = Scr_GetString(3);

    /* Interning puts the authored type on the same table the compiler uses for
     * string literals, so the handle compares equal to the one script's own
     * `wp.type == "w"` resolves to. */
    typeKey = Scr_AllocString(type, 1);

    index = coduomp_botpath_add(origin, typeKey,
                                (typeKey == keys->walkType) ? 1 : 0,
                                Scr_GetInt(4));

    if (index < 0) {
        Com_Printf("waypoint_add: rejected; the waypoint store is full\n");
    }

    Scr_AddInt(index);
}

/* NOT_FROM_ORIGINAL_SOURCE: attaches a one-way link from one waypoint to
 * another. Returns false when the source is out of range or already carries
 * the maximum number of links. */
void script_func_waypoint_link(void)
{
    if (Scr_GetNumParam() != 2 || Scr_GetType(0) != SCRIPT_VAR_INT ||
        Scr_GetType(1) != SCRIPT_VAR_INT) {
        Com_Printf("waypoint_link: USAGE: waypoint_link(fromIndex, toIndex)\n");
        Scr_AddBool(qfalse);
        return;
    }

    Scr_AddBool(coduomp_botpath_link(Scr_GetInt(0), Scr_GetInt(1)));
}

/* NOT_FROM_ORIGINAL_SOURCE: closes the graph, mirroring walk links in both
 * directions, and returns the node count. */
void script_func_waypoint_graph_end(void)
{
    int32_t nodeCount = coduomp_botpath_finish();
    int32_t dropped = coduomp_botpath_dropped_links();
    int32_t invalid = coduomp_botpath_invalid_links();

    if (dropped > 0) {
        Com_Printf("get_paths_to_waypoint: %d reverse links dropped; a node "
                   "exceeded the %d link ceiling\n",
                   dropped, (int)CODUOMP_BOTPATH_MAX_LINKS);
    }

    if (invalid > 0) {
        Com_Printf("get_paths_to_waypoint: %d links named a waypoint this map "
                   "does not define and were dropped\n",
                   invalid);
    }

    Scr_AddInt(nodeCount);
}

/* NOT_FROM_ORIGINAL_SOURCE: emits one waypoint as {index, origin, type,
 * stance}, appending it to the array sitting below it on the value stack. */
static void botpath_push_waypoint(int32_t index,
                                  const coduomp_botpath_keys_t *keys)
{
    const coduomp_botpath_node_t *node = coduomp_botpath_node(index);

    if (node == NULL) {
        return;
    }

    /* A struct, not an array. IsFieldObject accepts only types below
     * SCRIPT_VAR_ARRAY, so a node built by Scr_MakeArray is readable from
     * script only through bracket syntax; `.index` on one raises "array is
     * not an object". Scr_AddStruct is the spawnstruct() builtin and produces
     * the SCRIPT_VAR_STRUCT node dot access requires. */
    Scr_AddStruct();

    Scr_AddInt(index);
    Scr_AddArrayStringIndexed(keys->keyIndex);

    Scr_AddVector(node->origin);
    Scr_AddArrayStringIndexed(keys->keyOrigin);

    Scr_AddConstString(node->typeKey);
    Scr_AddArrayStringIndexed(keys->keyType);

    Scr_AddInt(node->stance);
    Scr_AddArrayStringIndexed(keys->keyStance);

    Scr_AddArray();
}

/* NOT_FROM_ORIGINAL_SOURCE */
void PlayerCmd_GetPathsToWaypoint(uint32_t entityNum)
{
    gentity_t *ent = &g_entities[entityNum];
    const coduomp_botpath_keys_t *keys = botpath_get_keys();
    int32_t routeCount = 0;
    int32_t offset = 0;
    int32_t nodeCount = coduomp_botpath_node_count();
    int32_t start;
    int32_t goal;
    int32_t maxPaths;
    int32_t pass;
    int32_t route;

    if (ent->client == NULL) {
        Com_Printf("get_paths_to_waypoint: entity %d is not a player\n",
                   (int)entityNum);
        Scr_MakeArray();
        return;
    }

    if (Scr_GetNumParam() != 2 || Scr_GetType(0) != SCRIPT_VAR_INT ||
        Scr_GetType(1) != SCRIPT_VAR_INT) {
        Com_Printf("get_paths_to_waypoint: USAGE: self get_paths_to_waypoint "
                   "<goalIndex> <numPaths>\n");
        Scr_MakeArray();
        return;
    }

    goal = Scr_GetInt(0);
    maxPaths = Scr_GetInt(1);

    if (maxPaths > CODUOMP_BOTPATH_MAX_PATHS) {
        maxPaths = CODUOMP_BOTPATH_MAX_PATHS;
    }

    start = coduomp_botpath_nearest(ent->s.origin);

    /* A route can never be longer than the node count, so leaving a full node
     * count of room before each search is what keeps the write in bounds. */
    for (pass = 0; pass < maxPaths; pass++) {
        int32_t length = 0;

        if (offset + nodeCount > (int32_t)(sizeof(botpathRoutes) /
                                          sizeof(botpathRoutes[0]))) {
            break;
        }

        if (!coduomp_botpath_next_path(start, goal, botpathRoutes + offset,
                                       &length)) {
            break;
        }

        botpathRouteLengths[routeCount++] = length;
        offset += length;
    }

    Scr_MakeArray();

    offset = 0;
    for (route = 0; route < routeCount; route++) {
        int32_t hop;

        Scr_MakeArray();

        for (hop = 0; hop < botpathRouteLengths[route]; hop++) {
            botpath_push_waypoint(botpathRoutes[offset + hop], keys);
        }

        Scr_AddArray();
        offset += botpathRouteLengths[route];
    }
}