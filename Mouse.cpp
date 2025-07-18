#include "Mouse.h"
#include <algorithm>
#include <numbers>

using std::numbers::pi_v;

void Mouse::UpdateCameraByMouse(Vector3 &cameraRotate, int mouseX, int mouseY, int preMouseX, int preMouseY, bool isDragging) {
	if (!isDragging) return;

	int dx = mouseX - preMouseX;
	int dy = mouseY - preMouseY;

	cameraRotate.y += dx * 0.01f;  // 左右（Yaw）
	cameraRotate.x += dy * 0.01f;  // 上下（Pitch）

	// 上下回転制限
	cameraRotate.x = std::clamp(cameraRotate.x, -pi_v<float> / 2.0f + 0.01f, pi_v<float> / 2.0f - 0.01f);
}

void Mouse::UpdateCameraZoom(Vector3 &cameraTranslate, float wheelDelta) {
	cameraTranslate.z += wheelDelta * 0.1f;  // ホイール感度調整
}