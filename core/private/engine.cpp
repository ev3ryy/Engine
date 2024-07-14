#include "../core/public/engine.hpp"
#include "../core/public/engine_logs.hpp"

namespace Engine {
    Core::Core(const char* title, const int width, const int height) {
        // first initialize components of vulkan
        //instance = VK_NULL_HANDLE;
        //physicalDevice = VK_NULL_HANDLE;
        //logicalDevice = VK_NULL_HANDLE;
        //queueFamily = (uint32_t) - 1;
        //queue = VK_NULL_HANDLE;
        //debugReport = VK_NULL_HANDLE;
        //pipelineCache = VK_NULL_HANDLE;
        //descriptorPool = VK_NULL_HANDLE;

        //minImageCount = 2;
        //swapChainRebuild = false;
    }

    Core::~Core() {

    }

    void Core::vulkanInitialize(std::vector<const char*> instanceExtensions) {
        VkResult result;

        createInstance(instanceExtensions);

        physicalDevice = selectPhysicalDevice();

        selectQueueFamily();
        createLogicalDevice();
        createDescriptorPool();
    }

    void Core::callback(int level, const char* description) {
        switch (level) {
        case 0:
            LOG_INFO(SS("Vulkan info: " << description << ".\n")); // Слой информации
            break;
        case 1:
            LOG_WARNING(SS("Vulkan warning: " << description << ".\n")); // Слой предупреждения
            break;
        case 2:
            LOG_ERROR(SS("Vulkan error: " << description << ".\n")); // Слой ошибки
            break;
        case 3:
            LOG_CRITICAL(SS("Vulkan critical error: " << description << ".\n")); // Слой критической ошибки
            break;
        default:
            break;
        }
    }

    void Core::checkVkResult(VkResult error) {
        if (error == 0) return; // Если error = 0, то указываем причину ошибки
        LOG_ERROR(SS("Vulkan error: " << error));
        if(error < 0) {
            abort(); // Если error < 0, то экстренно выходим из программы
        }
    }

    bool Core::isExtensionAvailable(const std::vector<VkExtensionProperties>& properties, const char* extension) {
        for (const auto& i : properties) {
            if (strcmp(extension, i.extensionName) == 0) {
                return true;

                /*
                * Циклом проходимся по всем доступным расширениям
                * strcmp - это сравнивание двух строк, если нашли совпадение, то возвращаем true
                * Если совпадения не найдено - возвращаем false
                */
            }
        }

        return false;
    }

    VkPhysicalDevice Core::selectPhysicalDevice() {
        uint32_t devicesCount; // Количество доступных устройств (видеокарт)
        VkResult result = vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr); // Заполняем devicesCount всеми доступными вариантами
        checkVkResult(result); // Чек на удачное выполнение vkEnumeratePhysicalDevices

        std::vector<VkPhysicalDevice> devices{devicesCount}; // Контейнер со всеми доступными нам устройствами
        result = vkEnumeratePhysicalDevices(instance, &devicesCount, devices.data()); 
        checkVkResult(result);

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties properties; // VkPhysicalDeviceProperties - характеристики устройства
            vkGetPhysicalDeviceProperties(device, &properties); // Получаем характеристики конкретного устройства

            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                return device; // Если устройство является дискретной видеокартой - возвращаем это устройство
            }
        }

        if (devicesCount > 0) {
            return devices[0]; // Если дискретного устройства не найдено, то возвращаем первое надейное устройство
        }

        return VK_NULL_HANDLE; // Возвращаем 0, если не нашли ни одного устройства
    }

    void Core::createInstance(std::vector<const char*> instanceExtensions) {
        VkResult result;

        VkInstanceCreateInfo createInfo{}; // create info экземпляра
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        uint32_t propertiesCount; // Общее количество поддерживаемых расширений экземпляра
        vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, nullptr); // Заполнение propertiesCount

        std::vector<VkExtensionProperties> properties{propertiesCount}; // Создание контейнера с расширениями
        result = vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, properties.data()); // Заполнение контейнера
        checkVkResult(result); // Чек на успешность выполнения vkEnumerateInstanceExtensionProperties

        /*
        * Это расширение выдает новые запросы о функциях и свойствах устройства, а также о свойствах формата, 
        * которые могут быть легко расширены другими расширениями, без каких-либо дополнительных запросов
        */
        if (isExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        /*
        * Это расширение позволяет приложениям контролировать, включаются ли устройства, предоставляющие это расширение, 
        * в результаты подсчета физических устройств
        * Так как устройства, поддерживающие это расширение, не являются полностью совместимыми с вулканом, 
        * загрузчик вулкана не сообщает об этих устройствах, если приложение не запрашивает их
        */
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (isExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

            /*
            * Указывает, что экземпляр будет перечислять доступные физические устройства и группы, совместимые с Vulkan Portability, 
            * в дополнение к физическим устройствам и группам вулкана, перечисляемым по умолчанию
            */
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

#ifdef APP_USE_VULKAN_DEBUG_REPORT // Директива препроцессора, проверяющая на активную сборку дебага (объявляется в engine.hpp)
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" }; // Создание массива слоев валидации
        createInfo.enabledLayerCount = 1; // Количество слоев
        createInfo.ppEnabledLayerNames = layers; // Активные слои
        instanceExtensions.push_back("VK_EXT_debug_report"); // В расширениях указываем наш слой валидации
#endif

        // Создание экземпляра
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();
        result = vkCreateInstance(&createInfo, allocator, &instance);
        checkVkResult(result);

        // Создание логгера
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        assert(vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT; // Выводим только ошибки и предупреждения
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        result = vkCreateDebugReportCallbackEXT(instance, &debug_report_ci, allocator, &debugReport);
        checkVkResult(result);
#endif
    }

    void Core::selectQueueFamily() {
        uint32_t familiesCount; // Общее количество семей
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familiesCount, nullptr); // Находим доступные семьи и записываем в familiesCount

        // Выделяем память под найденные семьи
        VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * familiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familiesCount, queues); // Загружаем в queues доступные семьи

        for (uint32_t i = 0; i < familiesCount; i++) {
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamily = i;
                break;

                /*
                * Циклом проходися по количеству всех семей
                * Если семья = графической очереди, то записываем ее в queueFamily
                * В данном случае используем тольку одну, графическую очередь
                */
            }
        }

        free(queues); // Высвобождаем память, выделенную под queues, т.к результат мы уже получили и записали в queueFamily
        assert(queueFamily != (uint32_t)-1);
    }

    void Core::createLogicalDevice() {
        VkResult result;

        std::vector<const char*> deviceExtensions{"VK_KHR_swapchain"}; // Создаем контейнер с расширением свап чейна

        uint32_t propertiesCount; // Общее количество спецификаций
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertiesCount, nullptr); // Заполняем propertiesCount

        std::vector<VkExtensionProperties> properties{propertiesCount}; // Создаем контейнер всех спецификаций с местом под найденные propertiesCount
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertiesCount, properties.data()); // Заполняем properties

        /*
        * Это расширение позволяет создавать несоответствующую реализацию вулкана поверх другого графического API (например DirectX).
        * Оно выявляет различия между этой реализацией и собственной реализацией вулкана.
        */
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        if (isExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        const float priority[]{ 1.0f }; // Приоритет очереди. Варьируется от 0.1f до 1.0f
        
        VkDeviceQueueCreateInfo queueInfo[1]{}; // createInfo для создание очереди
        queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; 
        queueInfo[0].queueFamilyIndex = queueFamily; // Назначаем текущую очередь
        queueInfo[0].queueCount = 1; // Количество очередей
        queueInfo[0].pQueuePriorities = priority; // Приоритет этой очереди

        VkDeviceCreateInfo createInfo{}; // createInfo для создания логического устройства
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]); // queueInfo
        createInfo.pQueueCreateInfos = queueInfo; // queueInfo
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); // Количество включаемых расширений
        createInfo.ppEnabledExtensionNames = deviceExtensions.data(); // Сами включаемые расширения

        result = vkCreateDevice(physicalDevice, &createInfo, allocator, &logicalDevice); // Создаем логическое устройство
        checkVkResult(result); // Проверяем на валидность vkCreateDevice

        vkGetDeviceQueue(logicalDevice, queueFamily, 0, &queue); // Получаем созданную очередь и записываем в queue
    }

    /*
    * Дескриптор пул - это место, где хранятся указатели на память в куче
    */
    void Core::createDescriptorPool() {
        VkResult result;

        VkDescriptorPoolSize poolSize[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };

        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        createInfo.maxSets = 1;
        createInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSize);
        createInfo.pPoolSizes = poolSize;

        result = vkCreateDescriptorPool(logicalDevice, &createInfo, allocator, &descriptorPool);
        checkVkResult(result);
    }

    void Core::createVulkanSurface(ImGui_ImplVulkanH_Window* window, VkSurfaceKHR surface, int width, int height) {
        window->Surface = surface;

        // check for window system integration (WSI) support
        VkBool32 result;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamily, window->Surface, &result);
        if (result != VK_TRUE) {
            callback(3, "device not support WSI");
            exit(-1);
        }

        const VkFormat requestSurfaceImageFormat[]{ VK_FORMAT_B8G8R8A8_UNORM,VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        window->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice, window->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

#ifdef LockFPS
        VkPresentModeKHR presentModes[]{ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
        VkPresentModeKHR presentModes[]{ VK_PRESENT_MODE_FIFO_KHR };
#endif
        window->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physicalDevice, window->Surface, &presentModes[0], IM_ARRAYSIZE(presentModes));

        IM_ASSERT(minImageCount >= window->ImageCount);
        ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, logicalDevice, window, queueFamily, allocator, width, height, minImageCount);
    }

    void Core::cleanupVulkan() {
        vkDestroyDescriptorPool(logicalDevice, descriptorPool, allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto vkDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        vkDestroyDebugReportCallback(instance, debugReport, allocator);
#endif

        vkDestroyDevice(logicalDevice, allocator);
        vkDestroyInstance(instance, allocator);
    }

    void Core::cleanupWindow() {
        ImGui_ImplVulkanH_DestroyWindow(instance, logicalDevice, &imguiWindowData, allocator);
    }

    void Core::frameRender(ImGui_ImplVulkanH_Window* window, ImDrawData* drawData) {
        VkResult result;

        VkSemaphore image_acquired_semaphore = window->FrameSemaphores[window->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = window->FrameSemaphores[window->SemaphoreIndex].RenderCompleteSemaphore;

        result = vkAcquireNextImageKHR(logicalDevice, window->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &window->FrameIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            swapChainRebuild = true;
            return;
        }
        checkVkResult(result);

        ImGui_ImplVulkanH_Frame* frame = &window->Frames[window->FrameIndex];
        {
            result = vkWaitForFences(logicalDevice, 1, &frame->Fence, VK_TRUE, UINT64_MAX);
            checkVkResult(result);

            result = vkResetFences(logicalDevice, 1, &frame->Fence);
            checkVkResult(result);
        }
        {
            result = vkResetCommandPool(logicalDevice, frame->CommandPool, 0);
            checkVkResult(result);

            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            
            result = vkBeginCommandBuffer(frame->CommandBuffer, &info);
            checkVkResult(result);
        }
        {
            VkRenderPassBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = window->RenderPass;
            info.framebuffer = frame->Framebuffer;
            info.renderArea.extent.width = window->Width;
            info.renderArea.extent.height = window->Height;
            info.clearValueCount = 1;
            info.pClearValues = &window->ClearValue;
            vkCmdBeginRenderPass(frame->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        ImGui_ImplVulkan_RenderDrawData(drawData, frame->CommandBuffer);

        vkCmdEndRenderPass(frame->CommandBuffer);
        {
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &waitStage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &frame->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            result = vkEndCommandBuffer(frame->CommandBuffer);
            checkVkResult(result);
            result = vkQueueSubmit(queue, 1, &info, frame->Fence);
            checkVkResult(result);
        }
    }

    void Core::framePresent(ImGui_ImplVulkanH_Window* window) {
        if (swapChainRebuild) {
            return;
        }

        VkSemaphore render_complete_semaphore = window->FrameSemaphores[window->SemaphoreIndex].RenderCompleteSemaphore;

        VkPresentInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &window->Swapchain;
        info.pImageIndices = &window->FrameIndex;

        VkResult result;
        result = vkQueuePresentKHR(queue, &info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            swapChainRebuild = true;
            return;
        }

        checkVkResult(result);
        window->SemaphoreIndex = (window->SemaphoreIndex + 1) % window->SemaphoreCount;
    }

    void Core::update(uint32_t tick) {
        //LOG_INFO(SS("Current tick: " << tick << ".\n"));
    }

    void Core::start() {

    }
}

int main(int, char**)
{
#ifdef NDEBUG
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);
#endif // NDEBUG

    static auto core = std::make_unique<Engine::Core>();

    if (!glfwInit())
        return 1;

    // Create window with Vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vulkan Engine", nullptr, nullptr);
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }

    std::vector<const char*> extensions;
    uint32_t extensionsCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    
    for (uint32_t i = 0; i < extensionsCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }
    core->vulkanInitialize(extensions);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(core->instance, window, core->allocator, &surface);
    core->checkVkResult(result);

    // Create Framebuffers
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ImGui_ImplVulkanH_Window* imguiWindow = &core->imguiWindowData;
    core->createVulkanSurface(imguiWindow, surface, width, height);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 15, NULL, io.Fonts->GetGlyphRangesCyrillic());
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = core->instance;
    info.PhysicalDevice = core->physicalDevice;
    info.Device = core->logicalDevice;
    info.QueueFamily = core->queueFamily;
    info.Queue = core->queue;
    info.PipelineCache = core->pipelineCache;
    info.DescriptorPool = core->descriptorPool;
    info.RenderPass = imguiWindow->RenderPass;
    info.Subpass = 0;
    info.MinImageCount = core->minImageCount;
    info.ImageCount = imguiWindow->ImageCount;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.Allocator = core->allocator;
    info.CheckVkResultFn = core->checkVkResult;
    ImGui_ImplVulkan_Init(&info);

    bool showDemoWindow = true;
    bool showAnotherWindow = false;
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    uint32_t tick = 0;
    // основной цикл
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        /*core->update(tick);
        tick++;*/
        // надеюсь будем расширять окно
        int frameWidth, frameHeight;
        glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
        if (frameWidth > 0 && frameHeight > 0 && (core->swapChainRebuild || core->imguiWindowData.Width != frameWidth || core->imguiWindowData.Height != frameHeight))
        {
            ImGui_ImplVulkan_SetMinImageCount(core->minImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(core->instance, core->physicalDevice, core->logicalDevice, &core->imguiWindowData, core->queueFamily, core->allocator, frameWidth, frameHeight, core->minImageCount);
            core->imguiWindowData.FrameIndex = 0;
            core->swapChainRebuild = false;
        }


        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Dear ImGui");

            ImGui::Text(u8"Просто текст");

            ImGui::SliderFloat(u8"Типа флоат", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3(u8"Типа цвет", (float*)&clearColor);

            if (ImGui::Button(u8"Кнопочка *тык*"))
                counter++;

            ImGui::SameLine();
            ImGui::Text(u8"Счетчик = %d", counter);

            if (ImGui::Button(u8"Выйти"))
                goto shutdown;

            ImGui::Text(u8"Фпс: %.1f", io.Framerate);

            ImGui::End();
        }

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            imguiWindow->ClearValue.color.float32[0] = clearColor.x * clearColor.w;
            imguiWindow->ClearValue.color.float32[1] = clearColor.y * clearColor.w;
            imguiWindow->ClearValue.color.float32[2] = clearColor.z * clearColor.w;
            imguiWindow->ClearValue.color.float32[3] = clearColor.w;
            core->frameRender(imguiWindow, draw_data);
            core->framePresent(imguiWindow);
        }
    }

shutdown:
    result = vkDeviceWaitIdle(core->logicalDevice);
    core->checkVkResult(result);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    core->cleanupWindow();
    core->cleanupVulkan();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
