#pragma once
#include <memory>

#include "kv_model.h"
#include "glm/ext/matrix_transform.hpp"

namespace kong
{
    struct Transform2Dcomponent
    {
        glm::vec2 translation{};
        glm::vec2 scale{1.0, 1.0};
        float rotation{0.0f};
        glm::mat2 mat2()
        {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);

            glm::mat2 rotationMat{{c, s}, {-s, c}};
            
            glm::mat2 scaleMat {{scale.x, 0.0}, {0.0, scale.y}};
            return rotationMat * scaleMat;
        }
    };
    class KongGameObject
    {
    public:
        using id_t = unsigned int;

        static KongGameObject CreateGameObject()
        {
            static id_t currentId = 0;
            return KongGameObject{currentId++};
        }

        // 禁用copy操作
        KongGameObject(const KongGameObject&) = delete;
        KongGameObject& operator=(const KongGameObject&) = delete;
        // move操作使用默认
        KongGameObject(KongGameObject&&) = default;
        KongGameObject& operator=(KongGameObject&&) = default;
        
        id_t getId() const {return id;}

        std::shared_ptr<KongModel> model{};
        glm::vec3 color{};
        Transform2Dcomponent transform2d{};
    private:
        KongGameObject(id_t objId) : id(objId) {};

        id_t id;
    };
    
}
