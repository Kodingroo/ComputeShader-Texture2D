#pragma once

#include "CoreMinimal.h"
#include "ShaderModSettings.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnShaderModSettingsChanged);

/*
 * Is a configuration class enabling the dynamic display of the Shader Mod Utility Widget
 */

UCLASS(config = EditorPerProjectUserSettings)
class COMPUTESHADERMODULEEDITOR_API UShaderModSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "Shader Mod")
	bool bEnableShaderMod;

	// Delegate to notify when settings change
	FOnShaderModSettingsChanged OnShaderModSettingsChanged;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
