#pragma once

#include "Modules/ModuleManager.h"

class FGsIdEditorModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
