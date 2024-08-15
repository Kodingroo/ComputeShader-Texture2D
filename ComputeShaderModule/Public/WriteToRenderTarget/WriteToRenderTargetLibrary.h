#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "WriteToRenderTargetLibrary.generated.h"

class UWriteToRenderTarget;

UCLASS()
class COMPUTESHADERMODULE_API UWriteToRenderTargetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	 * Executes the compute shader on the provided input texture and render target.
	 * It ensures that the compute shader runs with the correct input data and outputs to the specified render target.
	 */
	UFUNCTION(BlueprintCallable)
	static void ExecuteRTComputeShader(UTexture2D* InputTexture, UTextureRenderTarget2D* RT);

	/*
	 * A singleton instance of UWriteToRenderTarget used to maintain state between function calls.
	 * This instance is reused to avoid repeatedly creating and destroying objects.
	 */
	static UWriteToRenderTarget* WriteToRenderTargetInstance;
};
