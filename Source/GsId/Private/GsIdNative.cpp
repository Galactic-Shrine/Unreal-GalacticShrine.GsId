#include "CoreTypes.h"

#if !PLATFORM_WINDOWS && !PLATFORM_LINUX && !PLATFORM_MAC
    #error "GalacticShrine.GsId.Cpp requires Windows, Linux or macOS."
#endif

#if PLATFORM_WINDOWS
    #include "Windows/AllowWindowsPlatformTypes.h"
#endif

// Compile the mandatory GalacticShrine.GsId.Cpp implementation as part of
// the Unreal GsId module. The native source remains isolated in ThirdParty.
#include "../../ThirdParty/GalacticShrine/GsId/src/gsid.cpp"

#if PLATFORM_WINDOWS
    #include "Windows/HideWindowsPlatformTypes.h"
#endif
