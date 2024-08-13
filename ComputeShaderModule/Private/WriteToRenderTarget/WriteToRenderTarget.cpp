#include "WriteToRenderTarget.h"
#include "ComputeShaderModule/Public/WriteToRenderTarget/WriteToRenderTarget.h"

#include "PixelShaderUtils.h"
#include "RenderGraphResources.h"
#include "GlobalShader.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "RenderUtils.h"

// Declares a stats group to track performance metrics such as execution time and resource usage for debugging and optimization
DECLARE_STATS_GROUP(TEXT("WriteToRenderTarget"), STATGROUP_WriteToRenderTarget, STATCAT_Advanced);
// Defines a cycle stat to measure the time taken by the WriteToRenderTarget operation, useful for identifying performance bottlenecks
// "stat WriteToRenderTarget" will display the execution time in milliseconds in the console
DECLARE_CYCLE_STAT(TEXT("WriteToRenderTarget Execute"), STAT_WriteToRenderTarget_Execute, STATGROUP_WriteToRenderTarget);

class COMPUTESHADERMODULE_API FWriteToRenderTarget : public FGlobalShader
{
public:
    // Macros to declare this as a global shader and define parameter structure usage
    DECLARE_GLOBAL_SHADER(FWriteToRenderTarget);
    SHADER_USE_PARAMETER_STRUCT(FWriteToRenderTarget, FGlobalShader);

    // Permutation for different shader configurations
    class FWriteToRenderTarget_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
    using FPermutationDomain = TShaderPermutationDomain<FWriteToRenderTarget_Perm_TEST>;

    // Structure defining the parameters that will be passed to the shader
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_TEXTURE(Texture2D, InputTexture)
        SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, RenderTarget)
        // Color Change
        SHADER_PARAMETER(uint32, bInvertColors)
        SHADER_PARAMETER(uint32, bGreyscale)
        SHADER_PARAMETER(float, Contrast)
        // Deformations
        SHADER_PARAMETER(float, DistortionStrength)
        SHADER_PARAMETER(float, ImageScale)
        SHADER_PARAMETER(float, RotationAngle)
    END_SHADER_PARAMETER_STRUCT()

public:
    // Determines if this permutation should be compiled
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true; // Always compile this shader
    }

    // Modify the shader compilation environment
    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

        const FPermutationDomain PermutationVector(Parameters.PermutationId);

        // Constants that are used statically in the shader code
        OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_WriteToRenderTarget_X);
        OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_WriteToRenderTarget_Y);
        OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_WriteToRenderTarget_Z);
    }
};

IMPLEMENT_GLOBAL_SHADER(FWriteToRenderTarget, "/ComputeShaderModuleShaders/WriteToRenderTarget/WriteToRenderTarget.usf", "Main", SF_Compute);

// Color Change
uint32 FWriteToRenderTargetInterface::bInvertColors = 0;
uint32 FWriteToRenderTargetInterface::bGreyscale = 0;
float FWriteToRenderTargetInterface::Contrast = 1.0f;

// Deformations
float FWriteToRenderTargetInterface::DistortionStrength = 0.0f;
// Image Scale must start at 1.0f otherwise the image will be too small to see
float FWriteToRenderTargetInterface::ImageScale = 1.0f;
float FWriteToRenderTargetInterface::RotationAngle = 90.0f;

FRHICommandListImmediate* FWriteToRenderTargetInterface::StoredRHICmdList = nullptr;
UTexture2D* FWriteToRenderTargetInterface::StoredInputTexture = nullptr;
FWriteToRenderTargetDispatchParams FWriteToRenderTargetInterface::StoredParams;

void FWriteToRenderTargetInterface::Initialize(FRHICommandListImmediate& RHICmdList, UTexture2D* InputTexture, FWriteToRenderTargetDispatchParams Params)
{
    if (InputTexture && Params.RenderTarget)
    {
        StoredRHICmdList = &RHICmdList;
        StoredInputTexture = InputTexture;
        StoredParams = Params;
        UE_LOG(LogTemp, Warning, TEXT("FWriteToRenderTargetInterface::Initialize - Resources initialized successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FWriteToRenderTargetInterface::Initialize - Failed to initialize resources. InputTexture or RenderTarget is null."));
    }
}

void FWriteToRenderTargetInterface::SetInvertColors(bool bInvert)
{
    bInvertColors = bInvert ? 1 : 0;
    EnqueueShaderExecution();
}
void FWriteToRenderTargetInterface::SetGreyscale(bool bGrey)
{
    bGreyscale = bGrey ? 1 : 0;
    EnqueueShaderExecution();
}

void FWriteToRenderTargetInterface::SetContrast(float InContrast)
{
    Contrast = InContrast;
    EnqueueShaderExecution();
}

void FWriteToRenderTargetInterface::SetDistortionStrength(float InDistortionStrength)
{
    DistortionStrength = InDistortionStrength;
    EnqueueShaderExecution();
}

void FWriteToRenderTargetInterface::SetImageScale(float InImageScale)
{
    ImageScale = InImageScale;
    EnqueueShaderExecution();
}

void FWriteToRenderTargetInterface::SetRotationAngle(float InAngle)
{
    RotationAngle = InAngle;
    EnqueueShaderExecution();
}

void FWriteToRenderTargetInterface::EnqueueShaderExecution()
{
    // Enqueue the shader dispatch to be executed on the rendering thread
    if (StoredRHICmdList && StoredInputTexture && StoredParams.RenderTarget)
    {
        ENQUEUE_RENDER_COMMAND(ExecuteShader)(
            [](FRHICommandListImmediate& RHICmdList)
            {
                DispatchRenderThread(*StoredRHICmdList, StoredInputTexture, StoredParams);
            });
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("StoredRHICmdList %s, StoredInputTexture %s, StoredParams.RenderTarget %s"), StoredRHICmdList ? TEXT("true") : TEXT("false"), StoredInputTexture ? TEXT("true") : TEXT("false"), StoredParams.RenderTarget ? TEXT("true") : TEXT("false"));
    }
}

UTexture2D* FWriteToRenderTargetInterface::ResizeTexture(UTexture2D* SourceTexture, int32 TargetWidth, int32 TargetHeight)
{
    if (!SourceTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid SourceTexture."));
        return nullptr;
    }

    // Extract the texture's pixel data into an FImage
    FImage SourceImage;
    if (!FImageUtils::GetTexture2DSourceImage(SourceTexture, SourceImage))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to convert SourceTexture to FImage."));
        return nullptr;
    }

    // Prepare the destination array for resized image data
    TArray<FColor> ResizedColors;
    ResizedColors.SetNumUninitialized(TargetWidth * TargetHeight);

    // Resize the image using FImageUtils::ImageResize
    FImageUtils::ImageResize(SourceImage.SizeX, SourceImage.SizeY, SourceImage.AsBGRA8(), TargetWidth, TargetHeight, ResizedColors, true, true);

    // Create a new transient texture to hold the resized image
    UTexture2D* ResizedTexture = UTexture2D::CreateTransient(TargetWidth, TargetHeight, PF_B8G8R8A8);
    if (!ResizedTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ResizedTexture."));
        return nullptr;
    }

    // Lock the resized texture to update its pixel data
    void* TextureData = ResizedTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    if (!TextureData)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to lock ResizedTexture for writing."));
        return nullptr;
    }

    // Copy the resized pixel data into the new texture
    FMemory::Memcpy(TextureData, ResizedColors.GetData(), ResizedColors.Num() * sizeof(FColor));

    // Unlock and update the texture resource
    ResizedTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
    ResizedTexture->UpdateResource();
    
    return ResizedTexture;
}

void FWriteToRenderTargetInterface::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, UTexture2D* InputTexture, FWriteToRenderTargetDispatchParams Params)
{
    if (!StoredInputTexture)
    {
        return;
    }
    // Create a render graph builder which will manage the render graph for this dispatch
    FRDGBuilder GraphBuilder(RHICmdList);
    {
        // Measure the execution time of this block for performance profiling
        SCOPE_CYCLE_COUNTER(STAT_WriteToRenderTarget_Execute);
        // Declare a GPU stat for profiling purposes
        DECLARE_GPU_STAT(WriteToRenderTarget);
        // Mark a GPU event scope for debugging and profiling the shader dispatch process
        RDG_EVENT_SCOPE(GraphBuilder, "WriteToRenderTarget");
        // Mark a GPU stat scope for the render graph to assist in GPU performance profiling
        RDG_GPU_STAT_SCOPE(GraphBuilder, WriteToRenderTarget);
        // Define the permutation vector to manage different shader configurations
        typename FWriteToRenderTarget::FPermutationDomain PermutationVector;

        // Reference the compute shader using the global shader map for the current feature level
        TShaderMapRef<FWriteToRenderTarget> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);
        if (ComputeShader.IsValid()) 
        {
            // Get the RHI (Render Hardware Interface) texture reference from the input texture
            FTexture2DRHIRef InputTextureRHI = InputTexture->GetResource()->TextureRHI->GetTexture2D();

            // Allocate parameters for the shader pass
            FWriteToRenderTarget::FParameters* PassParameters = GraphBuilder.AllocParameters<FWriteToRenderTarget::FParameters>();
            PassParameters->InputTexture = InputTextureRHI;
            PassParameters->InputSampler = TStaticSamplerState<SF_Point>::GetRHI();

            // Color Change parameters
            PassParameters->bInvertColors = FWriteToRenderTargetInterface::bInvertColors;
            PassParameters->bGreyscale = FWriteToRenderTargetInterface::bGreyscale;
            PassParameters->Contrast = FWriteToRenderTargetInterface::Contrast;

            // Deformation parameters
            PassParameters->DistortionStrength = FWriteToRenderTargetInterface::DistortionStrength;
            PassParameters->ImageScale = FWriteToRenderTargetInterface::ImageScale;
            PassParameters->RotationAngle = FWriteToRenderTargetInterface::RotationAngle;

            // Describe the render target texture that will be created and used
            FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
                FIntPoint(InputTexture->GetSizeX(), InputTexture->GetSizeY()),  // Texture size
                PF_B8G8R8A8,  // Pixel format
                FClearValueBinding::White,  // Clear color
                TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV  // Texture flags
            );

            // Create the render target texture for the Render Dependency Graph (RDG) which schedules and manages rendering tasks
            // https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine?application_version=5.4
            FRDGTextureRef RenderTargetRDG = GraphBuilder.CreateTexture(Desc, TEXT("RenderTarget"));
            PassParameters->RenderTarget = GraphBuilder.CreateUAV(RenderTargetRDG);

            // Create a temporary texture to hold intermediate results
            FRDGTextureRef TmpTexture = GraphBuilder.CreateTexture(Desc, TEXT("WriteToRenderTarget_TempTexture"));
            
            // Register the external render target texture that will be used for output
            FRDGTextureRef TargetTexture = RegisterExternalTexture(GraphBuilder, Params.RenderTarget->GetRenderTargetTexture(), TEXT("WriteToRenderTarget_RT"));
            PassParameters->RenderTarget = GraphBuilder.CreateUAV(TmpTexture);

            // Calculate the group count for the compute shader dispatch based on the input dimensions
            auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z), FComputeShaderUtils::kGolden2DGroupSize);

            // Add a pass to the render graph to execute the compute shader
            GraphBuilder.AddPass(
                RDG_EVENT_NAME("ExecuteWriteToRenderTarget"),  // Event name for profiling
                PassParameters,  // Parameters for the shader pass
                ERDGPassFlags::AsyncCompute,  // Use asynchronous compute pipe for the pass
                [PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
                {
                    FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
                }
            );

            // Check if the target texture's format is compatible
            if (TargetTexture->Desc.Format == PF_B8G8R8A8)
            {
                // If compatible, copy the temporary texture to the target texture
                AddCopyTexturePass(GraphBuilder, TmpTexture, TargetTexture, FRHICopyTextureInfo());
            }
            else
            {
                #if WITH_EDITOR
                    // Display an error message in the editor if the texture format is incompatible
                    GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The provided render target has an incompatible format (Please change the RT format to: RGBA8).")));
                #endif
            }
        }
        else
        {
            #if WITH_EDITOR
                // Display an error message in the editor if the shader is invalid
                GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The compute shader has a problem.")));
            #endif
        }
    }

    // Execute the render graph to finalize the operations and process the shader execution
    GraphBuilder.Execute();
}
