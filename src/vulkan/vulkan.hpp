#pragma once
#include "../error.hpp"
#include <iostream>
#include <SDL2/SDL_vulkan.h>
#include "../include/VkBootstrap.hpp"
#include "../constants.hpp"
#include <memory>
#include "../sdl/sdl.hpp"
#include <fstream>
#include <stdexcept>
#include <stddef.h>
#include <glm/glm.hpp>

#define offsetof(s,m) (uint64_t)(&((s*)0)->m)

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

struct GfxPipeline {
    VkShaderModule vertexShaderMod;
    VkShaderModule fragmentShaderMod;
    VkPipelineShaderStageCreateInfo vertexShaderStgInfo;
    VkPipelineShaderStageCreateInfo fragmentShaderStgInfo;
    VkPipelineShaderStageCreateInfo* shaderStages;
    VkVertexInputBindingDescription bindingDesc;
    VkVertexInputAttributeDescription* attrDescs;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAsm;
    VkViewport viewport;
    VkPipelineViewportStateCreateInfo viewportState;
    VkRect2D scissor = {};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    VkPipelineLayout pipelineLayout;
    VkGraphicsPipelineCreateInfo pipelineInfo;
    VkPipeline pipeline;
};

struct RenderPass {
    VkRenderPass renderPass;
    VkAttachmentDescription colorAttachment;
    VkAttachmentReference colorAttachmentRef;
    VkSubpassDescription subpass;
    VkRenderPassCreateInfo renderPassInfo;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState colorBlendAttach;
    VkPipelineColorBlendStateCreateInfo colorBlend;
};

struct Renderer {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkDevice device;
    VkQueue gfxQueue;
    VkSwapchainKHR swapchain;
    RenderPass rp;
    GfxPipeline pipeline;
    vkb::InstanceBuilder instanceBuilder;
    vkb::Instance vkbInstance;
    vkb::PhysicalDevice physicalDevice;
    vkb::Device vkbDevice;
    vkb::Swapchain vkbSwapchain;
    uint32_t gfxQueueFamily;
    std::vector<VkImage> swpImages;
    std::vector<VkImageView> swpImageViews;
    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandPool commandPool;
    std::vector<VkFramebuffer> swapchainFramebuffers;
};

class VkRender {
    private:
        Renderer renderer;
        Window* win;
    public:
        VkRender(Window* window);
        ~VkRender();

        Renderer getRenderer() const;

        VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
        std::vector<char> readShaderFile(const std::string& filename);
        uint32_t findQueueFamilyIndex(VkPhysicalDevice physDev);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};