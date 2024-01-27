/* Public domain */

#include <sys/param.h>
#include <sys/systm.h>

#include <video/cmdline.h>

char *
linux_video_get_options(const char *connector_name)
{
        char tunable[64];
	char *option;

        /*
         * A user may use loader tunables to set a specific mode for the
         * console. Tunables are read in the following order:
         *     1. kern.vt.fb.modes.$connector_name
         *     2. kern.vt.fb.default_mode
         *
         * Example of a mode specific to the LVDS connector:
         *     kern.vt.fb.modes.LVDS="1024x768"
         *
         * Example of a mode applied to all connectors not having a
         * connector-specific mode:
         *     kern.vt.fb.default_mode="640x480"
         */
        snprintf(tunable, sizeof(tunable), "kern.vt.fb.modes.%s",
            connector_name);
        if (bootverbose) {
                printf("[drm] Connector %s: get mode from tunables:\n", connector_name);
                printf("[drm]  - %s\n", tunable);
                printf("[drm]  - kern.vt.fb.default_mode\n");
        }
        option = kern_getenv(tunable);
        if (option == NULL)
                option = kern_getenv("kern.vt.fb.default_mode");
        
        return (option);
}
