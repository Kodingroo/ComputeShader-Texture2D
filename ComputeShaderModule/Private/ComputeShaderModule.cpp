// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComputeShaderModule.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FComputeShaderModule"

void FComputeShaderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ShaderMod"))->GetBaseDir(), TEXT("Shaders/ComputeShaderModule/Private"));
	AddShaderSourceDirectoryMapping(TEXT("/ComputeShaderModuleShaders"), PluginShaderDir);
}

void FComputeShaderModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FComputeShaderModule, ShaderMod)