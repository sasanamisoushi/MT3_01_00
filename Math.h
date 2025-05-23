#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Math {
public:

	//平行移動
	Matrix4x4 MakeTranslateMatrix(const Vector3 &translate);

	//拡大縮小
	Matrix4x4 MkeScaleMatrix(const Vector3 &scale);

	//X軸回転行列
	Matrix4x4 MakeRoteXMatrix(float radian);

	// Y軸回転行列
	Matrix4x4 MakeRotateYMatrix(float radian);

	//Z軸回転行列
	Matrix4x4 MakeRotateZMatrix(float radian);

	// 積
	Matrix4x4 Multiply(const Matrix4x4 &m1, const Matrix4x4 &m2);

	// アフィン変換行列
	Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Vector3 &rotate, const Vector3 &translate);

	//座標変換
	Vector3 Transform(const Vector3 &vector, const Matrix4x4 &matrix);

	//遠近投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearZ, float farZ);

	//正規化デバイス座標
	Matrix4x4 MakeViewportMatrix(float width, float height);
};

