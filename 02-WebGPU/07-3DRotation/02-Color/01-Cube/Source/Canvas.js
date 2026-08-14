//* Global Variables
var canvas = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

//! WebGPU Related
var clearColor;
let device = null;
let context = null;
let queue = null;
let canvasFormat = null;
let animationFrameId = null;

let buffer_position_cube = null;
let buffer_color_cube = null;

let buffer_mvpUniform_cube = null;
let bindGroup_mvpUniform_cube = null;

let render_pipeline = null;
let perspectiveProjectionMatrix = null;

let depthTexture = null;

var angle = 0.0;
const animationSpeed = 0.5;

//* Animation Related
var requestAnimationFrame = window.requestAnimationFrame ||                // Chrome
                            window.mozRequestAnimationFrame ||             // Mozilla
                            window.webkitRequestAnimationFrame ||          // Safari
                            window.oRequestAnimationFrame ||               // Opera
                            window.msRequestAnimationFrame;                // Edge

var cancelAnimationFrame = window.cancelAnimationFrame ||                 // Chrome
                           window.webkitCancelRequestAnimationFrame ||    // Safari  
                           window.webkitCancelAnimationFrame ||           // Safari
                           window.mozCancelRequestAnimationFrame ||       // Mozilla
                           window.mozCancelAnimationFrame ||              // Mozilla
                           window.oCancelRequestAnimationFrame ||         // Opera
                           window.oCancelAnimationFrame ||                // Opera
                           window.msRequestCancelAnimationFrame ||        // Edge
                           window.msCancelAnimationFrame;                 // Edge

async function main()
{
    // Code

    // Get Canvas
    canvas = document.getElementById("ADN");
    if (!canvas)
        console.log("Failed To Obtain Canvas !!!");
    else 
        console.log("Canvas Obtained");

    // Backup Canvas Dimensions
    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    // Register Event Listeners
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mousedown, false);
    window.addEventListener("resize", resize, false);

    //* Best practices for WebGPU during full screen mode
    document.addEventListener("fullscreenchange", onFullScreenChange);
    document.addEventListener("webkitfullscreenchange", onFullScreenChange);

    //! Initialize WebGPU

    //* Step - 1 : Get GPU Interface
    const gpu = navigator.gpu;
    if (gpu == null)
    {
        console.log("WebGPU Is Not Supported By This Browser !!!");
        throw Error("WebGPU Is Not Supported By This Browser !!!");
    }
    else
        console.log("WebGPU Supported");

    //* Step - 2 : Get GPUAdapter Object From GPU Interface
    const adapter = await gpu.requestAdapter();
    if (adapter == null)
    {
        console.log("Failed to get GPUAdapter object !!!");
        throw Error("Failed to get GPUAdapter object !!!");
    }
    else
        console.log("GPUAdapter Instance Successfully Acquired");

    //* Step - 3 : Get GPU Device Object From Adapter
    device = await adapter.requestDevice();
    if (device == null)
    {
        console.log("Failed To Get GPU Device From Adapter !!!");
        throw Error("Failed To Get GPU Device From Adapter !!!");
    }
    else
        console.log("GPU Device successfully initialized");

    const adapterInfo = await adapter.info;
    console.log("WebGPU Device Information");
    console.log("---------------------------------------");
    console.log(`Vendor : ${adapterInfo.vendor}`);
    console.log(`Architecture : ${adapterInfo.architecture}`);
    console.log(`Description : ${adapterInfo.description}`);
    console.log("---------------------------------------");

    //* Step - 4 : Listener To Handle Lost Devices
    document.addEventListener("uncapturederror", onUncapturedError);
    device.lost.then(onDeviceLost);

    // Initialize
    await initialize();

    // Warmup Resize
    resize();

    // Display
    display();

}

function onUncapturedError(event)
{
    // Code
    console.error("WebGPU Uncaptured Error : ", event.error.message);
}

function onDeviceLost(info)
{
    // Code
    console.warn("WebGPU Device Lost : ", info.reason, ", Message : ", info.message);

    queue = null;

    buffer_position_cube = null;
    buffer_color_cube = null;

    buffer_mvpUniform_cube = null;
    bindGroup_mvpUniform_cube = null;

    render_pipeline = null;
    perspectiveProjectionMatrix = null;
    depthTexture = null;
}

function toggleFullScreen()
{
    // Code
    var fullscreen_element = document.fullscreenElement ||       // Generic
                             document.mozFullScreenElement ||    // Mozilla
                             document.webkitFullscreenElement || // Apple
                             document.msFullscreenElement ||     // Edge
                             null;                               
    
    if (fullscreen_element == null)
    {
        if (canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if (canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if (canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if (canvas.msRequestFullscreen)  
            canvas.msRequestFullScreen();
    }
    else
    {
        if (document.exitFullscreen)
            document.exitFullscreen();
        else if (document.mozExitFullScreen)
            document.mozExitFullscreen();
        else if (document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if (document.msExitFullscreen)  
            document.msExitFullscreen();
    }                     
}

function onFullScreenChange()
{
    // Code
    var fullscreen_element = document.fullscreenElement ||       // Generic
                             document.mozFullScreenElement ||    // Mozilla
                             document.webkitFullscreenElement || // Apple
                             document.msFullscreenElement ||     // Edge
                             null;                               
    
    if (fullscreen_element == null)
        bFullscreen = false;
    else
        bFullscreen = true;

    resize();
}

async function initialize()
{
    // Code

    //* Step - 5 : Get GPUQueue Object (Synchronous)
    queue = device.queue;
    console.log("GPUQueue Acquired Successfully");

    //* Step - 6 : Get WebGPU Context
    context = canvas.getContext("webgpu");
    if (context == null)
    {
        console.log("Failed To Get WebGPU Context From Canvas !!!");
        throw Error("Failed To Get WebGPU Context From Canvas !!!");
    }
    else
        console.log("WebGPU Context Successfully Initialized");

    //* Step - 7 : Get Preferred WebGPU Color Format For Canvas
    canvasFormat = navigator.gpu.getPreferredCanvasFormat();

    //* Step - 8 : Configure Canvas Using The Format
    const canvasConfiguration = 
    {
        device: device,
        format: canvasFormat,
        usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
        alphaMode: "opaque"
    };

    context.configure(canvasConfiguration);

    //* Step - 9 : Define the clear color
    clearColor = 
    { 
        r: 0.0,
        g: 0.0,
        b: 0.0,
        a: 1.0
    };

    //* Load Shaders From File
    const triangleVertWGSL = await loadShader("../Shaders/triangle.vert.wgsl");
    const triangleFragWGSL = await loadShader("../Shaders/triangle.frag.wgsl");

    //* Vertex Shader Module
    const shaderModuleDescriptor_vertexShader = 
    {
        code: triangleVertWGSL
    };

    const shaderModule_vertexShader = device.createShaderModule(shaderModuleDescriptor_vertexShader);
    if (shaderModule_vertexShader == null)
    {
        console.log("Failed To Create Vertex Shader Module !!!");
        throw Error("Failed To Create Vertex Shader Module !!!");
    }
    else
        console.log("Vertex Shader Module Successfully Created");

    //* Fragment Shader Module
    const shaderModuleDescriptor_fragmentShader = 
    {
        code: triangleFragWGSL
    };

    const shaderModule_fragmentShader = device.createShaderModule(shaderModuleDescriptor_fragmentShader);
    if (shaderModule_fragmentShader == null)
    {
        console.log("Failed To Create Fragment Shader Module !!!");
        throw Error("Failed To Create Fragment Shader Module !!!");
    }
    else
        console.log("Fragment Shader Module Successfully Created");

    var vertex_position_cube = new Float32Array([
        // Front
        1.0,  1.0,  1.0,  1.0,   // Top Right
       -1.0,  1.0,  1.0,  1.0,   // Top Left
        1.0, -1.0,  1.0,  1.0,   // Bottom Right

        1.0, -1.0,  1.0,  1.0,   // Bottom Right
       -1.0,  1.0,  1.0,  1.0,   // Top Left
       -1.0, -1.0,  1.0,  1.0,   // Bottom Left

        // Right
        1.0,  1.0, -1.0,  1.0,   // Top Right
        1.0,  1.0,  1.0,  1.0,   // Top Left
        1.0, -1.0, -1.0,  1.0,   // Bottom Right

        1.0, -1.0, -1.0,  1.0,   // Bottom Right
        1.0,  1.0,  1.0,  1.0,   // Top Left
        1.0, -1.0,  1.0,  1.0,   // Bottom Left

        // Back
        1.0,  1.0, -1.0,  1.0,   // Top Right
       -1.0,  1.0, -1.0,  1.0,   // Top Left
        1.0, -1.0, -1.0,  1.0,   // Bottom Right

        1.0, -1.0, -1.0,  1.0,   // Bottom Right
       -1.0,  1.0, -1.0,  1.0,   // Top Left
       -1.0, -1.0, -1.0,  1.0,   // Bottom Left

        // Left
       -1.0,  1.0,  1.0,  1.0,   // Top Right
       -1.0,  1.0, -1.0,  1.0,   // Top Left
       -1.0, -1.0,  1.0,  1.0,   // Bottom Right

       -1.0, -1.0,  1.0,  1.0,   // Bottom Right
       -1.0,  1.0, -1.0,  1.0,   // Top Left
       -1.0, -1.0, -1.0,  1.0,   // Bottom Left

        // Top
        1.0,  1.0, -1.0,  1.0,   // Top Right
       -1.0,  1.0, -1.0,  1.0,   // Top Left
        1.0,  1.0,  1.0,  1.0,   // Bottom Right

        1.0,  1.0,  1.0,  1.0,   // Bottom Right
       -1.0,  1.0, -1.0,  1.0,   // Top Left
       -1.0,  1.0,  1.0,  1.0,   // Bottom Left

        // Bottom
        1.0, -1.0,  1.0,  1.0,   // Top Right
       -1.0, -1.0,  1.0,  1.0,   // Top Left
        1.0, -1.0, -1.0,  1.0,   // Bottom Right

        1.0, -1.0, -1.0,  1.0,   // Bottom Right
       -1.0, -1.0,  1.0,  1.0,   // Top Left
       -1.0, -1.0, -1.0,  1.0   // Bottom Left
    ]);

    var vertex_color_cube = new Float32Array([
        // Front
        0.0,   0.0,   1.0,  1.0,  
        0.0,   0.0,   1.0,  1.0,  
        0.0,   0.0,   1.0,  1.0,  

        0.0,   0.0,   1.0,  1.0,  
        0.0,   0.0,   1.0,  1.0,  
        0.0,   0.0,   1.0,  1.0, 
        
        // Right
        1.0,   1.0,   0.0,  1.0,  
        1.0,   1.0,   0.0,  1.0,  
        1.0,   1.0,   0.0,  1.0,  

        1.0,   1.0,   0.0, 1.0,  
        1.0,   1.0,   0.0, 1.0,  
        1.0,   1.0,   0.0, 1.0,

        // Back
        0.0,   1.0,   1.0,  1.0,  
        0.0,   1.0,   1.0,  1.0,  
        0.0,   1.0,   1.0,  1.0,  

        0.0,   1.0,   1.0,  1.0,  
        0.0,   1.0,   1.0,  1.0,  
        0.0,   1.0,   1.0,  1.0,  

        // Left
        1.0,   0.0,   1.0,  1.0,  
        1.0,   0.0,   1.0,  1.0,  
        1.0,   0.0,   1.0,  1.0,  

        1.0,   0.0,   1.0,  1.0,  
        1.0,   0.0,   1.0,  1.0,  
        1.0,   0.0,   1.0,  1.0,
        
        // Top
        1.0,   0.0,   0.0,  1.0,  
        1.0,   0.0,   0.0,  1.0,  
        1.0,   0.0,   0.0,  1.0,  

        1.0,   0.0,   0.0,  1.0,  
        1.0,   0.0,   0.0,  1.0,  
        1.0,   0.0,   0.0,  1.0,  

        // Bottom
        0.0,   1.0,   0.0,  1.0,  
        0.0,   1.0,   0.0,  1.0,  
        0.0,   1.0,   0.0,  1.0,  

        0.0,   1.0,   0.0,  1.0,  
        0.0,   1.0,   0.0,  1.0,  
        0.0,   1.0,   0.0,  1.0
    ]);

    //! Common Bind Group Layout For Pyramid and Cube
    const bindGroupLayout_mvpUniform = createBindGroupLayout(0, GPUShaderStage.VERTEX, "uniform");

    //! Cube - Position Buffer, Color Buffer, Uniform Buffer, Bind Group
    //* ---------------------------------------------------------------------------------------------------------------------------------

    //* Vertex Buffers
    buffer_position_cube = createVertexBuffer(
        vertex_position_cube, 
        vertex_position_cube.byteLength, 
        vertex_position_cube.length, 
        GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    );

    buffer_color_cube = createVertexBuffer(
        vertex_color_cube, 
        vertex_color_cube.byteLength, 
        vertex_color_cube.length, 
        GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
    );

    //* MVP Uniform Buffer
    const mvpUniformBufferSize = 4 * 16;
    buffer_mvpUniform_cube = createUniformBuffer(mvpUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);
    
    //* Bind Group For MVP Uniform
    bindGroup_mvpUniform_cube = createBindGroup(buffer_mvpUniform_cube, 0, mvpUniformBufferSize, 0, bindGroupLayout_mvpUniform);
    //* ---------------------------------------------------------------------------------------------------------------------------------

    
    //* Step - 2: Pipeline Layout for MVP Uniform

    //* Step - 2A: Pipeline Layout Descriptor
    const pipelineLayoutDescriptor = 
    {
        bindGroupLayouts:
        [
            bindGroupLayout_mvpUniform
        ]
    };

    //* Step - 2B: Pipeline Layout
    const pipelineLayout = device.createPipelineLayout(pipelineLayoutDescriptor);
    if (pipelineLayout == null)
    {
        console.log("Failed To Create Pipeline Layout For MVP Uniform !!!");
        throw Error("Failed To Create Pipeline Layout For MVP Uniform !!!");
    }
    else
        console.log("Pipeline Layout For MVP Uniform Successfully Created");

    //! Render Pipeline

    //* Step - 1 : Pipeline Descriptor / PSO

    //* Step - 1A: Vertex Buffer Layout

    //! Position Attribute
    const positionVertexAttribute = 
    {
        shaderLocation: 0,  //* Maps to location(0) in Vertex Shader
        offset: 0,
        format: "float32x4"
    };

    const positionVertexBufferLayout = 
    {
        attributes: 
        [
            positionVertexAttribute
        ],
        arrayStride: 4 * 4,
        stepMode: "vertex"  // Jump vertex by vertex, not instance by instance
    };

    //! Color Attribute
    const colorVertexAttribute = 
    {
        shaderLocation: 1,  //* Maps to location(1) in Vertex Shader
        offset: 0,
        format: "float32x4"
    };

    const colorVertexBufferLayout = 
    {
        attributes: 
        [
            colorVertexAttribute
        ],
        arrayStride: 4 * 4,
        stepMode: "vertex"  // Jump vertex by vertex, not instance by instance
    };

    //* Step - 1B: Vertex Shader State
    const vertexShaderState = 
    {
        module: shaderModule_vertexShader,
        entryPoint: "main",
        buffers: 
        [
            positionVertexBufferLayout,
            colorVertexBufferLayout
        ]
    };

    //* Step - 1C: Fragment Shader State
    const colorTargetState = 
    {
        format: canvasFormat
    };

    const fragmentShaderState = 
    {
        module: shaderModule_fragmentShader,
        entryPoint: "main",
        targets:
        [
            colorTargetState
        ]
    };

    //* Step - 1D: Primitive State
    const primitiveState = 
    {
        frontFace: "ccw",    // Counter-Clockwise
        cullMode: "none",    // No Culling
        topology: "triangle-list"
    };

    //* Depth-Stencil State
    const depthStencilState = 
    {
        depthWriteEnabled: true,
        depthCompare: "less-equal",
        format: "depth24plus-stencil8"
    };

    //* Step - 1E: PSO
    const pipelineDescriptor = 
    {
        layout: pipelineLayout,
        vertex: vertexShaderState,
        fragment: fragmentShaderState,
        primitive: primitiveState,
        depthStencil: depthStencilState
    };

    render_pipeline = device.createRenderPipeline(pipelineDescriptor);
    if (render_pipeline == null)
    {
        console.log("Failed To Create Render Pipeline !!!");
        throw Error("Failed To Create Render Pipeline !!!");
    }
    else
        console.log("Render Pipeline Successfully Created");

    //* Initialize Projection Matrix
    perspectiveProjectionMatrix = mat4.create();
}

async function loadShader(path)
{
    // Code
    const response = await fetch(path);
    return await response.text();
}

function createVertexBuffer(_data, _size, _length, _bufferUsage)
{
    // Code
    const bufferDescriptor = 
    {
        size: _size,
        usage: _bufferUsage
    };

    buffer = device.createBuffer(bufferDescriptor);
    if (buffer == null)
    {
        console.log("Failed To Create Buffer !!!");
        throw Error("Failed To Create Buffer !!!");
    }
    else
        console.log("Buffer Successfully Created");

    queue.writeBuffer(buffer, 0, _data, 0, _data.length);
    console.log("Data Written To Buffer");

    return buffer;
}

function createBindGroupLayout(_bindingIndex, _shaderStageVisibility, _uniformType)
{
    // Code

    //* Step - 1A: Bind Group Layout Entry
    const bindGroupLayoutEntry = 
    {
        binding: _bindingIndex,
        visibility: _shaderStageVisibility,
        buffer: 
        {
            type: _uniformType
        }
    };

    //* Step - 1B: Bind Group Layout Descriptor
    const bindGroupLayoutDescriptor = 
    {
        entries: 
        [
            bindGroupLayoutEntry
        ]
    };

    //* Step - 1C: Bind Group Layout
    const bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDescriptor);
    if (bindGroupLayout == null)
    {
        console.log("Failed To Create Bind Group Layout !!!");
        throw Error("Failed To Create Bind Group Layout !!!");
    }
    else
        console.log("Bind Group Layout Successfully Created");

    return bindGroupLayout;
}

function createUniformBuffer(_uniformBufferSize, _uniformBufferUsage)
{
    // Code
    const bufferDescriptor = 
    {
        size: _uniformBufferSize,
        usage: _uniformBufferUsage
    };

    let buffer_uniform = device.createBuffer(bufferDescriptor);
    if (buffer_uniform == null)
    {
        console.log("Failed To Create Uniform Buffer !!!");
        throw Error("Failed To Create Uniform Buffer !!!");
    }
    else
        console.log("Uniform Buffer Successfully Created");

    return buffer_uniform;
}

function createBindGroup(_uniformBuffer, _offset, _uniformBufferSize, _binding, _bindGroupLayout)
{
    // Code

    //* Step - 1A: Buffer Binding Property
    const bufferBinding = 
    {
        buffer: _uniformBuffer,
        offset: _offset,
        size: _uniformBufferSize
    };

    //* Step - 1B: Buffer Binding Entry
    const bindGroupEntry = 
    {
        binding: _binding,
        resource: bufferBinding
    };

    //* Step - 1C: Buffer Binding Descriptor
    const bindGroupDescriptor = 
    {
        layout: _bindGroupLayout,
        entries:
        [
            bindGroupEntry
        ]
    };

    //* Step - 1D: Bind Group
    bindGroup = device.createBindGroup(bindGroupDescriptor);
    if (bindGroup == null)
    {
        console.log("Failed To Create Bind Group !!!");
        throw Error("Failed To Create Bind Group !!!");
    }
    else
        console.log("Bind Group Successfully Created");

    return bindGroup
}

function resize()
{
    // Code
    if (bFullscreen)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    //* Depth Texture
    if (device != null)
    {
        if (depthTexture != null)
        {
            depthTexture.destroy();
            depthTexture = null; 
        }

        // Depth Texture Descriptor
        const depthTextureDescriptor = 
        {
            size: [canvas.width, canvas.height, 1],
            dimension: "2d",
            format: "depth24plus-stencil8",
            usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC
        };

        // Create Depth Texture
        depthTexture = device.createTexture(depthTextureDescriptor);
        if (depthTexture == null)
        {
            console.log("Failed To Create Depth Texture !!!");
            throw Error("Failed To Create Depth Texture !!!");
        }
    }

    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width) / parseFloat(canvas.height), 0.1, 100.0);
}

function display()
{
    // Code
    if (device == null)
        return;

    //* Step - 10 : Get Command Encoder Of The Device (Created per frame due to async nature)
    const commandEncoder = device.createCommandEncoder();
    if (commandEncoder == null)
    {
        console.log("Failed To Get Command Encoder From Device !!!");
        throw Error("Failed To Get Command Encoder From Device !!!");
    }

    //* Step - 11 : Create Renderpass Color Attachment
    const renderPassColorAttachment = 
    {
        view: context.getCurrentTexture().createView(),
        clearValue: clearColor,
        loadOp: "clear",
        storeOp: "store"
    };

    //* Create Renderpass Depth Attachment
    const renderPassDepthAttachment = 
    {
        view: depthTexture.createView(),
        depthClearValue: 1.0,
        depthLoadOp: "clear",
        depthStoreOp: "store",
        stencilClearValue: 0,
        stencilLoadOp: "clear",
        stencilStoreOp: "store"
    };

    //* Step - 12 : Create Renderpass Descriptor (GPURenderPassDescriptor)
    const renderPassDescriptor = 
    {
        colorAttachments: [renderPassColorAttachment],
        depthStencilAttachment:  renderPassDepthAttachment
    };
    
    //* Cube
    //* -----------------------------------------------------------------------------------------------------------------------------
    //! Transformations
    let modelViewMatrix = mat4.create();
    let modelViewProjectionMatrix = mat4.create();
    let translationMatrix = mat4.create();
    let rotationMatrix = mat4.create();

    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -6.0]);
    mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadians(angle));
    mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadians(angle));
    mat4.rotateZ(rotationMatrix, rotationMatrix, degreeToRadians(angle));

    mat4.multiply(modelViewMatrix, translationMatrix, rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    //! Update Uniform Buffer
    queue.writeBuffer(buffer_mvpUniform_cube, 0, modelViewProjectionMatrix, 0, modelViewProjectionMatrix.length);
    //* -----------------------------------------------------------------------------------------------------------------------------

    //* Step - 13 : Begin The Render Pass
    const renderPassEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
    {
        renderPassEncoder.setPipeline(render_pipeline);
        renderPassEncoder.setViewport(0, 0, canvas.width, canvas.height, 0.0, 1.0);
        renderPassEncoder.setScissorRect(0, 0, canvas.width, canvas.height);

        //* Cube
        renderPassEncoder.setVertexBuffer(0, buffer_position_cube);
        renderPassEncoder.setVertexBuffer(1, buffer_color_cube);
        renderPassEncoder.setBindGroup(0, bindGroup_mvpUniform_cube);
        renderPassEncoder.draw(36);
    }
    renderPassEncoder.end();

    //* Step - 14 : Finish The Command Encoder And Submit To Queue
    queue.submit([commandEncoder.finish()]);

    update();

    //! Animation Loop
    animationFrameId = requestAnimationFrame(display);
}

function update()
{
    // Code
    angle += animationSpeed;
    if (angle > 360.0)
        angle = angle - 360.0;
}

function degreeToRadians(degrees)
{
    return (degrees * (Math.PI / 180.0));
}

function uninitialize()
{
    // Code
    if (animationFrameId != null)
    {
        cancelAnimationFrame(animationFrameId);
        animationFrameId = null;
    }

    if (depthTexture)
    {
        depthTexture.destroy();
        depthTexture = null;
    }

    //* Unconfigure/Destroy Context
    if (context != null)
    {
        context.unconfigure();
        context = null;
    }

    //* Step - 15 : Destroy The Device
    if (device != null)
    {
        device.destroy();
        device = null;
        queue = null;

        buffer_position_cube = null;
        buffer_color_cube = null;
        buffer_mvpUniform_cube = null;
        bindGroup_mvpUniform_cube = null;
        
        render_pipeline = null;
    }

    perspectiveProjectionMatrix = null;
}

function keyDown(event)
{
    // Code
    switch(event.key)
    {
        case 'f':
        case 'F':
            toggleFullScreen();
        break;

        case 'q':
        case 'Q':
            uninitialize();
            window.close(); // Not Applicable For All Browsers
        break;
            
    }
}

function mousedown()
{
    // Code
}

