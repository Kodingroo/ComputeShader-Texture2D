#include "WriteToRenderTarget/WriteToRenderTargetLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "WriteToRenderTarget/WriteToRenderTarget.h"

UWriteToRenderTarget* UWriteToRenderTargetLibrary::WriteToRenderTargetInstance = nullptr;

/*
 * Executes the compute shader by ensuring that the texture is processed, resized if necessary,
 * and then dispatched to run on the render thread with the current shader parameters.
 */
void UWriteToRenderTargetLibrary::ExecuteRTComputeShader(UTexture2D* InputTexture, UTextureRenderTarget2D* RT)
{
    if (!InputTexture || !RT)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid input texture or render target."));
        return;
    }

    // If the singleton instance already exists, apply the current shader parameters.
    if (WriteToRenderTargetInstance)
    {
        // Reapply shader parameters to the instance before execution
        WriteToRenderTargetInstance->SetInvertColors(WriteToRenderTargetInstance->bInvertColors == 1);
        WriteToRenderTargetInstance->SetGreyscale(WriteToRenderTargetInstance->bGreyscale == 1);
        WriteToRenderTargetInstance->SetContrast(WriteToRenderTargetInstance->Contrast);
        WriteToRenderTargetInstance->SetDistortionStrength(WriteToRenderTargetInstance->DistortionStrength);
        WriteToRenderTargetInstance->SetImageScale(WriteToRenderTargetInstance->ImageScale);
        WriteToRenderTargetInstance->SetRotationAngle(WriteToRenderTargetInstance->RotationAngle);
    }
    else
    {
        // Create a new instance if it doesn't exist
        WriteToRenderTargetInstance = NewObject<UWriteToRenderTarget>();
        UE_LOG(LogTemp, Warning, TEXT("WriteToRenderTargetInstance created."));
    }

    // Resize the texture if its dimensions do not match the render target's dimensions
    UTexture2D* ResizedTexture = InputTexture;
    if (InputTexture->GetSizeX() != RT->SizeX || InputTexture->GetSizeY() != RT->SizeY)
    {
        ResizedTexture = WriteToRenderTargetInstance->ResizeTexture(InputTexture, RT->SizeX, RT->SizeY);
        if (!ResizedTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to resize texture."));
            return;
        }
    }

    FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
    FWriteToRenderTargetDispatchParams Params(RT->SizeX, RT->SizeY, 1);
    Params.RenderTarget = RT->GameThread_GetRenderTargetResource();

    // Initialize the shader resources before dispatching is necessary
    // as the shader resources are not available on the render thread
    WriteToRenderTargetInstance->Initialize(RHICmdList, ResizedTexture, Params);

    // Enqueue the shader execution on the render thread
    ENQUEUE_RENDER_COMMAND(ExecuteShader)(
        [ResizedTexture, Params](FRHICommandListImmediate& RHICmdList)
        {
            WriteToRenderTargetInstance->DispatchRenderThread(RHICmdList, ResizedTexture, Params);
        });
}
