#include "Mouse.h"
#include <Novice.h>
#include <numbers>
#include <algorithm>


using std::numbers::pi_v;

extern Math math_; 

void Mouse::Initialize() {
    rotate_ = { 0.2f, 0.0f, 0.0f };
    position_ = { 0.0f, 2.0f, -7.5f };
}

void Mouse::Update(const char *keys) {
    static int preMouseX = 0, preMouseY = 0;

    int mouseX = 0, mouseY = 0;
    Novice::GetMousePosition(&mouseX, &mouseY);
    int deltaX = mouseX - preMouseX;
    int deltaY = mouseY - preMouseY;

    // 回転（右クリック）
    if (Novice::IsPressMouse(1)) {
        const float sensitivity = 0.005f;
        rotate_.y += deltaX * sensitivity;
        rotate_.x += deltaY * sensitivity;
        rotate_.x = std::clamp(rotate_.x, -pi_v<float> / 2.0f + 0.01f, pi_v<float> / 2.0f - 0.01f);
    }

    // パン移動（Shift + 右クリック）
    if (Novice::IsPressMouse(1) && keys[DIK_LSHIFT]) {
        const float panSpeed = 0.01f;
        Vector3 right = {
            cosf(rotate_.y),
            0.0f,
            -sinf(rotate_.y)
        };
        Vector3 up = { 0.0f, 1.0f, 0.0f };

        position_.x -= (right.x * deltaX + up.x * deltaY) * panSpeed;
        position_.y -= (right.y * deltaX + up.y * deltaY) * panSpeed;
        position_.z -= (right.z * deltaX + up.z * deltaY) * panSpeed;
    }

    // ズーム（ホイール）
    int wheel = Novice::GetWheel();
    if (wheel != 0) {
        Vector3 forward = {
            -sinf(rotate_.y),
            0.0f,
            -cosf(rotate_.y)
        };
        const float zoomSpeed = 0.05f;
        float zoom = static_cast<float>(wheel) / 120.0f;  // 通常化
        position_.x += forward.x * zoom * zoomSpeed;
        position_.z += forward.z * zoom * zoomSpeed;
    }

    Matrix4x4 cameraMatrix = math_.MakeAffineMatrix({ 1,1,1 }, rotate_, position_);
    Matrix4x4 viewMatrix = math_.Inverse(cameraMatrix);
    Matrix4x4 projectionMatrix = math_.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
    viewProjection_ = math_.Multiply(viewMatrix, projectionMatrix);

    preMouseX = mouseX;
    preMouseY = mouseY;
}


