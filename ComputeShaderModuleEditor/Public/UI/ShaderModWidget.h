#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "ShaderModWidget.generated.h"

/*
 * Is an editor utility widget that provides a user interface for controlling the parameters of the compute shader.
 * Includes checkboxes and sliders for modifying color and deformation effects.
 */

UCLASS()
class COMPUTESHADERMODULEEDITOR_API UShaderModWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UCheckBox* CheckBox_InvertColors;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* CheckBox_Grayscale;

	UPROPERTY(meta = (BindWidget))
	class USlider* Slider_Contrast;

	UPROPERTY(meta = (BindWidget))
	class USlider* Slider_Distortion;

	UPROPERTY(meta = (BindWidget))
	class USlider* Slider_Scaling;

	UPROPERTY(meta = (BindWidget))
	class USlider* Slider_Rotation;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Reset;

	UFUNCTION()
	void OnInvertColorsChanged(bool bIsChecked);

	UFUNCTION()
	void OnGrayscaleChanged(bool bIsChecked);

	UFUNCTION()
	void OnContrastChanged(float Value);

	UFUNCTION()
	void OnDistortionChanged(float Value);

	UFUNCTION()
	void OnScalingChanged(float Value);

	UFUNCTION()
	void OnRotationChanged(float Value);
	
	UFUNCTION()
	void OnResetClicked();

private:
	void ResetShaderParameters(); 
};
