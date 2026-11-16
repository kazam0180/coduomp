#include "q_math.h"

#include "qcommon/q_shared_types.h"

#include <signal.h>
#include <stdint.h>


enum {
    BOX_PLANE_SIGNBITS_COUNT = 8,
    BOX_PLANE_INVALID_READ_ADDRESS = 1
};

void Com_Error(errorParm_t code, const char *format, ...);

/*
 * The eight valid paths are instruction-identical, apart from relocations,
 * in all three authoritative Windows owners and in the Linux engine:
 *
 *   CoDUOMP.exe            0x004346e0..0x00434915
 *   uo_cgame_mp_x86.dll    0x3004c840..0x3004ca75
 *   uo_ui_mp_x86.dll       0x40004850..0x40004a85
 *   coduo_lnxded           0x080cb294..0x080cb467
 *
 * Each front dot is (x+y)+z, each back dot is z+(x+y), and both remain live
 * in x87 registers through their comparisons.  The process control word is
 * the only valid-input Windows/Linux precision difference.
 */
typedef long double box_plane_distance_t;
#define BOX_FRONT_DOT(cx, cy, cz)                                            \
    (((long double)plane->normal[0] * (long double)(cx) +                   \
      (long double)plane->normal[1] * (long double)(cy)) +                  \
     (long double)plane->normal[2] * (long double)(cz))
#define BOX_BACK_DOT(cx, cy, cz)                                             \
    ((long double)plane->normal[2] * (long double)(cz) +                    \
     ((long double)plane->normal[0] * (long double)(cx) +                   \
      (long double)plane->normal[1] * (long double)(cy)))

int32_t BoxOnPlaneSide(const vec3_t mins, const vec3_t maxs,
                       const cplane_t *plane)
{
    box_plane_distance_t frontDistance;
    box_plane_distance_t backDistance;
    int32_t sides = 0;

    if (plane->signbits >= BOX_PLANE_SIGNBITS_COUNT) {
        /* Preserve this recovered boundary's validated input, state, and compatibility invariants. */
#if   defined(__i386__) || defined(__x86_64__)
        __asm__ volatile("int3");
#else
        /* NOT_FROM_ORIGINAL_SOURCE: portable equivalent of the x86 INT3. */
        raise(SIGTRAP);
#endif
        Com_Error(ERR_DROP,
                  "\x15" "BoxOnPlaneSide: invalid signbits for plane");
#if   defined(__i386__) || defined(__x86_64__)
        __asm__ volatile("int3");
        __builtin_unreachable();
#else
        raise(SIGTRAP);
        __builtin_unreachable();
#endif
    }

    switch (plane->signbits) {
    case 0:
        frontDistance = BOX_FRONT_DOT(maxs[0], maxs[1], maxs[2]);
        backDistance = BOX_BACK_DOT(mins[0], mins[1], mins[2]);
        break;
    case 1:
        frontDistance = BOX_FRONT_DOT(mins[0], maxs[1], maxs[2]);
        backDistance = BOX_BACK_DOT(maxs[0], mins[1], mins[2]);
        break;
    case 2:
        frontDistance = BOX_FRONT_DOT(maxs[0], mins[1], maxs[2]);
        backDistance = BOX_BACK_DOT(mins[0], maxs[1], mins[2]);
        break;
    case 3:
        frontDistance = BOX_FRONT_DOT(mins[0], mins[1], maxs[2]);
        backDistance = BOX_BACK_DOT(maxs[0], maxs[1], mins[2]);
        break;
    case 4:
        frontDistance = BOX_FRONT_DOT(maxs[0], maxs[1], mins[2]);
        backDistance = BOX_BACK_DOT(mins[0], mins[1], maxs[2]);
        break;
    case 5:
        frontDistance = BOX_FRONT_DOT(mins[0], maxs[1], mins[2]);
        backDistance = BOX_BACK_DOT(maxs[0], mins[1], maxs[2]);
        break;
    case 6:
        frontDistance = BOX_FRONT_DOT(maxs[0], mins[1], mins[2]);
        backDistance = BOX_BACK_DOT(mins[0], maxs[1], maxs[2]);
        break;
    case 7:
    default:
        frontDistance = BOX_FRONT_DOT(mins[0], mins[1], mins[2]);
        backDistance = BOX_BACK_DOT(maxs[0], maxs[1], maxs[2]);
        break;
    }

    if (frontDistance >= (long double)plane->dist) {
        sides = BOX_ON_PLANE_SIDE_FRONT;
    }
    if (!(backDistance >= (long double)plane->dist)) {
        sides |= BOX_ON_PLANE_SIDE_BACK;
    }
    return sides;
}

#undef BOX_BACK_DOT
#undef BOX_FRONT_DOT
