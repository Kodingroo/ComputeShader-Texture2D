#include "System/ShaderModSettings.h"

void UShaderModSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Check if the changed property is bEnableShaderMod
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UShaderModSettings, bEnableShaderMod))
	{
		OnShaderModSettingsChanged.Broadcast();
	}
}
