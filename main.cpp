
#include <Novice.h>

#include <numbers>
#include <imgui.h>
#include "math.h"
#include <algorithm>

using namespace KamataEngine::MathUtility;

using std::numbers::pi_v;

const char kWindowTitle[] = "LE2C_07_sasnami_sousi";



struct AABB {
	Vector3 min;
	Vector3 max;
	uint32_t color;
};

struct Sphere {
	Vector3 center; //中心点
	float radius;   //半径
	Vector3 color;
};


Math math_;

uint32_t ConvertColor(const Vector3 &color) {
	return (static_cast<int>(color.x * 255.0f) << 24) |
		(static_cast<int>(color.y * 255.0f) << 16) |
		(static_cast<int>(color.z * 255.0f) << 8) | 0xFF;
}

Vector3 Perpendicular(const Vector3 &vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y, vector.x, 0.0f };
	}
	return { 0.0f, -vector.z, vector.y };
}

Vector3 Normalize(const Vector3 &v) {
	float length = Length(v);
	if (length == 0.0f) return { 0.0f, 0.0f, 0.0f };
	return { v.x / length, v.y / length, v.z / length };
}

Vector3 cross(const Vector3 &v1, const Vector3 &v2) {
	Vector3 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

void DrawGrid(const Matrix4x4 &viewProjectionMatrix, const Matrix4x4 &viewportMatrix) {
	const float kGridHalfWidth = 2.0f;                                      //Gridの半分の幅
	const uint32_t kSubdivison = 10;                                        //分離数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivison);  //1つ分の長さ

	//奥から手前への線を順々に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivison; ++xIndex) {
		//ワールド座標系上の始点と終点を求める
		float x = -kGridHalfWidth + xIndex * kGridEvery;
		Vector3 start = { x, 0.0f, -kGridHalfWidth };
		Vector3 end = { x, 0.0f, kGridHalfWidth };

		//スクリーン座標系まで変換をかける
		start = math_.Transform(start, viewProjectionMatrix);
		start = math_.Transform(start, viewportMatrix);
		end = math_.Transform(end, viewProjectionMatrix);
		end = math_.Transform(end, viewportMatrix);

		//変換した座標を使って
		Novice::DrawLine(static_cast<int>(start.x), static_cast<int>(start.y),
			static_cast<int>(end.x), static_cast<int>(end.y), 0xAAAAAAFF);



	}
	//左から右も同じように順々に引いていく
	for (uint32_t zIndex = 0; zIndex <= kSubdivison; ++zIndex) {
		float z = -kGridHalfWidth + zIndex * kGridEvery;
		Vector3 start = { -kGridHalfWidth, 0.0f, z };
		Vector3 end = { kGridHalfWidth, 0.0f, z };

		// ワールド→スクリーン変換
		start = math_.Transform(start, viewProjectionMatrix);
		start = math_.Transform(start, viewportMatrix);
		end = math_.Transform(end, viewProjectionMatrix);
		end = math_.Transform(end, viewportMatrix);

		// 線を描画
		Novice::DrawLine(static_cast<int>(start.x), static_cast<int>(start.y),
			static_cast<int>(end.x), static_cast<int>(end.y), 0xAAAAAAFF); // グレー
	}

}

void DrawAABB(const AABB &aabb, const Matrix4x4 &viewProjectionMatrix, const Matrix4x4 &viewportMatrix, uint32_t color) {
	Vector3 vertices[8] = {
		{ aabb.min.x, aabb.min.y, aabb.min.z },
		{ aabb.max.x, aabb.min.y, aabb.min.z },
		{ aabb.max.x, aabb.max.y, aabb.min.z },
		{ aabb.min.x, aabb.max.y, aabb.min.z },
		{ aabb.min.x, aabb.min.y, aabb.max.z },
		{ aabb.max.x, aabb.min.y, aabb.max.z },
		{ aabb.max.x, aabb.max.y, aabb.max.z },
		{ aabb.min.x, aabb.max.y, aabb.max.z }
	};

	for (int i = 0; i < 8; ++i) {
		vertices[i] = math_.Transform(vertices[i], viewProjectionMatrix);
		vertices[i] = math_.Transform(vertices[i], viewportMatrix);
	}

	int edges[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0},
		{4, 5}, {5, 6}, {6, 7}, {7, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	for (int i = 0; i < 12; ++i) {
		Novice::DrawLine(
			static_cast<int>(vertices[edges[i][0]].x),
			static_cast<int>(vertices[edges[i][0]].y),
			static_cast<int>(vertices[edges[i][1]].x),
			static_cast<int>(vertices[edges[i][1]].y),
			color
		);
	}
}

void DrawSphere(const Sphere &sphere, const Matrix4x4 &viewProjectionMatrix, const Matrix4x4 &viewportMatrix, uint32_t color) {
	const float pi = 3.1415926535f;
	const uint32_t kSubdivision = 10;  //分離数
	const float kLonEvery = 2.0f * pi / float(kSubdivision);//軽度１つ分の角度
	const float kLatEvery = pi / float(kSubdivision);//緯度分割1つ分の角度

	//緯度の方向に分割　-π/2~π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;  //現在の軽度

		//軽度の方向に分割
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;  //現在の軽度
			Vector3 a = {
				sphere.center.x + sphere.radius * cosf(lat) * cosf(lon),
				sphere.center.y + sphere.radius * sinf(lat),
				sphere.center.z + sphere.radius * cosf(lat) * sinf(lon)
			};

			Vector3 b = {
				sphere.center.x + sphere.radius * cosf(lat + kLatEvery) * cosf(lon),
				sphere.center.y + sphere.radius * sinf(lat + kLatEvery),
				sphere.center.z + sphere.radius * cosf(lat + kLatEvery) * sinf(lon)
			};

			Vector3 c = {
				sphere.center.x + sphere.radius * cosf(lat) * cos(lon + kLonEvery),
				sphere.center.y + sphere.radius * sinf(lat),
				sphere.center.z + sphere.radius * cosf(lat) * sinf(lon + kLonEvery)
			};

			a = math_.Transform(a, viewProjectionMatrix);
			a = math_.Transform(a, viewportMatrix);
			b = math_.Transform(b, viewProjectionMatrix);
			b = math_.Transform(b, viewportMatrix);
			c = math_.Transform(c, viewProjectionMatrix);
			c = math_.Transform(c, viewportMatrix);

			Novice::DrawLine(int(a.x), int(a.y), int(b.x), int(b.y), color);

			Novice::DrawLine(int(a.x), int(a.y), int(c.x), int(c.y), color);
		}
	}
}


bool IsCollision(const AABB &aabb1, const Sphere &sphere1) {
	// 球の中心点
		const Vector3 & center = sphere1.center;

	// 最近接点（AABB上で球の中心に最も近い点）を求める
	Vector3 closestPoint;
	closestPoint.x = std::clamp(center.x, aabb1.min.x, aabb1.max.x);
	closestPoint.y = std::clamp(center.y, aabb1.min.y, aabb1.max.y);
	closestPoint.z = std::clamp(center.z, aabb1.min.z, aabb1.max.z);

	// 最近接点と球の中心の距離を求める
	Vector3 difference = {
		center.x - closestPoint.x,
		center.y - closestPoint.y,
		center.z - closestPoint.z
	};

	float distanceSq = difference.x * difference.x +
		difference.y * difference.y +
		difference.z * difference.z;

	// 距離の二乗が半径の二乗より小さければ衝突
	return distanceSq <= (sphere1.radius * sphere1.radius);
}




// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };


	Vector3 cameraTranslate{ 0.0f,2.0f,-7.49f };
	Vector3 cameraRotate{ 0.2f,0.0f,0.0f };

	AABB aabb1{
		.min{-0.5f, 0.01f, -0.5f},
		.max{0.0f, 0.51f, 0.0f},
		.color{0xFFFFFFFF},
	};

	
	Sphere sphere1{ {0.0f, 0.0f, 0.5f}, 1.0f,{1.0f, 1.0f, 1.0f} };

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		aabb1.min.x = (std::min)(aabb1.min.x,aabb1.max.x);
		aabb1.max.x = (std::max)(aabb1.min.x, aabb1.max.x);
		aabb1.min.y = (std::min)(aabb1.min.y, aabb1.max.y);
		aabb1.max.y = (std::max)(aabb1.min.y, aabb1.max.y);
		aabb1.min.z = (std::min)(aabb1.min.z, aabb1.max.z);
		aabb1.max.z = (std::max)(aabb1.min.z, aabb1.max.z);


		if (IsCollision(aabb1, sphere1)) {
			aabb1.color = 0xFF0000FF;  //衝突したら色を赤にする
		} else {
			aabb1.color = 0xFFFFFFFF;
		}

		Matrix4x4 cameraMatrix = math_.MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = math_.Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = math_.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = math_.Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = math_.MakeViewportMatrix(1280, 720);


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("aabb1.min", &aabb1.min.x, 0.01f);
		ImGui::DragFloat3("aabb1.max", &aabb1.max.x, 0.01f);
		ImGui::DragFloat3("SphereCenter1", &sphere1.center.x, 0.01f);
		ImGui::DragFloat("SphereRadius1", &sphere1.radius, 0.01f);
		ImGui::End();

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		DrawAABB(aabb1, viewProjectionMatrix, viewportMatrix, aabb1.color);
		DrawSphere(sphere1, viewProjectionMatrix, viewportMatrix, ConvertColor(sphere1.color));
		

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
