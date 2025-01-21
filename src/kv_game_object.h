#pragma once
#include <memory>

#include "kv_model.h"
#include "glm/ext/matrix_transform.hpp"

namespace kong
{
    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.0, 1.0, 1.0};
        glm::vec3 rotation{0.0f};
        
        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
            {
                scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.0f,
            },
            {translation.x, translation.y, translation.z, 1.0f}};
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
        TransformComponent transform{};
    private:
        KongGameObject(id_t objId) : id(objId) {};

        id_t id;
    };
    
}
