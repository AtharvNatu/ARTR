# Problem - CUDA / OpenCL Indirect Drawing

### Initial implementation achieved approximately 49 FPS (NVIDIA RTX 5070)

- The rendering pipeline used a traditional fixed-function vertex input stage.

- The GPU's Input Assembler was hard-wired to fetch data from both the CPU and GPU buffers for every vertex. 

- Even if the shader only used one, the hardware wasted half the memory bandwidth pre-fetching data which was about to be discarded.


# Solution - Programmable Vertex Pulling

- Instead of allowing the fixed-function hardware to fetch vertex data:
 
    - Vertex input attributes are removed.

    - CPU and GPU buffers are exposed as `Storage Buffers (SSBOs)`.

    - Vertex data was fetched explicitly inside the shader using `gl_VertexIndex`.

# Result

- By moving from fixed-function fetch to programmable fetch:

    - Redundant memory transactions are eliminated, reducing memory traffic by `50%`.

    - Vertex cache efficiency improved.


## Observed improvement:

```text
49 FPS → 140 FPS
```

This represented approximately:

```text
2.8× rendering throughput increase
```

for the tested workload.

