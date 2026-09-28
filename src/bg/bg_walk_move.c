#include "bg_pmove.h"

#include "compat/coduo_native_x87.h"
#include "math/q_math.h"


#include <math.h>
#include <stdint.h>


#define PM_WALK_WALLJUMP_HEIGHT_DELTA 18.0f
#define PM_WALK_WALLJUMP_NEAR_DAMP 0.65f
#define PM_WALK_WALLJUMP_AIR_DAMP 0.5f
#define PM_WALK_OVERBOUNCE 1.001f
#define PM_WALK_STAND_ACCELERATE 9.0f
#define PM_WALK_CROUCH_ACCELERATE 12.0f
#define PM_WALK_PRONE_ACCELERATE 19.0f
#define PM_WALK_SLICK_ACCELERATE 1.0f
#define PM_WALK_LAND_ACCELERATE_SCALE 0.25f

enum {
    PM_WALK_WALLJUMP_NEAR_TIME = 1800,
    PM_WALK_WALLJUMP_AIR_TIME = 1200
};

/*
 * Complete grounded movement step. The two authoritative Windows bodies are
 * instruction-identical apart from relocated globals and callees:
 *
 *   uo_cgame_mp_x86.dll  0x300091e0..0x30009652
 *   uo_game_mp_x86.dll   0x20008f90..0x20009402
 *
 * Linux game retains the same state machine at RVA 0x00024b7c. Its compiler
 * calls PM_ClipVelocity where Windows inlines it. The only behavior-relevant
 * realization differences are the wish-vector operand order, the spill of the
 * normalized wish length, the binary64 glibc-sqrt boundary, and the final dot
 * product order. Those expressions alone select a platform realization.
 */
void PM_WalkMove(void)
{
    playerState_t *ps = pm->ps;

    if ((ps->playerStateFlags & PMF_WALLJUMP) != 0) {
        /* NOT_FROM_ORIGINAL_SOURCE: jump_slowdownEnable 0 keeps the post-jump
         * window bookkeeping below but drops its velocity damping, so landing
         * and stepping out of a jump no longer cost 35% or 50% of the speed.
         * The PMF_WALLJUMP flag, pmTime window, and jumpOriginZ latch keep
         * their recovered lifetimes in both modes. */
        const qboolean dampJumpVelocity =
            bg_jumpSlowdownEnable.integer != 0 ? qtrue : qfalse;

        if (ps->pmTime > PM_WALK_WALLJUMP_NEAR_TIME) {
            ps->playerStateFlags &= ~(uint32_t)PMF_WALLJUMP;
            ps->jumpOriginZ = 0.0f;
            if (dampJumpVelocity != qfalse) {
                for (int32_t lane = 0; lane < 3; ++lane) {
                    ps->velocity[lane] = (float)(
                        (long double)ps->velocity[lane] *
                        (long double)PM_WALK_WALLJUMP_NEAR_DAMP);
                }
            }
        } else if (ps->pmTime == 0) {
            const qboolean nearGround =
                (long double)ps->psOrigin[2] <
                    (long double)ps->jumpOriginZ +
                        (long double)PM_WALK_WALLJUMP_HEIGHT_DELTA
                    ? qtrue : qfalse;
            const float damping = nearGround != qfalse
                                      ? PM_WALK_WALLJUMP_NEAR_DAMP
                                      : PM_WALK_WALLJUMP_AIR_DAMP;
            ps->pmTime = nearGround != qfalse
                             ? PM_WALK_WALLJUMP_NEAR_TIME
                             : PM_WALK_WALLJUMP_AIR_TIME;
            if (dampJumpVelocity != qfalse) {
                for (int32_t lane = 0; lane < 3; ++lane) {
                    ps->velocity[lane] = (float)(
                        (long double)ps->velocity[lane] *
                        (long double)damping);
                }
            }
        }
    }

    if (PM_CheckJump() != qfalse) {
        PM_AirMove();
        pm->ps->lastJumpCommandTime = pm->command.commandTime;
        return;
    }

    PM_Friction();

    const float forwardMove = (float)pm->command.forwardmove;
    const float rightMove = (float)pm->command.rightmove;
    usercmd_t command = pm->command;
    const float commandScale = PM_CmdScale_Walk(&command);

    /* MSVC inlines these two calls and folds Y/X/Z, whereas the retained
     * standalone Windows PM_ClipVelocity body folds Y/Z/X. Preserve the
     * caller's actual instruction graph rather than substituting the callee's
     * near-equivalent graph. It also reuses one normal.z*0 result across both
     * projections. */
    vec3_t *const bases[2] = {&pml.forward, &pml.right};
    const long double zeroNormalProduct =
        (long double)pml.groundTrace.normal[2] * 0.0L;
    for (int32_t basisIndex = 0; basisIndex < 2; ++basisIndex) {
        float *const basis = *bases[basisIndex];
        long double backoff =
            ((long double)pml.groundTrace.normal[1] *
                 (long double)basis[1] +
             (long double)pml.groundTrace.normal[0] *
                 (long double)basis[0]) +
            zeroNormalProduct;
        const float overbounce = backoff < 0.0L
                                     ? PM_WALK_OVERBOUNCE : 0.99900097f;
        backoff *= (long double)overbounce;
        basis[0] = (float)(
            (long double)basis[0] -
            (long double)pml.groundTrace.normal[0] * backoff);
        basis[1] = (float)(
            (long double)basis[1] -
            (long double)pml.groundTrace.normal[1] * backoff);
        basis[2] = (float)(
            -((long double)pml.groundTrace.normal[2] * backoff));
    }
    (void)VectorNormalize(pml.forward);
    (void)VectorNormalize(pml.right);

    vec3_t wishVelocity;
    for (int32_t lane = 0; lane < 3; ++lane) {
        wishVelocity[lane] = (float)(
            (long double)pml.right[lane] * (long double)rightMove +
            (long double)pml.forward[lane] * (long double)forwardMove);
    }

    vec3_t wishDirection = {
        wishVelocity[0], wishVelocity[1], wishVelocity[2]
    };

    const float wishSpeed = (float)(
        VectorNormalize(wishDirection) * (long double)commandScale);

    float acceleration;
    if ((pml.groundTrace.surfaceFlags & SURF_SLICK) != 0 ||
        (pm->ps->playerStateFlags & PMF_NO_GROUNDFRICTION) != 0) {
        acceleration = PM_WALK_SLICK_ACCELERATE;
    } else {
        switch (PM_GetEffectiveStance(pm->ps)) {
        case EFFECTIVE_STANCE_PRONE:
            acceleration = PM_WALK_PRONE_ACCELERATE;
            break;
        case EFFECTIVE_STANCE_CROUCH:
            acceleration = PM_WALK_CROUCH_ACCELERATE;
            break;
        default:
            acceleration = PM_WALK_STAND_ACCELERATE;
            break;
        }
    }

    if ((pm->ps->playerStateFlags & PMF_LAND_STUN) != 0) {
        acceleration = (float)(
            (long double)acceleration *
            (long double)PM_WALK_LAND_ACCELERATE_SCALE);
    }

    PM_Accelerate(wishDirection, wishSpeed, acceleration);

    ps = pm->ps;
    if ((pml.groundTrace.surfaceFlags & SURF_SLICK) != 0 ||
        (ps->playerStateFlags & PMF_NO_GROUNDFRICTION) != 0) {
        ps->velocity[2] = (float)(
            (long double)ps->velocity[2] -
            (long double)ps->gravity * (long double)pml.frametime);
    }

    vec3_t originalVelocity = {
        ps->velocity[0], ps->velocity[1], ps->velocity[2]
    };
    float speed;
    speed = (float)coduo_x87_sqrtl(
        ((long double)ps->velocity[0] * (long double)ps->velocity[0] +
         (long double)ps->velocity[1] * (long double)ps->velocity[1]) +
        (long double)ps->velocity[2] * (long double)ps->velocity[2]);

    /* This call is also inlined by MSVC and folds Z/Y/X. */
    long double velocityBackoff =
        ((long double)pml.groundTrace.normal[2] *
             (long double)ps->velocity[2] +
         (long double)pml.groundTrace.normal[1] *
             (long double)ps->velocity[1]) +
        (long double)pml.groundTrace.normal[0] *
            (long double)ps->velocity[0];
    const float velocityOverbounce = velocityBackoff < 0.0L
                                         ? PM_WALK_OVERBOUNCE : 0.99900097f;
    velocityBackoff *= (long double)velocityOverbounce;
    ps->velocity[0] = (float)(
        (long double)ps->velocity[0] -
        (long double)pml.groundTrace.normal[0] * velocityBackoff);
    ps->velocity[1] = (float)(
        (long double)ps->velocity[1] -
        (long double)pml.groundTrace.normal[1] * velocityBackoff);
    ps->velocity[2] = (float)(
        (long double)ps->velocity[2] -
        (long double)pml.groundTrace.normal[2] * velocityBackoff);

    const long double projection =
        ((long double)originalVelocity[2] * (long double)ps->velocity[2] +
         (long double)originalVelocity[1] * (long double)ps->velocity[1]) +
        (long double)originalVelocity[0] * (long double)ps->velocity[0];
    const qboolean preserveSpeed = projection > 0.0L ? qtrue : qfalse;

    if (preserveSpeed != qfalse) {
        (void)VectorNormalize(ps->velocity);
        for (int32_t lane = 0; lane < 3; ++lane) {
            ps->velocity[lane] = (float)(
                (long double)speed * (long double)ps->velocity[lane]);
        }
    }

    if (ps->velocity[0] != 0.0f || ps->velocity[1] != 0.0f) {
        PM_StepSlideMove(0);
    }
    PM_SetMovementDir();
}
