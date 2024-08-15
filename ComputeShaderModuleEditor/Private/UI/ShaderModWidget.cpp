#include "UI/ShaderModWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "WriteToRenderTarget/WriteToRenderTarget.h"
#include "WriteToRenderTarget/WriteToRenderTargetLibrary.h"

void UShaderModWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (CheckBox_InvertColors)
    {
        CheckBox_InvertColors->OnCheckStateChanged.AddDynamic(this, &UShaderModWidget::OnInvertColorsChanged);
    }

    if (CheckBox_Grayscale)
    {
        CheckBox_Grayscale->OnCheckStateChanged.AddDynamic(this, &UShaderModWidget::OnGrayscaleChanged);
    }

    if (Slider_Contrast)
    {
        Slider_Contrast->OnValueChanged.AddDynamic(this, &UShaderModWidget::OnContrastChanged);
    }

    if (Slider_Distortion)
    {
        Slider_Distortion->OnValueChanged.AddDynamic(this, &UShaderModWidget::OnDistortionChanged);
    }

    if (Slider_Scaling)
    {
        Slider_Scaling->OnValueChanged.AddDynamic(this, &UShaderModWidget::OnScalingChanged);
    }

    if (Slider_Rotation)
    {
        Slider_Rotation->OnValueChanged.AddDynamic(this, &UShaderModWidget::OnRotationChanged);
    }
    
    if (Button_Reset)
    {
        Button_Reset->OnClicked.AddDynamic(this, &UShaderModWidget::OnResetClicked);
    }
}

void UShaderModWidget::OnInvertColorsChanged(bool bIsChecked)
{
    if (!WriteToRenderTargetInstance) { CheckWriteToRenderTargetInstance(); }
    
    WriteToRenderTargetInstance->SetInvertColors(bIsChecked);
}

void UShaderModWidget::OnGrayscaleChanged(bool bIsChecked)
{
    if (!WriteToRenderTargetInstance) { CheckWriteToRenderTargetInstance(); }

    WriteToRenderTargetInstance->SetGreyscale(bIsChecked);
}

void UShaderModWidget::OnContrastChanged(float Value)
{
    if (!WriteToRenderTargetInstance) { CheckWriteToRenderTargetInstance(); }

    WriteToRenderTargetInstance->SetContrast(Value);
}

void UShaderModWidget::OnDistortionChanged(float Value)
{
    if (!WriteToRenderTargetInstance) { CheckWriteToRenderTargetInstance(); }

    WriteToRenderTargetInstance->SetDistortionStrength(Value);
}

void UShaderModWidget::OnScalingChanged(float Value)
{
    if (!WriteToRenderTargetInstance) { CheckWriteToRenderTargetInstance(); }

    WriteToRenderTargetInstance->SetImageScale(Value);
}

void UShaderModWidget::OnRotationChanged(float Value)
{
    if (!WriteToRenderTargetInstance) { CheckWriteToRenderTargetInstance(); }

    WriteToRenderTargetInstance->SetRotationAngle(Value);
}

void UShaderModWidget::OnResetClicked()
{
    ResetShaderParameters();
}

void UShaderModWidget::ResetShaderParameters()
{
    // Reset all parameters to their default values
    CheckBox_InvertColors->SetIsChecked(false);
    CheckBox_Grayscale->SetIsChecked(false);
    Slider_Contrast->SetValue(1.0f);
    Slider_Distortion->SetValue(0.0f);
    Slider_Scaling->SetValue(1.0f);
    Slider_Rotation->SetValue(90.0f);

    OnInvertColorsChanged(false);
    OnGrayscaleChanged(false);
    OnContrastChanged(1.0f);
    OnDistortionChanged(0.0f);
    OnScalingChanged(1.0f);
    OnRotationChanged(90.0f);
}

void UShaderModWidget::CheckWriteToRenderTargetInstance()
{
    if (UWriteToRenderTarget* ActiveInstance = UWriteToRenderTargetLibrary::WriteToRenderTargetInstance)
    {
        WriteToRenderTargetInstance = ActiveInstance;
    }
}
