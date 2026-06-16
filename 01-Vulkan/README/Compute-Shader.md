
# Compute Shader Optimization using Resizable BAR (ReBAR)


- Resizable BAR is a PCIe capability that allows the CPU to access a larger portion of GPU VRAM directly.

    Without ReBAR:
    ```text
    CPU
    ↓
    256 MB VRAM Window
    ↓
    Driver Remapping
    ↓
    VRAM
    ```

- Historically the CPU could only address a small aperture of VRAM at one time.

- Large updates required:

    - Window remapping
    - Additional driver management
    - Increased CPU overhead

    With ReBAR enabled:
    ```text
    CPU
    ↓
    Entire VRAM Address Space
    ```

- The CPU can directly access GPU-local memory.


## Solution — Device Local + Host Visible Allocation

- Buffers were allocated using:

    ```text
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    ```

- This enabled placement into:

    ```text
    Host-visible VRAM
    ```

- Advantages:

    - `Persistent Mapping`

        Buffers were mapped once and reused throughout execution:

        ```c
        vkMapMemory(...)
        ```

        - Benefits:

            - Eliminated repeated map/unmap calls
            - Reduced driver overhead
            - Reduced frame stutter

    - `Zero-Copy Data Flow`

        The compute stage and graphics stage now shared the same physical memory.


        Pipeline:

        ```text
        Compute Shader
        ↓
        VRAM
        ↓
        Pipeline Barrier
        ↓
        Vertex Stage
        ```

        No intermediate transfer required.


    - `Direct Compute Output Consumption`

        Previous flow:

        ```text
        Compute
        ↓
        Copy
        ↓
        Graphics
        ```

        Optimized flow:

        ```text
        Compute
        ↓
        Single Buffer
        ↓
        Graphics
        ```

        Synchronization became:

        ```c
        vkCmdPipelineBarrier(
            COMPUTE_SHADER_BIT,
            VERTEX_INPUT_BIT
        )
        ```

        which guaranteed visibility without copying.
        

# Result

Compute throughput improved from:

```text
25–35 FPS
```

to approximately:

```text
160+ FPS
```

## Key contributors:

 - Removal of staging copies
 - Elimination of PCIe round-trips
 - Persistent mapping
 - Shared compute → graphics memory path
 - Reduced synchronization overhead


Overall, the optimization replaced:

```text
Copy-Based GPU Workflow
```

with:

```text
Shared GPU Memory + Explicit Synchronization
```

resulting in substantially higher utilization of GPU bandwidth and lower CPU overhead.
