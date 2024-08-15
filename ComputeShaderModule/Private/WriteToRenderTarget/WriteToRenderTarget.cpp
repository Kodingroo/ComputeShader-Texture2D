#include "WriteToRenderTarget/WriteToRenderTarget.h"
#include "RenderGraphBuilder.h"
#include "RHIResources.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterUtils.h"
#include "Engine/Texture2D.h"
#include "PixelShaderUtils.h"
#include "GlobalShader.h"
#include "ImageUtils.h"

// Stat declarations for profiling and performance monitoring
DECLARE_STATS_GROUP(TEXT("WriteToRenderTarget"), STATGROUP_WriteToRenderTarget, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("WriteToRenderTarget Execute"), STAT_WriteToRenderTarget_Execute, STATGROUP_WriteToRenderTarget);

// This class represents the global shader used to write to a render target
class FWriteToRenderTarget : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FWriteToRenderTarget);
    SHADER_USE_PARAMETER_STRUCT(FWriteToRenderTarget, FGlobalShader);

    // Define a permutation domain for shader configuration
    class FWriteToRenderTarget_Perm_TEST : SHADER_PERMUTATION_INT("TEST", 1);
    using FPermutationDomain = TShaderPermutationDomain<FWriteToRenderTarget_Perm_TEST>;

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_TEXTURE(Texture2D, InputTexture) // The input texture to be processed
        SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler) // Sampler state for the input texture
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, RenderTarget) // The render target to output the processed texture
        // Color change
        SHADER_PARAMETER(uint32, bInvertColors) // Boolean parameter for inverting colors
        SHADER_PARAMETER(uint32, bGreyscale) // Boolean parameter for applying grayscale
        SHADER_PARAMETER(float, Contrast) // Float parameter for contrast adjustment
        // Deformation
        SHADER_PARAMETER(float, DistortionStrength) // Float parameter for distortion strength
        SHADER_PARAMETER(float, ImageScale) // Float parameter for image scaling
        SHADER_PARAMETER(float, RotationAngle) // Float parameter for image rotation
    END_SHADER_PARAMETER_STRUCT()

    // This function determines whether the shader permutation should be compiled
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return true;
    }

    // This function modifies the shader compilation environment by setting constants and configurations
    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.SetDefine(TEXT("THREADS_X"), NUM_THREADS_WriteToRenderTarget_X);
        OutEnvironment.SetDefine(TEXT("THREADS_Y"), NUM_THREADS_WriteToRenderTarget_Y);
        OutEnvironment.SetDefine(TEXT("THREADS_Z"), NUM_THREADS_WriteToRenderTarget_Z);
    }
};

// Implementation of the global shader              // Shader file path                // Entry point function name  // Shader function (Compute)
IMPLEMENT_GLOBAL_SHADER(FWriteToRenderTarget, "/ComputeShaderModuleShaders/WriteToRenderTarget/WriteToRenderTarget.usf", "Main", SF_Compute);

/*
 * Initializes the resources necessary during shader execution.
 */
void UWriteToRenderTarget::Initialize(FRHICommandListImmediate& RHICmdList, UTexture2D* InputTexture, FWriteToRenderTargetDispatchParams Params)
{
    if (InputTexture && Params.RenderTarget)
    {
        StoredRHICmdList = &RHICmdList;
        StoredInputTexture = InputTexture;
        StoredParams = Params;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Initialize - Failed to initialize resources. InputTexture or RenderTarget is null."));
    }
}

void UWriteToRenderTarget::SetInvertColors(bool bInvert)
{
    bInvertColors = bInvert ? 1 : 0;
    EnqueueShaderExecution();
}

void UWriteToRenderTarget::SetGreyscale(bool bGrey)
{
    bGreyscale = bGrey ? 1 : 0;
    EnqueueShaderExecution();
}

void UWriteToRenderTarget::SetContrast(float InContrast)
{
    Contrast = InContrast;
    EnqueueShaderExecution();
}

void UWriteToRenderTarget::SetDistortionStrength(float InDistortionStrength)
{
    DistortionStrength = InDistortionStrength;
    EnqueueShaderExecution();
}

void UWriteToRenderTarget::SetImageScale(float InImageScale)
{
    ImageScale = InImageScale;
    EnqueueShaderExecution();
}

void UWriteToRenderTarget::SetRotationAngle(float InAngle)
{
    RotationAngle = InAngle;
    EnqueueShaderExecution();
}

UTexture2D* UWriteToRenderTarget::ResizeTexture(UTexture2D* SourceTexture, int32 TargetWidth, int32 TargetHeight)
{
    if (!SourceTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid SourceTexture."));
        return nullptr;
    }
    
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

/*
 * Enqueues the shader execution command on the render thread. This function checks if the necessary resources
 * are available and then enqueues the shader to be executed using the stored parameters.
 */
void UWriteToRenderTarget::EnqueueShaderExecution()
{
    if (StoredRHICmdList && StoredInputTexture && StoredParams.RenderTarget)
    {
        ENQUEUE_RENDER_COMMAND(ExecuteShader)(
            [this](FRHICommandListImmediate& RHICmdList)
            {
                DispatchRenderThread(*StoredRHICmdList, StoredInputTexture, StoredParams);
            });
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("StoredRHICmdList %s, StoredInputTexture %s, StoredParams.RenderTarget %s"), 
            StoredRHICmdList ? TEXT("true") : TEXT("false"), 
            StoredInputTexture ? TEXT("true") : TEXT("false"), 
            StoredParams.RenderTarget ? TEXT("true") : TEXT("false"));
    }
}

/*
 * This function executes the shader on the render thread. It builds the render graph, allocates
 * the necessary parameters, and dispatches the compute shader to process the input texture and
 * write to the render target.
 */
void UWriteToRenderTarget::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, UTexture2D* InputTexture, FWriteToRenderTargetDispatchParams Params)
{
    if (!StoredInputTexture)
    {
        return;
    }
    
    FRDGBuilder GraphBuilder(RHICmdList);
    {
        SCOPE_CYCLE_COUNTER(STAT_WriteToRenderTarget_Execute);
        DECLARE_GPU_STAT(WriteToRenderTarget);
        RDG_EVENT_SCOPE(GraphBuilder, "WriteToRenderTarget");
        RDG_GPU_STAT_SCOPE(GraphBuilder, WriteToRenderTarget);

        FWriteToRenderTarget::FPermutationDomain PermutationVector;
        TShaderMapRef<FWriteToRenderTarget> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel), PermutationVector);
        if (ComputeShader.IsValid()) 
        {
            FTexture2DRHIRef InputTextureRHI = InputTexture->GetResource()->TextureRHI->GetTexture2D();
            FWriteToRenderTarget::FParameters* PassParameters = GraphBuilder.AllocParameters<FWriteToRenderTarget::FParameters>();
            PassParameters->InputTexture = InputTextureRHI;
            PassParameters->InputSampler = TStaticSamplerState<SF_Point>::GetRHI();
            // Color change
            PassParameters->bInvertColors = bInvertColors;
            PassParameters->bGreyscale = bGreyscale;
            PassParameters->Contrast = Contrast;
            // Deformation
            PassParameters->DistortionStrength = DistortionStrength;
            PassParameters->ImageScale = ImageScale;
            PassParameters->RotationAngle = RotationAngle;

            FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
                FIntPoint(InputTexture->GetSizeX(), InputTexture->GetSizeY()),
                PF_B8G8R8A8,
                FClearValueBinding::White,
                TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV
            );

            FRDGTextureRef RenderTargetRDG = GraphBuilder.CreateTexture(Desc, TEXT("RenderTarget"));
            PassParameters->RenderTarget = GraphBuilder.CreateUAV(RenderTargetRDG);
            FRDGTextureRef TmpTexture = GraphBuilder.CreateTexture(Desc, TEXT("WriteToRenderTarget_TempTexture"));
            FRDGTextureRef TargetTexture = RegisterExternalTexture(GraphBuilder, Params.RenderTarget->GetRenderTargetTexture(), TEXT("WriteToRenderTarget_RT"));
            PassParameters->RenderTarget = GraphBuilder.CreateUAV(TmpTexture);

            auto GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(Params.X, Params.Y, Params.Z), FComputeShaderUtils::kGolden2DGroupSize);

            GraphBuilder.AddPass(
                RDG_EVENT_NAME("ExecuteWriteToRenderTarget"),
                PassParameters,
                ERDGPassFlags::AsyncCompute,
                [PassParameters, ComputeShader, GroupCount](FRHIComputeCommandList& RHICmdList)
                {
                    FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, GroupCount);
                }
            );

            if (TargetTexture->Desc.Format == PF_B8G8R8A8)
            {
                AddCopyTexturePass(GraphBuilder, TmpTexture, TargetTexture, FRHICopyTextureInfo());
            }
            else
            {
                #if WITH_EDITOR
                    GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The provided render target has an incompatible format (Please change the RT format to: RGBA8).")));
                #endif
            }
        }
        else
        {
            #if WITH_EDITOR
                GEngine->AddOnScreenDebugMessage((uint64)42145125184, 6.f, FColor::Red, FString(TEXT("The compute shader has a problem.")));
            #endif
        }
    }

    GraphBuilder.Execute();
}

/*
 * Dispatches the shader on the render thread via a game thread command.
 * This ensures that the shader is executed in the correct rendering context.
 */
void UWriteToRenderTarget::DispatchGameThread(UTexture2D* InputTexture, FWriteToRenderTargetDispatchParams Params)
{
    ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
        [InputTexture, Params, this](FRHICommandListImmediate& RHICmdList)
        {
            DispatchRenderThread(RHICmdList, InputTexture, Params);
        });
}

/*
 * Determines whether the shader is being executed on the render thread or game thread,
 * and dispatches the shader accordingly.
 */
void UWriteToRenderTarget::Dispatch(UTexture2D* InputTexture, FWriteToRenderTargetDispatchParams Params)
{
    if (IsInRenderingThread())
    {
        DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), InputTexture, Params);
    }
    else
    {
        DispatchGameThread(InputTexture, Params);
    }
}
