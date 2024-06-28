#include <iostream>
#include "render.hpp"

const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
};

int main(int argc, char** argv) {
    Window window;
    VkRender render(&window);

    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkSemaphore> imageAvailableSemaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> renderFinishedSemaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkFence> inFlightFences(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaInfo = {};
    semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto renderer = render.getRenderer();
        if (vkCreateSemaphore(renderer.device, &semaInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(renderer.device, &semaInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
                std::cerr << "CREATE SEMA ERR" << std::endl;
                window.~Window(); // kill yourself! (literally)
                render.~VkRender(); // kill yourself! (literally)
                THROW();
            }

        if (vkCreateFence(renderer.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            std::cerr << "CREATE FENCE ERR" << std::endl;
            window.~Window(); // kill yourself! (literally)
            render.~VkRender(); // kill yourself! (literally)
            THROW();
        }
    }

    VkBuffer vertexBuf;
    VkDeviceMemory vertexBufMem;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(render.getRenderer().device, &bufferInfo, nullptr, &vertexBuf) != VK_SUCCESS) {
            std::cerr << "CREATE BUFFER ERR" << std::endl;
            window.~Window(); // kill yourself! (literally)
            render.~VkRender(); // kill yourself! (literally)
            THROW();
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(render.getRenderer().device, vertexBuf, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = render.findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(render.getRenderer().device, &allocInfo, nullptr, &vertexBufMem) != VK_SUCCESS) {
        std::cerr << "ALLOCATE BUFFER MEM ERR" << std::endl;
        window.~Window(); // kill yourself! (literally)
        render.~VkRender(); // kill yourself! (literally)
        THROW();
    }

    vkBindBufferMemory(render.getRenderer().device, vertexBuf, vertexBufMem, 0);

    void* data;
    vkMapMemory(render.getRenderer().device, vertexBufMem, 0, bufferInfo.size, 0, &data);
    std::memcpy(data, vertices.data(), (std::size_t) bufferInfo.size);
    vkUnmapMemory(render.getRenderer().device, vertexBufMem);

    int currentFrame = 0;
    bool running = true;
    SDL_Event event;
    while (running) {
        Renderer renderer = render.getRenderer();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        vkWaitForFences(renderer.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(renderer.device, 1, &inFlightFences[currentFrame]);

        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(renderer.device, renderer.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            //shit
            std::cout << "outta date!" << std::endl;
        } else if (result != VK_SUCCESS) {
            std::cerr << "SWAPCHAIN ACQUIRE NEXT IMAGE ERR" << std::endl;
            window.~Window();
            render.~VkRender();
            THROW();
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        vkBeginCommandBuffer(renderer.commandBuffers[imageIndex], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderer.rp.renderPass;
        renderPassInfo.framebuffer = renderer.swapchainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = renderer.vkbSwapchain.extent;

        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(renderer.commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(renderer.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline.pipeline);

        VkBuffer vertexBuffers[] = { vertexBuf };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(renderer.commandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);

        vkCmdDraw(renderer.commandBuffers[imageIndex], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdEndRenderPass(renderer.commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(renderer.commandBuffers[imageIndex]) != VK_SUCCESS) {
            std::cerr << "END CMD BUFFER ERR" << std::endl;
            window.~Window();
            render.~VkRender();
            THROW();
        }

        VkSubmitInfo subInfo = {};
        subInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemas[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        //subInfo.waitSemaphoreCount = 2;
        //subInfo.pWaitSemaphores = waitSemas;
        //subInfo.pWaitDstStageMask = waitStages;

        subInfo.commandBufferCount = 1;
        subInfo.pCommandBuffers = &renderer.commandBuffers[imageIndex];

        VkSemaphore signalSemas[] = { renderFinishedSemaphores[currentFrame] };
        //subInfo.signalSemaphoreCount = 1;
        //subInfo.pSignalSemaphores = signalSemas;

        vkResetFences(renderer.device, 1, &inFlightFences[currentFrame]);

        //std::cout << *renderer.commandBuffers[imageIndex] << std::endl;

        if (vkQueueSubmit(renderer.gfxQueue, 1, &subInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            std::cerr << "GFX QUEUE SUBMIT ERR" << std::endl;
            window.~Window();
            render.~VkRender();
            THROW();
        }

        VkPresentInfoKHR presInfo = {};
        presInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        //presInfo.waitSemaphoreCount = 1;
        //presInfo.pWaitSemaphores = signalSemas;

        VkSwapchainKHR swapchains[] = { renderer.swapchain };
        presInfo.swapchainCount = 1;
        presInfo.pSwapchains = swapchains;
        presInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(renderer.gfxQueue, &presInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            //boring
            std::cout << "outta date!" << std::endl;
        } else if (result != VK_SUCCESS) { //fun
            std::cerr << "GFX QUEUE PRESENT ERR" << std::endl;
            window.~Window();
            render.~VkRender();
            THROW();
        }

        std::cout << "frame " << currentFrame << std::endl;
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(render.getRenderer().device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(render.getRenderer().device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(render.getRenderer().device, inFlightFences[i], nullptr);
    }

    return EXIT_SUCCESS;
}