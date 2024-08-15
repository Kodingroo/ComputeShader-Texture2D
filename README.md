# Technical Assignment: Compute Shader Plugin for Unreal Engine

## Introduction

This project was undertaken as a technical assignment for Corpy & Co., completed over the weekend of August 10th, 2024. Designed to be as user-friendly as possible, it demonstrates my technical proficiency and creative problem-solving within the Unreal Engine environment, with the goal of contributing meaningfully to the Corpy & Co. team.

The project integrates a custom compute shader plugin into Unreal Engine, enabling advanced image manipulation within the engine. The shader processes textures by applying transformations such as color inversion and rotation. These features are accessible both through C++ and Blueprint interfaces, allowing flexibility in how they are used across a project.

The compute shader leverages the GPU's parallel processing capabilities to execute multiple threads simultaneously. This approach enables real-time transformations like color inversion, grayscale, and distortion with minimal performance impact. It is designed to be adaptable, handling texture operations that can be initiated from either the game thread or the rendering thread, ensuring smooth integration with Unreal Engine’s rendering pipeline. This setup is ideal for tasks that require dynamic texture processing, such as real-time image effects or procedural texture generation, while maintaining compatibility with Unreal’s existing systems.

## Table of Contents

1. [Usage in Unreal Engine](#usage-in-unreal-engine)
   - [Blueprint Integration](#blueprint-integration)
2. [Key Classes and Structures](#key-classes-and-structures)
   - [FWriteToRenderTargetDispatchParams](#fwritetorendertargetdispatchparams)
   - [FWriteToRenderTargetInterface](#fwritetorendertargetinterface)
   - [UWriteToRenderTargetLibrary](#uwritetorendertargetlibrary)
3. [Shader Details](#shader-details)
   - [Shader Code Breakdown](#shader-code-breakdown)
   - [Usage](#usage)
4. [Module Setup](#module-setup)
   - [ComputeShaderModule](#computeshadermodule)
   - [ComputeShaderModuleEditor](#computeshadermoduleeditor)
5. [Conclusion](#conclusion)

## 1. Usage in Unreal Engine

### Blueprint Integration

The goal was to provide a user-friendly interface that allows designers and developers to easily add and manipulate 2D textures. The `UWriteToRenderTargetLibrary` class includes the `ExecuteRTComputeShader` function, which can be called directly from Blueprints to execute the compute shader on a specified input texture and render target.

**Steps for Blueprint Integration**

1. **Enable the Shader Mode Utility Widget (If Not Already Visible):**
   - Navigate to *Editor Preferences > Corpy & Co* and ensure that the Shader Mode Utility Widget is enabled. This will allow you to manipulate shader parameters directly within the editor.

2. **Open the Blueprint Editor:**
   - Navigate to the Blueprint where you want to integrate the shader. In the sample project, this will be the `W_TestPlane` in the Content folder.

3. **Add the Compute Shader Node:**
   - Search for the `ExecuteRTComputeShader` function in the Blueprint node library.
   - Add this node to your Blueprint and connect it to the appropriate execution path.

4. **Specify Input Parameters:**
   - Assign the `InputTexture` (a `Texture2D` object) that you want to process.
   - Set the `RenderTarget` (such as the provided `RT_ComputeShaderTexture2D` found in the Shaders folder of the Compute Shader Mod Content directory) where the processed output will be written.

5. **Manipulate Shader Parameters in the Utility Widget:**
   - Use the Shader Mode Utility Widget to dynamically control the shader's parameters, including `InvertColors`, `Greyscale`, `Contrast`, `DistortionStrength`, `ImageScale`, and `RotationAngle`. These parameters are adjusted in real-time, providing immediate visual feedback.

6. **Apply the Sample Material:**
   - To visualize the shader's effects, apply the `M_ShaderModRenderTarget` material (also located in the Shaders folder) to a plane in your scene. This setup allows you to observe the processed output in real-time.

7. **Trigger the Shader:**
   - The shader will execute when the Blueprint reaches the `ExecuteRTComputeShader` node, applying the specified effects to the `InputTexture` and writing the result to the `RenderTarget`.

## 2. Key Classes and Structures

### FWriteToRenderTargetDispatchParams

`FWriteToRenderTargetDispatchParams` defines the dimensions (X, Y, Z) for the shader execution and holds a reference to the render target. This struct is essential for setting up the shader environment and ensuring proper execution on the GPU and render thread.

### UWriteToRenderTargetLibrary

`UWriteToRenderTargetLibrary` provides a Blueprint-accessible way to invoke the compute shader. This class simplifies the process of setting up and dispatching the shader by encapsulating the necessary steps within a single function call, `ExecuteRTComputeShader`. This approach allows designers and developers to utilize the shader in both the Unreal Editor and at runtime without requiring deep C++ knowledge.

## 3. Shader Details

### Shader Code Breakdown

The shader code is designed to apply various image transformations to a texture, including color inversion, grayscale conversion, contrast adjustment, and geometric distortions like scaling and rotation. The shader begins by defining key resources, such as the input texture, the sampler for texture sampling, and the output render target where the processed image will be written.

The core of the shader lies in its `Main` function, which is executed for each pixel in the output render target. The function starts by calculating the UV coordinates for the current pixel, which are used to sample the input texture. These coordinates are then manipulated based on the shader's parameters: the UVs are rotated, scaled, and distorted according to the specified transformation values. The shader then samples the texture at the distorted coordinates, applies grayscale and contrast adjustments if needed, and optionally inverts the colors. The final color is written to the output render target, resulting in the desired visual effect.

By using the `[numthreads]` directive, the shader is designed to run multiple threads in parallel, allowing it to process large textures efficiently on the GPU. The shader's design is modular, enabling developers to easily toggle effects or adjust parameters without modifying the core logic. This flexibility makes it well-suited for real-time applications where dynamic texture manipulation is required.

### Usage

The shader operates in two main contexts within the project. On the Game Thread, it handles real-time texture processing during gameplay, allowing dynamic adjustments to textures through Blueprints. On the Render Thread, it is responsible for post-processing effects and editor utility operations, ensuring efficient execution of custom rendering logic.

To use the shader effectively, it is important to first initialize the shader resources, such as the input texture and render target. Once initialized, the shader can be dispatched to perform its operations. The flexibility of the system allows it to be used in various scenarios, from real-time image processing to dynamic texture generation. By integrating the shader into an application, developers can create complex visual effects that enhance the overall user experience.

## 4. Module Setup

### ComputeShaderModule

The `ComputeShaderModule` is the core module responsible for managing and executing the compute shader operations within the project. This module is configured as part of Unreal Engine's modular system, which allows it to be loaded and unloaded dynamically. The module's setup includes defining private and public dependencies, ensuring that the shader can interact with the rendering pipeline and GPU resources effectively.

### ComputeShaderModuleEditor

The `ComputeShaderModuleEditor` class extends the functionality of the `ComputeShaderModule` by providing tools and utilities specifically designed for the Unreal Engine editor environment. This editor module includes components such as the `ShaderModWidget`, a user interface element that allows developers to interact with the shader parameters directly from within the editor. The module also manages editor-specific settings, enabling the configuration of shader behavior and features through the Unreal Editor. By integrating with Unreal Engine's editor framework, `ComputeShaderModuleEditor` allows for real-time adjustments and testing of shader effects.

## 5. Conclusion

This project demonstrates the integration of a custom compute shader within Unreal Engine, providing a user-friendly interface for designers and developers to manipulate 2D textures in real-time. By leveraging the power of the GPU and the provided tools and assets, the shader efficiently applies a range of visual effects, all accessible directly from a utility widget.

I sincerely appreciate the chance to work on this assignment for Corpy & Co. It has been an excellent research project that has allowed me to deepen my understanding of shader development and Unreal Engine’s capabilities. I look forward to the possibility of collaborating on similar innovative projects in the future.
