#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * This Editor module contains code that is only relevant when the game or application is being developed
 * Has the Utility Widget and the settings for the Compute Shader Module
 */

class FComputeShaderModuleEditor : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void OnPostWorldInitialization(UWorld* World, UWorld::InitializationValues IVS);
    void PreExitCleanup() const;
    void OnSettingsChanged();
    static void ShowShaderModUtility();

};
