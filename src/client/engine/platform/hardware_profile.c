#include "hardware_profile.h"


/* NOT_FROM_ORIGINAL_SOURCE: isolated improved hardware-policy interface. The
 * stock source does not reference it. It identifies the host CPU family instead of treating
 * every macOS renderer as equivalent.  The runtime sysctl also recognizes an
 * Apple-Silicon host when an x86_64 build is running through Rosetta. */
qboolean coduomp_is_apple_silicon(void)
{
    return qfalse;
}
