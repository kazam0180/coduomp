/* NOT_FROM_ORIGINAL_SOURCE: waypoint graph store and A* route search for the
 * bot pathfinder. Nothing here touches the script VM, so the same translation
 * unit is compiled into the offline route harness. */

#include <string.h>

#include "math/q_math.h"
#include "qcommon/q_vector_types.h"
#include "script_bot_path.h"

static coduomp_botpath_node_t botpathNodes[CODUOMP_BOTPATH_MAX_NODES];
static int32_t botpathNodeCount;

/* Search working set. Indexed by node; reset_search() clears exactly
 * [0, botpathNodeCount) so a stale entry from a larger previous graph cannot
 * reach a comparison. */
static float botpathG[CODUOMP_BOTPATH_MAX_NODES];
static float botpathF[CODUOMP_BOTPATH_MAX_NODES];
static int32_t botpathParent[CODUOMP_BOTPATH_MAX_NODES];
static uint8_t botpathClosed[CODUOMP_BOTPATH_MAX_NODES];

/* Binary min-heap over f. Stores node indices only; ties break on the index so
 * a run is reproducible. One-based so that parent/child arithmetic reaching
 * zero is a bug rather than a silent root case. */
static int32_t botpathHeap[CODUOMP_BOTPATH_MAX_NODES + 1];
static int32_t botpathHeapSize;

/* The route handed out by the previous call, kept only to detect a retry that
 * found nothing better. */
static int32_t botpathPrevious[CODUOMP_BOTPATH_MAX_NODES];
static int32_t botpathPreviousLength;
static uint8_t botpathHasPrevious;

/* Reverse edges the last finish() could not attach because the receiving node
 * was already at its link ceiling. The caller reports this rather than the
 * graph truncating itself silently. */
static int32_t botpathDroppedLinks;

/* Links the last finish() removed because they named a node the file never
 * declared. Kept apart from the ceiling count so a malformed waypoint file is
 * not reported as a capacity problem. */
static int32_t botpathInvalidLinks;

#define BOTPATH_HEAP_ROOT 1

/* A route whose every edge already carries this much extra cost cannot be made
 * to diverge by another pass, so the retry stops rather than handing back the
 * same chain under a new name. */
#define BOTPATH_PENALTY_FACTOR 2.0f

static void botpath_heap_swap(int32_t a, int32_t b)
{
    int32_t swap = botpathHeap[a];

    botpathHeap[a] = botpathHeap[b];
    botpathHeap[b] = swap;
}

static qboolean botpath_heap_push(int32_t node)
{
    int32_t child;

    if (botpathHeapSize >= CODUOMP_BOTPATH_MAX_NODES) {
        return qfalse;
    }

    child = ++botpathHeapSize;
    botpathHeap[child] = node;

    while (child > BOTPATH_HEAP_ROOT) {
        int32_t parent = child >> 1;

        if (botpathF[botpathHeap[parent]] <= botpathF[botpathHeap[child]]) {
            break;
        }

        botpath_heap_swap(parent, child);
        child = parent;
    }

    return qtrue;
}

static qboolean botpath_heap_pop(int32_t *outNode)
{
    int32_t index = BOTPATH_HEAP_ROOT;

    if (botpathHeapSize == 0) {
        return qfalse;
    }

    *outNode = botpathHeap[BOTPATH_HEAP_ROOT];
    botpathHeap[BOTPATH_HEAP_ROOT] = botpathHeap[botpathHeapSize--];

    while (index <= (botpathHeapSize >> 1)) {
        int32_t child = index << 1;

        if (child < botpathHeapSize &&
            botpathF[botpathHeap[child + 1]] < botpathF[botpathHeap[child]]) {
            child++;
        }
        if (botpathF[botpathHeap[index]] <= botpathF[botpathHeap[child]]) {
            break;
        }

        botpath_heap_swap(index, child);
        index = child;
    }

    return qtrue;
}

static void botpath_reset_search(void)
{
    int32_t index;

    for (index = 0; index < botpathNodeCount; index++) {
        botpathG[index] = 0.0f;
        botpathF[index] = 0.0f;
        botpathParent[index] = -1;
        botpathClosed[index] = 0;
    }
    botpathHeapSize = 0;
}

/* Adds `to` to `from`'s link list unless it is already reachable that way.
 * Returns qfalse when the node is at its link ceiling, which the caller
 * reports rather than silently truncating the graph. */
static qboolean botpath_link(int32_t from, int32_t to)
{
    coduomp_botpath_node_t *node = &botpathNodes[from];
    int32_t index;

    for (index = 0; index < node->linkCount; index++) {
        if (node->link[index] == to) {
            return qtrue;
        }
    }

    if (node->linkCount >= CODUOMP_BOTPATH_MAX_LINKS) {
        return qfalse;
    }

    node->linkPenalty[node->linkCount] = 0.0f;
    node->link[node->linkCount] = (uint16_t)to;
    node->linkCount++;
    return qtrue;
}

/* Penalties are memory of failure, and memory has to fade: without a ceiling
 * every repath taxes the walked edges again, so a room whose routes keep
 * failing becomes untraversable in planning while the bot is still standing in
 * it. Each replan then avoids the last failure harder, which walks a stuck bot
 * from one corner of the room to the other instead of out of it. Clamped at
 * four edge lengths, a failed edge costs at most five times its length: enough
 * to prefer a genuinely different branch, never enough to prefer an absurd
 * detour or to turn visited ground into a permanent no-go zone. */
#define BOTPATH_PENALTY_MAX_FACTOR 4.0f

/* Charges every directed edge the route used, so the next search prefers a
 * different branch at each node the route passed through. */
static void botpath_penalize_route(const int32_t *path, int32_t length,
                                   int32_t start)
{
    int32_t index;

    for (index = 0; index < length; index++) {
        int32_t from = (index == 0) ? start : path[index - 1];
        int32_t to = path[index];
        int32_t link;

        for (link = 0; link < botpathNodes[from].linkCount; link++) {
            if (botpathNodes[from].link[link] != to) {
                continue;
            }

            float edgeLength = VectorDistance(botpathNodes[from].origin,
                                                botpathNodes[to].origin);
            float penalty = botpathNodes[from].linkPenalty[link] +
                            edgeLength * BOTPATH_PENALTY_FACTOR;
            float ceiling = edgeLength * BOTPATH_PENALTY_MAX_FACTOR;

            botpathNodes[from].linkPenalty[link] =
                (penalty > ceiling) ? ceiling : penalty;
            break;
        }
    }
}

/* NOT_FROM_ORIGINAL_SOURCE: one A* pass over the current penalties.
 * Reconstructs `goal` back to `start` into `outPath` and returns qfalse when
 * no route exists. */
static qboolean botpath_search_once(int32_t start, int32_t goal,
                                    int32_t *outPath, int32_t *outLength)
{
    int32_t node;

    botpath_reset_search();

    botpathG[start] = 0.0f;
    botpathF[start] = VectorDistance(botpathNodes[start].origin,
                                     botpathNodes[goal].origin);
    botpathParent[start] = -1;

    if (!botpath_heap_push(start)) {
        return qfalse;
    }

    while (botpath_heap_pop(&node)) {
        int32_t index;

        if (botpathClosed[node]) {
            continue;
        }
        botpathClosed[node] = 1;

        if (node == goal) {
            int32_t length = 0;

            for (index = goal; index != -1; index = botpathParent[index]) {
                if (index != start) {
                    outPath[length++] = index;
                }
            }

            /* Parents run goal->start; the caller walks start->goal. */
            for (index = 0; index < length / 2; index++) {
                int32_t swap = outPath[index];

                outPath[index] = outPath[length - 1 - index];
                outPath[length - 1 - index] = swap;
            }

            *outLength = length;
            return qtrue;
        }

        for (index = 0; index < botpathNodes[node].linkCount; index++) {
            int32_t next = botpathNodes[node].link[index];
            float cost;

            if (botpathClosed[next]) {
                continue;
            }

            cost = botpathG[node] +
                   VectorDistance(botpathNodes[node].origin,
                                  botpathNodes[next].origin) +
                   botpathNodes[node].linkPenalty[index];

            if (botpathParent[next] == -1 || cost < botpathG[next]) {
                botpathG[next] = cost;
                botpathF[next] = cost +
                                 VectorDistance(botpathNodes[next].origin,
                                                botpathNodes[goal].origin);
                botpathParent[next] = node;
                if (!botpath_heap_push(next)) {
                    return qfalse;
                }
            }
        }
    }

    return qfalse;
}

void coduomp_botpath_reset(void)
{
    botpathNodeCount = 0;
    botpathHeapSize = 0;
    botpathPreviousLength = 0;
    botpathHasPrevious = 0;
    botpathDroppedLinks = 0;
    botpathInvalidLinks = 0;
}

int32_t coduomp_botpath_node_count(void)
{
    return botpathNodeCount;
}

const coduomp_botpath_node_t *coduomp_botpath_node(int32_t index)
{
    if (index < 0 || index >= botpathNodeCount) {
        return NULL;
    }
    return &botpathNodes[index];
}

int32_t coduomp_botpath_add(const vec3_t origin, uint16_t typeKey,
                            uint8_t mirrorable, int32_t stance)
{
    coduomp_botpath_node_t *node;

    if (botpathNodeCount >= CODUOMP_BOTPATH_MAX_NODES) {
        return -1;
    }

    node = &botpathNodes[botpathNodeCount];
    memcpy(node->origin, origin, sizeof(node->origin));
    node->typeKey = typeKey;
    node->mirrorable = mirrorable;
    node->stance = stance;
    node->linkCount = 0;

    return botpathNodeCount++;
}

qboolean coduomp_botpath_link(int32_t from, int32_t to)
{
    if (from < 0 || from >= botpathNodeCount) {
        return qfalse;
    }

    /* Stored as given. Waypoint files routinely point forward, so a link
     * naming a node that has not been added yet is normal and is resolved by
     * finish() once the whole file has been read. */
    return botpath_link(from, to);
}

int32_t coduomp_botpath_finish(void)
{
    int32_t from;

    botpathDroppedLinks = 0;
    botpathInvalidLinks = 0;

    /* Resolve forward references first: a link may name a node that was added
     * after it, and a link naming a node that does not exist at all is
     * dropped rather than left to dangle. */
    for (from = 0; from < botpathNodeCount; from++) {
        int32_t write = 0;
        int32_t read;

        for (read = 0; read < botpathNodes[from].linkCount; read++) {
            int32_t to = botpathNodes[from].link[read];

            if (to >= botpathNodeCount) {
                botpathInvalidLinks++;
                continue;
            }
            botpathNodes[from].link[write] = botpathNodes[from].link[read];
            write++;
        }
        botpathNodes[from].linkCount = (uint8_t)write;
    }

    for (from = 0; from < botpathNodeCount; from++) {
        /* Only the links the author recorded are mirrored. Appending to a
         * neighbour changes that neighbour's own list, so the count is captured
         * before the loop rather than re-read each iteration. */
        int32_t forwardCount = botpathNodes[from].linkCount;
        int32_t index;

        if (!botpathNodes[from].mirrorable) {
            continue;
        }

        for (index = 0; index < forwardCount; index++) {
            int32_t to = botpathNodes[from].link[index];

            if (to == from) {
                continue;
            }

            if (!botpath_link(to, from)) {
                botpathDroppedLinks++;
            }
        }
    }

    return botpathNodeCount;
}

int32_t coduomp_botpath_dropped_links(void)
{
    return botpathDroppedLinks;
}

int32_t coduomp_botpath_invalid_links(void)
{
    return botpathInvalidLinks;
}

int32_t coduomp_botpath_nearest(const vec3_t pos)
{
    int32_t nearest = -1;
    float nearestDistance = 0.0f;
    int32_t index;

    for (index = 0; index < botpathNodeCount; index++) {
        float distance = VectorDistanceSquared(pos, botpathNodes[index].origin);

        if (nearest == -1 || distance < nearestDistance) {
            nearestDistance = distance;
            nearest = index;
        }
    }

    return nearest;
}

int32_t coduomp_botpath_next_path(int32_t start, int32_t goal,
                                  int32_t *outPath, int32_t *outLength)
{
    int32_t length = 0;

    if (outPath == NULL || outLength == NULL) {
        return 0;
    }
    if (botpathNodeCount == 0 || start < 0 || start >= botpathNodeCount) {
        return 0;
    }
    if (goal < 0 || goal >= botpathNodeCount || start == goal) {
        return 0;
    }

    if (!botpath_search_once(start, goal, outPath, &length)) {
        return 0;
    }

    /* A single-node route has no edge to charge, so another pass would hand
     * back the identical chain. */
    if (length == 0) {
        return 0;
    }

    if (botpathHasPrevious && botpathPreviousLength == length &&
        memcmp(botpathPrevious, outPath,
               sizeof(int32_t) * (size_t)length) == 0) {
        return 0;
    }

    memcpy(botpathPrevious, outPath, sizeof(int32_t) * (size_t)length);
    botpathPreviousLength = length;
    botpathHasPrevious = 1;

    botpath_penalize_route(outPath, length, start);

    *outLength = length;
    return 1;
}