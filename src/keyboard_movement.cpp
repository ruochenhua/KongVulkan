#include "keyboard_movement.h"

using namespace kong;

void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, KongGameObject& gameObject)
{
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotation.y += 1.0f;
    if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotation.y -= 1.0f;
    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotation.x += 1.0f;
    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotation.x -= 1.0f;

    if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon())
    {
        gameObject.transform.rotation += lookSpeed * dt * normalize(rotation);
    }

    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.0, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0, -forwardDir.x};
    const glm::vec3 upDir{0.0f, -1.0f, 0.0f};

    glm::vec3 moveDir{0.0f, 0.0f, 0.0f};
    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
    {
        gameObject.transform.translation += moveSpeed * dt * normalize(moveDir);
    }
}
