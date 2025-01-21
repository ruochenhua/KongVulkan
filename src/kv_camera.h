#pragma once
#define GLM_FORCE_RADIANS
// depth from 0 to 1, not -1 to 1 (opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace kong
{
    class KongCamera
    {
        public:
        void SetOrthographicProjection(
            float left, float right, float bottom,
            float top, float near, float far);

        void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

        void SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up = glm::vec3{0, -1, 0});
        void SetViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3{0, -1, 0});
        void SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation);
        
        const glm::mat4& GetProjectionMatrix() const {return m_projectionMatrix;}
        const glm::mat4& GetViewMatrix() const {return m_viewMatrix;}
        
    private:
        glm::mat4 m_projectionMatrix{1.0f};
        glm::mat4 m_viewMatrix{1.0f};
    };
}