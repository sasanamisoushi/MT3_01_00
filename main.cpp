
#include <Novice.h>

#include <numbers>
#include <imgui.h>
#include "math.h"

using namespace KamataEngine::MathUtility;

using std::numbers::pi_v;

const char kWindowTitle[] = "LE2C_07_sasnami_sousi";

struct Segment {
	Vector3 origin;   //始点
	Vector3 diff;     //終点
	uint32_t color;
};

struct Plane {
	Vector3 normal;
	float distance;
};

Math math_;

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




bool IsCollision(const Segment &segment, const Plane &plane) {
	// 始点と終点の距離を法線方向に投影
	float d1 = math_.Dot(segment.origin, plane.normal) - plane.distance;
	float d2 = math_.Dot(segment.diff, plane.normal) - plane.distance;

	// 片方が正、片方が負 → 交差している
	return (d1 * d2 <= 0.0f);
}

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

void DrawPlane(const Plane &plane, const Matrix4x4 &viewProjectionMatrix, const Matrix4x4 &viewportMatrix, uint32_t color) {
	Vector3 center = math_.Multiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y, - perpendiculars[0].z };
	perpendiculars[2] = cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y,-perpendiculars[2].z };

	
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = math_.Multiply(2.0f, perpendiculars[index]);
		Vector3 point = math_.Add(center, extend);
		points[index] = math_.Transform(math_.Transform(point, viewProjectionMatrix), viewportMatrix);
	}

	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[2].x, (int)points[2].y, (int)points[1].x, (int)points[1].y, color);
	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);
	Novice::DrawLine((int)points[3].x, (int)points[3].y, (int)points[0].x, (int)points[0].y, color);
}

Plane plane = { {0.0f,1.0f,0.0f},0.0f };

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };


	Vector3 cameraTranslate{ 0.0f,0.5f,-7.49f };
	Vector3 cameraRotate{ -0.2f,0.0f,0.0f };
	Segment segment = { {-1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f},0xFFFFFFFF };

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

		plane.normal = Normalize(plane.normal);

		if (IsCollision(segment, plane)) {
			segment.color = 0xFF0000FF;  //衝突したら色を赤にする
		} else {
			segment.color = 0xFFFFFFFF;
		}

		Matrix4x4 viewMatrix = math_.MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, cameraRotate, cameraTranslate);
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
		ImGui::DragFloat3("plane.Normal", &plane.normal.x, 0.01f);
		ImGui::DragFloat("Plane Distance", &plane.distance, 0.01f);
		ImGui::DragFloat3("Segment.Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment.Diff", &segment.diff.x, 0.01f);
		ImGui::End();


		DrawGrid(viewProjectionMatrix, viewportMatrix);
		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0x00FF00FF);

		Vector3 start = math_.Transform(math_.Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = math_.Transform(math_.Transform(math_.Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), segment.color);

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
