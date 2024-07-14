#ifndef ENGINE_H
#define ENGINE_H

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

// c++ библиотеки
#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include <vulkan/vulkan.h>

// dear imgui 
#include "../core/imgui/imgui.h"
#include "../core/imgui/imgui_impl_glfw.h"
#include "../core/imgui/imgui_impl_vulkan.h"

// volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

#define SS(x) ( ((std::stringstream&)(std::stringstream() << x )).str()) // интерполяция строк или c++17 moment

#define LockFPS = true

namespace Engine {
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

#ifdef APP_USE_VULKAN_DEBUG_REPORT
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
	{
		(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
		fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
		return VK_FALSE;
	}
#endif // APP_USE_VULKAN_DEBUG_REPORT

	class Engine {
	public:
		virtual void start() {}
		virtual void update(uint32_t tick) {}
	};

	class Core : public Engine {
	public:
		/*
		* Конструктор и деструктор класса
		*/

		Core(const char* title, const int width, const int height);
		~Core();

		/*
		* Защита от копирования класса
		*/

		Core() = default;
		Core(Core const&) = delete;
		void operator=(Core const&) = delete;
	public:
		/*
		* Настройки вулкана
		*/
		VkAllocationCallbacks* allocator = VK_NULL_HANDLE;
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice = VK_NULL_HANDLE;
		uint32_t queueFamily = (uint32_t) - 1;
		VkQueue queue = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
		VkPipelineCache pipelineCache = VK_NULL_HANDLE;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

		ImGui_ImplVulkanH_Window imguiWindowData;
		int minImageCount = 0;
		bool swapChainRebuild = false;

		// инициализация вулкана
		 void vulkanInitialize(std::vector<const char*> instanceExtensions);
		 void createInstance(std::vector<const char*> instanceExtensions);
		 void selectQueueFamily();
		 void createLogicalDevice();
		 void createDescriptorPool();
		 void createVulkanSurface(ImGui_ImplVulkanH_Window* window, VkSurfaceKHR surface, int width, int height);
		 void cleanupVulkan();
		 void cleanupWindow();
		 void frameRender(ImGui_ImplVulkanH_Window* window, ImDrawData* drawData);
		 void framePresent(ImGui_ImplVulkanH_Window* window);
		 VkPhysicalDevice selectPhysicalDevice();

		// вспомогательные функции
		 bool isExtensionAvailable(const std::vector<VkExtensionProperties>& properties, const char* extension);
		 void callback(int level, const char* description);
		static void checkVkResult(VkResult error);

		/*
		* Настройки окна
		*/

		GLFWwindow* window;
		const char* m_title = "Engine";
		const int m_width = 800;
		const int m_height = 600;

		void window_initialize();
		void mainLoop();

		// Engine 
		virtual void start() override;
		virtual void update(uint32_t tick) override;
	};
}

#endif // ENGINE