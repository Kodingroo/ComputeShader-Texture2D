#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "ShaderParameterMacros.h"
#include "WriteToRenderTarget.generated.h"

#define NUM_THREADS_WriteToRenderTarget_X 32
#define NUM_THREADS_WriteToRenderTarget_Y 32
#define NUM_THREADS_WriteToRenderTarget_Z 1

/*
 * FWriteToRenderTargetDispatchParams defines the dimensions (X, Y, Z) for the shader execution and holds a reference to the render target.
 * This struct is essential for setting up the shader environment and ensuring proper execution on the GPU and render thread.
 */
struct COMPUTESHADERMODULE_API FWriteToRenderTargetDispatchParams
{
    int X; 
    int Y; 
    int Z; 
    FRenderTarget* RenderTarget;

    // Default constructor is required for the ENQUEUE_RENDER_COMMAND macro, otherwise it will not compile
    FWriteToRenderTargetDispatchParams()
        : X(0), Y(0), Z(0), RenderTarget(nullptr) {}

    // Constructor to initialize the dispatch parameters
    FWriteToRenderTargetDispatchParams(int x, int y, int z)
        : X(x), Y(y), Z(z), RenderTarget(nullptr) {}
};

UCLASS()
class COMPUTESHADERMODULE_API UWriteToRenderTarget : public UObject
{
    GENERATED_BODY()

public:
    void DispatchRenderThread(
        FRHICommandListImmediate& RHICmdList,
        UTexture2D* InputTexture,
        FWriteToRenderTargetDispatchParams Params
    );

    void DispatchGameThread(
        UTexture2D* InputTexture,
        FWriteToRenderTargetDispatchParams Params
    );

    void Dispatch(
        UTexture2D* InputTexture,
        FWriteToRenderTargetDispatchParams Params
    );

    /*
     * Initializes the shader parameters and stores them for use in subsequent shader dispatches.
     * This function is critical for setting up the shader environment with the correct input texture and render target.
     */
    void Initialize(FRHICommandListImmediate& RHICmdList, UTexture2D* InputTexture,
                    FWriteToRenderTargetDispatchParams Params);

	// Color change
    void SetInvertColors(bool bInvert);
    void SetGreyscale(bool bGrey);
    void SetContrast(float Contrast);
    // Deformation
    void SetDistortionStrength(float Distortion);
    void SetImageScale(float Scale);
    void SetRotationAngle(float Angle);

    /*
     * Resizes the input texture to the specified dimensions.
     * This function ensures that the input texture has the correct dimensions for processing
     */
    UTexture2D* ResizeTexture(UTexture2D* SourceTexture, int32 TargetWidth, int32 TargetHeight);

    void EnqueueShaderExecution();

    // Shader parameters for image processing, initialized with default values
	// Color change
	uint32 bInvertColors = 0;         // Whether to invert colors (0 = false, 1 = true)
    uint32 bGreyscale = 0;            // Whether to convert to grayscale (0 = false, 1 = true)
    float Contrast = 1.0f;            
	// Deformation
    float DistortionStrength = 0.0f;  
    float ImageScale = 1.0f;          // Scaling factor for the image (1.0 = 100%)
    float RotationAngle = 90.0f;      // Rotation angle in degrees (default 90 degrees)
    
private:
    UPROPERTY()
    UTexture2D* StoredInputTexture;  
    FRHICommandListImmediate* StoredRHICmdList;  
    FWriteToRenderTargetDispatchParams StoredParams;  
};
