#pragma once
#include "math.h"

class Mouse {
public:
    void Initialize();
    void Update(const char *keys);
    const Matrix4x4 &GetViewProjectionMatrix() const { return viewProjection_; }
    const Vector3 &GetPosition() const { return position_; }
    Vector3 &GetRotate() { return rotate_; }

private:
    Vector3 rotate_{ 0.2f, 0.0f, 0.0f };
    Vector3 position_{ 0.0f, 2.0f, -7.5f };

    Matrix4x4 viewProjection_;
};

