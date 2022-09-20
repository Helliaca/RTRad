# RTRad

## What is it?

RTRad is an enhanced implementation of *progressive refinement radiosity* using Nvidia's RTX technology to accelerate visibility calculations.

The project is built on [Falcor 4.4](https://github.com/NVIDIAGameWorks/Falcor/releases/tag/4.4) and contains a highly configureable pipeline.
GI lightmaps can be generated in a variety of different ways and individual data components can easily be visualized. Custom scenes and models can be defined and loaded into the rendering environment using the `.pyscene` format.

The development took place as part of a Masters's thesis at the [Freie Universität Berlin](https://www.fu-berlin.de/). The thesis contains a detailed account on all features and requirements. It can be viewed and used as a form of documentation [here](https://www.todo.com/).

Here's a simple demonstration video that serves as an overview:

[![Demo Video](https://i.imgur.com/aFJFiaD.png)](https://www.youtube.com/watch?v=84rVIG8G1Eg)

# Instructions

## Setup

If you are looking to run this application without making code-based changes, simply head on over to the [github releases tab](https://github.com/Helliaca/RTRad/releases) and download the newest version. Make sure you have a GPU which supports DirectX Raytracing with a reasonably recent driver installed.

For a development setup you can use newer versions of Visual Studio, but you will need to have Visual Studio 2019 installed in order to build the original Falcor 4.4 from source.
You also need:

- Windows 10 version 20H2 (October 2020 Update) or newer
- [Windows 10 SDK (10.0.19041.0) for Windows 10, version 2004](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/)

Once you've got those:
- Clone the git repository
- Go into `/Tools` and run `update_dependencies.bat`
- Open the sln file in Visual Studio, if VS asks you if you want to upgrade the project, select "No Upgrade" on all options
- Right click the "RTRad" project and click on "Set us Startup Project"
- Select the "Debug D3D12" configuration
- Start/Build the project

### Common issues

Some issues can occur whilst setting thins up. Ultimately, make sure that you:

- Have all the dependencies installed (VS2019 and Windows 10 SDK)
- Have selected the "Debug D3D12" configuration
- Have RTRad selected as your startup project
- If you `right-click > Properties` any of the two projects (Falcor and RTRad) it should have "10.0.19041.0" as the Windows SDK version and "Visual Studio 2019 (v142)" as the Platform Toolset.
- The C++ Language Standard should be "ISO C++ 17 Standard"
- Falcor's configuration type should be set to "Dynamic Library (.dll)" and RTRad to "Application (.exe)"

## Generating Lightmaps

Once you get the project running, you will see a basic scene with a GUI.

The upper left-most panel will let you run a new radiosity pass (corresponding to one bounce of light) or reset back to zero.
The upper right-most panel lets you select a resolution for the lightmap. Stick to low resolutions unless you use one of the faster methods/variants listed further down.

In the bottom left you can set "Batching Settings", which determines how many pixels (ie. radiosity patches) are processed before returning control back to the CPU. If you experience crashes or freezes, try lowering the batch-size. Passes generally run faster with the largest batch-size you can get away with (depends on GPU).

You can press "F2" to toggle the GUI and "P" to bring up the profiler. Whilst the profiler is active, the pass-time of the current (ie. last) pass will be displayed on the bottom of the "RTRad" panel.

### Methods:

There are roughly 4 different variants of progressive radiosity you can execute:

- *Pure Progressive Radiosity* by selecting the integral type as "Area" and sampling method to be "None". This executes a complete radiosity pass where all other patches area sampled to calculate lighting contribution. This method provides the best lighting but is prohibitive for larger lightmaps.
- *Undersampled* by selecting the integral type as "Area" and sampling settings to be one of the three "Undersampling" options, "Monte-Carlo" generally provides the best results. You will also have to select a "Sampling Window". The larger this one is, the faster the algorithm will run. I generally recommend a 1/128 ratio between lightmap resolution and sampling window, which will let you compute large lightmaps in a fairly small time with virtually no cost in visual fidelity.
- *Adaptive Subdivision* by selecting the integral type as "Area" and sampling settings as "Adaptive Subdivision". This is a mostly slower version of Undersampling but which can provide lightmaps that require smaller memory by packing them into quad-trees. The speed of this algorithm will depend on how the max. node size and gradient threshold interact with the scene composition and is generally hard to predict accurately.
- *Hemispheric* by selecting the integral type as "Hemispheric". This is generally the fastest method and best-suited for very large lightmaps. Unfortunately, it can provide unsatisfactory results in scenes with small light-sources. You can select how many samples you shoot for each pixel in the lightmap, though the method is fast enough so that you can leave this at the maximum value of 1024.

Note: The *first* pass you execute after loading a scene can be slower than subsequent ones, because the accleration structure needs to be rebuilt.

For detailed information on each of these methods, please refer to the underlying [thesis](https://www.todo.com/).

### Visualizing Data

...

### Loading New Scenes

...

### Creating New Scenes

...

### Extra Features

- Viscaching
- Sewseams
- Cons. Rasterization
- ...

![Image](https://i.imgur.com/hB4Bq5a.jpg)
