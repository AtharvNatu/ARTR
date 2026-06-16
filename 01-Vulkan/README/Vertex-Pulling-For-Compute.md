# Programmable Vertex Pulling - Indirect Drawing

## Problem

49 FPS to 140 FPS

External Vertex Buffer and CPU Vertex Buffer have added flags as SSBO

The initial template for indirect drawing uses fixed-function vertex attributes

 - Your initial setup used "Fixed-Function" vertex attributes. The GPU's Input Assembler was hard-wired to fetch data from both your CPU and GPU buffers for every vertex. Even if the shader only used one, the hardware wasted half your memory bandwidth pre-fetching data you were about to throw away.

## Solution

We switched to Vertex Pulling. By binding your buffers as Storage Buffers (SSBOs) and using gl_VertexIndex to fetch data only when needed, the GPU stopped the unnecessary pre-fetching. This reduced memory traffic by 50%.


# Resizable BAR - Compute Shader

Using resizable bar - gives option  to bypass device_local only which needs staging buffer for updating data to buffer

Resizable BAR (ReBAR)ReBAR is a PCIe technology that removed the "256MB window" bottleneck.

- Without ReBAR: The CPU can only see 256MB of VRAM at a time. To update your vertex buffer, the driver has to constantly map and unmap small chunks of memory, creating massive CPU overhead.

 - With  ReBAR: The CPU sees the entire VRAM (your 16GB) as a single, continuous block. This allowed us to:
 
     1. Persistent Mapping: Map the entire buffer once at startup and never call vkMapMemory again, eliminating a major source of stuttering and driver overhead.
     
     2. Zero-Copy Flow: We combined DEVICE_LOCAL and HOST_VISIBLE flags. This put the data in high-speed VRAM (for the 160 FPS Compute shader writes) while still allowing the CPU to see it for your "CPU-toggle" path.
     

Fixing the Compute BottleneckYour Compute Shader was originally running at 25–35 FPS because it was either performing a slow vkCmdCopyBuffer or writing directly to system RAM across the PCIe bus.The Fix: By allocating on the ReBAR-enabled Device Local heap, the Compute Shader now writes directly to the same VRAM the graphics pipeline uses.The Result: We removed the copy command and used a single Pipeline Barrier to hand off the data instantly from the Compute stage to the Vertex stage.