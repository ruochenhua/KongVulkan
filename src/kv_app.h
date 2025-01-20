#pragma once
#include <memory>

#include "kv_game_object.h"
#include "kv_pipeline.h"
#include "kv_renderer.h"
#include "kv_swap_chain.h"
#include "kv_window.h"

namespace kong
{
    class KongApp
    {
    public:
        KongApp();
        ~KongApp();
    
        KongApp(const KongApp&) = delete;
        KongApp& operator=(const KongApp&) = delete;
        
        void run();

        static constexpr int window_width = 800;
        static constexpr int window_height = 600;
        
    private:
        void loadGameobjects();
        
        KongWindow m_window {window_width, window_height, "kong vulkan"};
        KongDevice m_device{m_window};
        KongRenderer m_renderer{m_window, m_device};
        
        std::vector<KongGameObject> m_gameObjects; 
    };
}
