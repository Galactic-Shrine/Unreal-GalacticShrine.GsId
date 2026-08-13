#include "GsIdEditorModule.h"

#include "Localization/GsIdEditorLocalizedMetadata.h"

#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FGsIdEditorModule, GsIdEditor)

void FGsIdEditorModule::StartupModule()
{
    GsIdEditorLocalization::StartupLocalizedMetadata();
}

void FGsIdEditorModule::ShutdownModule()
{
    GsIdEditorLocalization::ShutdownLocalizedMetadata();
}
