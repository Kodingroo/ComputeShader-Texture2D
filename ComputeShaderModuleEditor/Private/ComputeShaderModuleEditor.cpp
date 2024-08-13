#include "ComputeShaderModuleEditor.h"
#include "ISettingsModule.h"
#include "System/ShaderModSettings.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Widgets/Docking/SDockTab.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "FComputeShaderModuleEditor"

void FComputeShaderModuleEditor::StartupModule()
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->RegisterSettings("Editor", "Corpy & Co", "ShaderMod",
            LOCTEXT("ShaderModSettingsName", "Shader Mod Settings"),
            LOCTEXT("ShaderModSettingsDescription", "Configure the Shader Mod settings"),
            GetMutableDefault<UShaderModSettings>()
        );
    }

    UShaderModSettings* ShaderModSettings = GetMutableDefault<UShaderModSettings>();
    if (ShaderModSettings)
    {
        ShaderModSettings->OnShaderModSettingsChanged.AddRaw(this, &FComputeShaderModuleEditor::OnSettingsChanged);

        // Register for post-world initialization, so we can show the Shader Mod utility to the user immediately if bEnableShaderMod is true
        FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FComputeShaderModuleEditor::OnPostWorldInitialization);
    }

    FCoreDelegates::OnEnginePreExit.AddRaw(this, &FComputeShaderModuleEditor::PreExitCleanup);

}

void FComputeShaderModuleEditor::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
    UShaderModSettings* ShaderModSettings = GetMutableDefault<UShaderModSettings>();
    if (ShaderModSettings)
    {
        if (ShaderModSettings->bEnableShaderMod)
        {
            ShowShaderModUtility();
        }
    }
}

void FComputeShaderModuleEditor::PreExitCleanup() const
{
    // Cleanup that involves settings should be done here
    UShaderModSettings* ShaderModSettings = GetMutableDefault<UShaderModSettings>();
    if (ShaderModSettings)
    {
        ShaderModSettings->OnShaderModSettingsChanged.RemoveAll(this);
    }

    // Remove other delegates here if needed
    if (FWorldDelegates::OnPostWorldInitialization.IsBoundToObject(this))
    {
        FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
    }

    // Unregister settings and other final cleanup steps
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Editor", "Corpy & Co", "ShaderMod");
    }
}

void FComputeShaderModuleEditor::ShutdownModule()
{
    // Minimal cleanup, just logging
    UE_LOG(LogTemp, Warning, TEXT("ShutdownModule: Completed shutdown cleanup"));
}

// Show the Shader Mod Utility widget to the user in a new tab
void FComputeShaderModuleEditor::ShowShaderModUtility()
{
    UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();

    if (!EditorUtilitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get EditorUtilitySubsystem."));
        return;
    }

    UBlueprint* UMGBP = LoadObject<UBlueprint>(nullptr, TEXT("/ShaderMod/UI/W_ShaderModUtility.W_ShaderModUtility"));
    UEditorUtilityWidgetBlueprint* EditorWidget = Cast<UEditorUtilityWidgetBlueprint>(UMGBP);

    if (EditorWidget)
    {
        EditorUtilitySubsystem->SpawnAndRegisterTab(EditorWidget);
    }
}

void FComputeShaderModuleEditor::OnSettingsChanged()
{
    UShaderModSettings* ShaderModSettings = GetMutableDefault<UShaderModSettings>();
    if (ShaderModSettings)
    {
        if (ShaderModSettings->bEnableShaderMod)
        {
            ShowShaderModUtility();
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FComputeShaderModuleEditor, ComputeShaderModuleEditor)
