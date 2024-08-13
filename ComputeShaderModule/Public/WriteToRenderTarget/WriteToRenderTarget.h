#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"

#include "WriteToRenderTarget.generated.h"

/*
 * FWriteToRenderTargetDispatchParams defines the dimensions (X, Y, Z) for the shader execution and holds a reference to the render target.
 * This struct is essential for setting up the shader environment and ensuring proper execution on the GPU and render thread.
 */
struct COMPUTESHADERMODULE_API FWriteToRenderTargetDispatchParams
{
    int X;  // X dimension for dispatch
    int Y;  // Y dimension for dispatch
    int Z;  // Z dimension for dispatch

    FRenderTarget* RenderTarget;  // Pointer to the render target

    // Default constructor is required for the ENQUEUE_RENDER_COMMAND macro, otherwise it will not compile
    FWriteToRenderTargetDispatchParams()
        : X(0), Y(0), Z(0), RenderTarget(nullptr) {}

    // Constructor to initialize the dispatch parameters
    FWriteToRenderTargetDispatchParams(int x, int y, int z)
        : X(x), Y(y), Z(z), RenderTarget(nullptr) {}
};

/*
 * FWriteToRenderTargetInterface provides a public interface for executing compute shaders within the Unreal Engine framework.
 * It manages the interaction between the game thread and render thread, ensuring correct shader execution on the GPU.
 * The class handles tasks like shader dispatch, resource initialization, and texture resizing, making it essential for GPU-based operations.
 */
class COMPUTESHADERMODULE_API FWriteToRenderTargetInterface {
public:
    // Executes the shader on the render thread
    static void DispatchRenderThread(
        FRHICommandListImmediate& RHICmdList,  // Reference to the RHI command list for executing GPU commands
        UTexture2D* InputTexture,  // Pointer to the input texture
        FWriteToRenderTargetDispatchParams Params  // Parameters required for dispatching the shader
    );

    // Executes the shader on the render thread from the game thread via EnqueueRenderThreadCommand
    static void DispatchGameThread(
        UTexture2D* InputTexture,  // Pointer to the input texture
        FWriteToRenderTargetDispatchParams Params  // Parameters required for dispatching the shader
    )
    {
        // Enqueues a render command to be executed on the render thread
        ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
        [InputTexture, Params](FRHICommandListImmediate& RHICmdList)
        {
            DispatchRenderThread(RHICmdList, InputTexture, Params);  // Calls the render thread dispatch function
        });
    }

    // Dispatches the shader. Can be called from any thread
    static void Dispatch(
        UTexture2D* InputTexture,  // Pointer to the input texture
        FWriteToRenderTargetDispatchParams Params  // Parameters required for dispatching the shader
    )
    {
        // Checks if the current thread is the rendering thread
        if (IsInRenderingThread()) {
            // If in rendering thread, dispatch directly
            DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), InputTexture, Params);
        } else {
            // Otherwise, enqueue the render command to be executed on the render thread
            DispatchGameThread(InputTexture, Params);
        }
    }

    static void Initialize(FRHICommandListImmediate& RHICmdList, UTexture2D* InputTexture,
                           FWriteToRenderTargetDispatchParams Params);
    
    // Color Change
    static void SetInvertColors(bool bInvert);
    static void SetGreyscale(bool bGrey);
    static void SetContrast(float Contrast);
    // Deformations
    static void SetDistortionStrength(float Distortion);
    static void SetImageScale(float Scale);
    static void EnqueueShaderExecution();
    static void SetRotationAngle(float Angle);
    
    static UTexture2D* ResizeTexture(UTexture2D* SourceTexture, int32 TargetWidth, int32 TargetHeight);

private:
    // Color change parameters
    static uint32 bInvertColors;
    static uint32 bGreyscale;
    static float Contrast;

    // Deformation parameters
    static float DistortionStrength;
    static float ImageScale;
    static float RotationAngle;
    
    static FRHICommandListImmediate* StoredRHICmdList;
    static UTexture2D* StoredInputTexture;
    static FWriteToRenderTargetDispatchParams StoredParams;
};

// This is a static blueprint library that can be used to invoke our compute shader from blueprints.
UCLASS()
class COMPUTESHADERMODULE_API UWriteToRenderTargetLibrary : public UObject
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintCallable)
    static void ExecuteRTComputeShader(UTexture2D* InputTexture, UTextureRenderTarget2D* RT)
    {
        if (!InputTexture || !RT)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid input texture or render target."));
            return;
        }

        // Resize texture on the game thread
        UTexture2D* ResizedTexture = InputTexture;
        if (InputTexture->GetSizeX() != 512 || InputTexture->GetSizeY() != 512)
        {
            ResizedTexture = FWriteToRenderTargetInterface::ResizeTexture(InputTexture, 512, 512);
            if (!ResizedTexture)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to resize texture."));
                return;
            }
        }

        // Initialize shader resources on the game thread
        FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
        FWriteToRenderTargetDispatchParams Params(RT->SizeX, RT->SizeY, 1);
        Params.RenderTarget = RT->GameThread_GetRenderTargetResource();

        // Initialize the shader resources before dispatching is necessary
        // as the shader resources are not available on the render thread
        FWriteToRenderTargetInterface::Initialize(RHICmdList, ResizedTexture, Params);

        // Enqueue the dispatch to run on the render thread
        ENQUEUE_RENDER_COMMAND(ExecuteShader)(
            [ResizedTexture, Params](FRHICommandListImmediate& RHICmdList)
            {
                FWriteToRenderTargetInterface::DispatchRenderThread(RHICmdList, ResizedTexture, Params);
            });
    }
};
