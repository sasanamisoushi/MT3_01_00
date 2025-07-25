#include <Novice.h>
#include <numbers>
#include <imgui.h>
#include "math.h"
#include <algorithm>
#include "Mouse.h"




using namespace KamataEngine::MathUtility;

using std::numbers::pi_v;

const char kWindowTitle[] = "LE2C_07_sasnami_sousi";

Mouse mouse;




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




void DrawBezier(const Vector3 &controlPoint0, const Vector3 &controlPoint1, const Vector3 &controlPoint2, const Matrix4x4 &viewProjectionMatrix, const Matrix4x4 &viewportMatrix, uint32_t color) {
	const int kSegmentCount = 32;
	for (int i = 0; i < kSegmentCount; ++i) {
		float t0 = static_cast<float>(i) / kSegmentCount;
		float t1 = static_cast<float>(i + 1) / kSegmentCount;

		Vector3 a0 = math_.Leap(controlPoint0, controlPoint1, t0);
		Vector3 a1 = math_.Leap(controlPoint1, controlPoint2, t0);
		Vector3 point0 = math_.Leap(a0, a1, t0);

		Vector3 b0 = math_.Leap(controlPoint0, controlPoint1, t1);
		Vector3 b1 = math_.Leap(controlPoint1, controlPoint2, t1);
		Vector3 point1 = math_.Leap(b0, b1, t1);

		Vector3 ndc0 = math_.Transform(point0, viewProjectionMatrix);
		Vector3 ndc1 = math_.Transform(point1, viewProjectionMatrix);
		Vector3 screen0 = math_.Transform(ndc0, viewportMatrix);
		Vector3 screen1 = math_.Transform(ndc1, viewportMatrix);

		Novice::DrawLine(static_cast<int>(screen0.x), static_cast<int>(screen0.y),
			static_cast<int>(screen1.x), static_cast<int>(screen1.y),
			color);
	}

	Vector3 controlPoints[3] = { controlPoint0, controlPoint1, controlPoint2 };
	for (int i = 0; i < 3; ++i) {
		Vector3 ndc = math_.Transform(controlPoints[i], viewProjectionMatrix);
		Vector3 screen = math_.Transform(ndc, viewportMatrix);

		int x = static_cast<int>(screen.x);
		int y = static_cast<int>(screen.y);
		int radius = 5;

		// 円（塗りつぶし）で描画
		Novice::DrawEllipse(x, y, radius, radius, 0.0f, 0xFFFFFFFF, kFillModeSolid);
	}
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

	
	Vector3 controlPoints[3] = {
		{-0.8f,0.58f,1.0f},
		{1.76f,1.0f,-0.3f},
		{0.94f,-0.7f,2.3f},
	};

	

	// マウス入力取得
	int mouseX = 0, mouseY = 0;
	static int preMouseX = 0, preMouseY = 0;
	Novice::GetMousePosition(&mouseX, &mouseY);

	mouse.Initialize();

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

		

		mouse.Update(keys);

		cameraRotate = mouse.GetRotate();
		cameraTranslate = mouse.GetPosition();

		
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
		ImGui::DragFloat3("Camera Pos", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("Camera Rot", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("controlPoints[0]", &controlPoints[0].x, 0.01f);
		ImGui::DragFloat3("controlPoints[1]", &controlPoints[1].x, 0.01f);
		ImGui::DragFloat3("controlPoints[2]", &controlPoints[2].x, 0.01f);
		ImGui::End();

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		Vector3 bezierColor = { 1.0f, 0.0f, 0.0f }; // 赤色
		uint32_t color = ConvertColor(bezierColor);

		DrawBezier(controlPoints[0], controlPoints[1], controlPoints[2], viewProjectionMatrix, viewportMatrix, color);
	

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
