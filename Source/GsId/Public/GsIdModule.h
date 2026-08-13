#pragma once

#include "Modules/ModuleManager.h"

class FGsIdModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
