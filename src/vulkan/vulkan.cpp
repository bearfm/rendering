#include "vulkan.hpp"

#include "../sdl/sdl.hpp"
#include "../main/render.hpp"

VkRender::VkRender(Window* window) {
    this->win = window;

    auto instanceBuilderRet = renderer.instanceBuilder.set_app_name(NAME)
        .request_validation_layers(true)
        .require_api_version(1,1,0)
        .use_default_debug_messenger()
        .set_headless(false)
        .build();

    if (!instanceBuilderRet) { std::cerr << instanceBuilderRet.error().message() << std::endl; THROW(); }
    renderer.vkbInstance = instanceBuilderRet.value();

    renderer.instance = renderer.vkbInstance.instance;

    renderer.instanceBuilder.use_default_debug_messenger();

    renderer.instanceBuilder.request_validation_layers();

    auto systemInfoRet = vkb::SystemInfo::get_system_info();
    if (!systemInfoRet) { std::cerr << systemInfoRet.error().message() << std::endl; THROW(); }

    auto systemInfo = systemInfoRet.value();
    if (systemInfo.is_layer_available("VK_LAYER_LUNARG_api_dump")) {
        renderer.instanceBuilder.enable_layer("VK_LAYER_LUNARG_api_dump");
    }

    if (systemInfo.validation_layers_available) {
        renderer.instanceBuilder.enable_validation_layers();
    }

    if (!SDL_Vulkan_CreateSurface(this->win->getContext().window, renderer.instance, &renderer.surface)) {
        std::cerr << "SDL_VULKAN_CREATESURFACE ERR: " << SDL_GetError() << std::endl;
        this->~VkRender(); // kill yourself! (literally)
        THROW();
    }

    vkb::PhysicalDeviceSelector selector{ renderer.vkbInstance };
    renderer.physicalDevice = selector
        .set_surface(renderer.surface)
        .select()
        .value();

    vkb::DeviceBuilder deviceBuilder{ renderer.physicalDevice };
    renderer.vkbDevice = deviceBuilder.build().value();

    renderer.device = renderer.vkbDevice.device;
    renderer.gfxQueue = renderer.vkbDevice.get_queue(vkb::QueueType::graphics).value();
    renderer.gfxQueueFamily = renderer.vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    vkb::SwapchainBuilder swpBuilder{ renderer.vkbDevice, renderer.surface };
    renderer.vkbSwapchain = swpBuilder
        .build()
        .value();

    renderer.vkbSwapchain.present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    renderer.swapchain = renderer.vkbSwapchain.swapchain;

    renderer.swpImages = renderer.vkbSwapchain.get_images().value();
    renderer.swpImageViews = renderer.vkbSwapchain.get_image_views().value();

    //render pass finally
    renderer.rp.colorAttachment = {};
    renderer.rp.colorAttachment.format = renderer.vkbSwapchain.image_format;
    renderer.rp.colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    renderer.rp.colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderer.rp.colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    renderer.rp.colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderer.rp.colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderer.rp.colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderer.rp.colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderer.rp.colorAttachmentRef = {};
    renderer.rp.colorAttachmentRef.attachment = 0;
    renderer.rp.colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderer.rp.subpass = {};
    renderer.rp.subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    renderer.rp.subpass.colorAttachmentCount = 1;
    renderer.rp.subpass.pColorAttachments = &renderer.rp.colorAttachmentRef;

    renderer.rp.renderPassInfo = {};
    renderer.rp.renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderer.rp.renderPassInfo.attachmentCount = 1;
    renderer.rp.renderPassInfo.pAttachments = &renderer.rp.colorAttachment;
    renderer.rp.renderPassInfo.subpassCount = 1;
    renderer.rp.renderPassInfo.pSubpasses = &renderer.rp.subpass;

    if (vkCreateRenderPass(renderer.device, &renderer.rp.renderPassInfo, nullptr, &renderer.rp.renderPass) != VK_SUCCESS) {
        std::cerr << "RENDER PASS CREATE ERR" << std::endl;
        this->~VkRender(); // kill yourself! (literally)
        SDL_DestroyWindow(this->win->getContext().window);
        SDL_Quit();
        THROW();
    }

    renderer.pipeline.vertexShaderMod = createShaderModule(renderer.device, readShaderFile("shaders/vert.vert.spv"));
    renderer.pipeline.fragmentShaderMod = createShaderModule(renderer.device, readShaderFile("shaders/frag.frag.spv"));

    renderer.pipeline.vertexShaderStgInfo = {};
    renderer.pipeline.vertexShaderStgInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    renderer.pipeline.vertexShaderStgInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    renderer.pipeline.vertexShaderStgInfo.module = renderer.pipeline.vertexShaderMod;
    renderer.pipeline.vertexShaderStgInfo.pName = "main";

    renderer.pipeline.fragmentShaderStgInfo = {};
    renderer.pipeline.fragmentShaderStgInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    renderer.pipeline.fragmentShaderStgInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    renderer.pipeline.fragmentShaderStgInfo.module = renderer.pipeline.vertexShaderMod;
    renderer.pipeline.fragmentShaderStgInfo.pName = "main";

    renderer.pipeline.shaderStages = new VkPipelineShaderStageCreateInfo[2];
    renderer.pipeline.shaderStages[0] = renderer.pipeline.vertexShaderStgInfo;
    renderer.pipeline.shaderStages[1] = renderer.pipeline.fragmentShaderStgInfo;

    renderer.pipeline.bindingDesc = {};
    renderer.pipeline.bindingDesc.binding = 0;
    renderer.pipeline.bindingDesc.stride = sizeof(Vertex);
    renderer.pipeline.bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    renderer.pipeline.attrDescs = new VkVertexInputAttributeDescription[2];
    renderer.pipeline.attrDescs[0].binding = 0;
    renderer.pipeline.attrDescs[0].location = 0;
    renderer.pipeline.attrDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    renderer.pipeline.attrDescs[0].offset = offsetof(Vertex, position);

    renderer.pipeline.attrDescs[1].binding = 0;
    renderer.pipeline.attrDescs[1].location = 1;
    renderer.pipeline.attrDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    renderer.pipeline.attrDescs[1].offset = offsetof(Vertex, color);

    renderer.pipeline.vertexInputInfo = {};
    renderer.pipeline.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    renderer.pipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    renderer.pipeline.vertexInputInfo.pVertexBindingDescriptions = &renderer.pipeline.bindingDesc;
    renderer.pipeline.vertexInputInfo.vertexAttributeDescriptionCount = 2;
    renderer.pipeline.vertexInputInfo.pVertexAttributeDescriptions = renderer.pipeline.attrDescs;

    renderer.pipeline.inputAsm = {};
    renderer.pipeline.inputAsm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    renderer.pipeline.inputAsm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    renderer.pipeline.inputAsm.primitiveRestartEnable = VK_FALSE;

    renderer.pipeline.viewport = {};
    renderer.pipeline.viewport.x = 0.0f;
    renderer.pipeline.viewport.y = 0.0f;
    renderer.pipeline.viewport.width = (float) renderer.vkbSwapchain.extent.width;
    renderer.pipeline.viewport.height = (float) renderer.vkbSwapchain.extent.height;
    renderer.pipeline.viewport.minDepth = 0.0f;
    renderer.pipeline.viewport.maxDepth = 1.0f;

    renderer.pipeline.scissor.offset = { 0, 0 };
    renderer.pipeline.scissor.extent = renderer.vkbSwapchain.extent;

    renderer.pipeline.viewportState = {};
    renderer.pipeline.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    renderer.pipeline.viewportState.viewportCount = 1;
    renderer.pipeline.viewportState.pViewports = &renderer.pipeline.viewport;
    renderer.pipeline.viewportState.scissorCount = 1;
    renderer.pipeline.viewportState.pScissors = &renderer.pipeline.scissor;

    renderer.rp.rasterizer = {};
    renderer.rp.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    renderer.rp.rasterizer.depthClampEnable = VK_FALSE;
    renderer.rp.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    renderer.rp.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    renderer.rp.rasterizer.lineWidth = 1.0f;
    renderer.rp.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    renderer.rp.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    renderer.rp.rasterizer.depthBiasEnable = VK_FALSE;

    renderer.rp.multisampling = {};
    renderer.rp.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    renderer.rp.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    renderer.rp.colorBlendAttach = {};
    renderer.rp.colorBlendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    renderer.rp.colorBlendAttach.blendEnable = VK_FALSE;

    renderer.rp.colorBlend = {};
    renderer.rp.colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    renderer.rp.colorBlend.logicOpEnable = VK_FALSE;
    renderer.rp.colorBlend.attachmentCount = 1;
    renderer.rp.colorBlend.pAttachments = &renderer.rp.colorBlendAttach;

    renderer.pipeline.pipelineLayoutInfo = {};
    renderer.pipeline.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    renderer.pipeline.pipelineLayoutInfo.setLayoutCount = 0;
    renderer.pipeline.pipelineLayoutInfo.pSetLayouts = nullptr;
    renderer.pipeline.pipelineLayoutInfo.pushConstantRangeCount = 0;
    renderer.pipeline.pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(renderer.device, &renderer.pipeline.pipelineLayoutInfo, nullptr, &renderer.pipeline.pipelineLayout) != VK_SUCCESS) {
        std::cerr << "RENDER PIPELINE LAYOUT CREATE ERR" << std::endl;
        this->~VkRender(); // kill yourself! (literally)
        SDL_DestroyWindow(this->win->getContext().window);
        SDL_Quit();
        THROW();
    }

    renderer.pipeline.pipelineInfo = {};
    renderer.pipeline.pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    renderer.pipeline.pipelineInfo.stageCount = 2;
    renderer.pipeline.pipelineInfo.pStages = renderer.pipeline.shaderStages;
    renderer.pipeline.pipelineInfo.pVertexInputState = &renderer.pipeline.vertexInputInfo;
    renderer.pipeline.pipelineInfo.pInputAssemblyState = &renderer.pipeline.inputAsm;
    renderer.pipeline.pipelineInfo.pViewportState = &renderer.pipeline.viewportState;
    renderer.pipeline.pipelineInfo.pRasterizationState = &renderer.rp.rasterizer;
    renderer.pipeline.pipelineInfo.pMultisampleState = &renderer.rp.multisampling;
    renderer.pipeline.pipelineInfo.pColorBlendState = &renderer.rp.colorBlend;
    renderer.pipeline.pipelineInfo.layout = renderer.pipeline.pipelineLayout;
    renderer.pipeline.pipelineInfo.renderPass = renderer.rp.renderPass;
    renderer.pipeline.pipelineInfo.subpass = 0;
    renderer.pipeline.pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(renderer.device, VK_NULL_HANDLE, 1, &renderer.pipeline.pipelineInfo, nullptr, &renderer.pipeline.pipeline) != VK_SUCCESS) {
        std::cerr << "RENDER PIPELINE CREATE ERR" << std::endl;
        this->~VkRender(); // kill yourself! (literally)
        SDL_DestroyWindow(this->win->getContext().window);
        SDL_Quit();
        THROW();
    }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = VkRender::findQueueFamilyIndex(renderer.physicalDevice);
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(renderer.device, &poolInfo, nullptr, &renderer.commandPool) != VK_SUCCESS) {
        std::cerr << "CREATE COMMAND POOL ERR" << std::endl;
        this->~VkRender(); // kill yourself! (literally)
        THROW();
    }

    renderer.commandBuffers = std::vector<VkCommandBuffer>(renderer.vkbSwapchain.get_images().value().size());
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(renderer.commandBuffers.size());

    if (vkAllocateCommandBuffers(renderer.device, &allocInfo, renderer.commandBuffers.data()) != VK_SUCCESS) {
        std::cerr << "CREATE COMMAND BUFFERS ERR" << std::endl;
        this->~VkRender(); // kill yourself! (literally)
        THROW();
    }

    renderer.swapchainFramebuffers = std::vector<VkFramebuffer>(renderer.vkbSwapchain.get_image_views().value().size());

    for (std::size_t i = 0; i < renderer.vkbSwapchain.get_image_views().value().size(); i++) {
        VkImageView attachments[] = {
            renderer.vkbSwapchain.get_image_views().value()[i]
        };

        VkFramebufferCreateInfo fbInfo = {};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = renderer.rp.renderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = attachments;
        fbInfo.width = renderer.vkbSwapchain.extent.width;
        fbInfo.height = renderer.vkbSwapchain.extent.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(renderer.device, &fbInfo, nullptr, &renderer.swapchainFramebuffers[i]) != VK_SUCCESS) {
            std::cerr << "CREATE FRAMEBUFFER ERR" << std::endl;
            this->~VkRender(); // kill yourself! (literally)
            THROW();
        }
    }

    vkDestroyShaderModule(renderer.device, renderer.pipeline.vertexShaderMod, nullptr);
    vkDestroyShaderModule(renderer.device, renderer.pipeline.fragmentShaderMod, nullptr);
}

VkRender::~VkRender() {
    std::cout << "Deconstructing VkRender at line " << __LINE__ << " in " << __FILE__ << std::endl;
    vkDestroyCommandPool(renderer.device, renderer.commandPool, nullptr);
    vkb::destroy_instance(renderer.vkbInstance);
    vkDestroyShaderModule(renderer.device, renderer.pipeline.vertexShaderMod, nullptr);
    vkDestroyShaderModule(renderer.device, renderer.pipeline.fragmentShaderMod, nullptr);
    vkDestroyInstance(renderer.instance, nullptr);
    delete[] renderer.pipeline.shaderStages;
    delete[] renderer.pipeline.attrDescs;
}

Renderer VkRender::getRenderer() const {
    return renderer;
}

VkShaderModule VkRender::createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> VkRender::readShaderFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file!");
    }

    std::size_t fileSize = (std::size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

uint32_t VkRender::findQueueFamilyIndex(VkPhysicalDevice physDev) {
    uint32_t queueFamCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamCount, queueFamilies.data());

    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { // idk
            return i;
        }
    }

    std::cerr << "FIND QUEUE FAMILY ERR" << std::endl;
    this->~VkRender(); // kill yourself! (literally)
    THROW();
}

uint32_t VkRender::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(renderer.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    std::cerr << "FIND MEMORY TYPE ERR" << std::endl;
    this->~VkRender(); // kill yourself! (literally)
    THROW();
}