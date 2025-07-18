#pragma once
#include "math.h"

class Mouse {
public:
	void UpdateCameraByMouse(Vector3 &cameraRotate, int mouseX, int mouseY, int preMouseX, int preMouseY, bool isDragging);
	void UpdateCameraZoom(Vector3 &cameraTranslate, float wheelDelta);
};

