
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



struct Triangle {
	Vector3 vertices[3];
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




bool IsCollision(const Triangle &triangle, const Segment &segment) {
	Vector3 edge1 = math_.Subtract(triangle.vertices[1], triangle.vertices[0]);
	Vector3 edge2 = math_.Subtract(triangle.vertices[2], triangle.vertices[0]);
	Vector3 normal = Normalize(cross(edge1, edge2));

	float d1 = math_.Dot(segment.origin, normal) - math_.Dot(triangle.vertices[0], normal);
	Vector3 end = math_.Add(segment.origin, segment.diff);
	float d2 = math_.Dot(end, normal) - math_.Dot(triangle.vertices[0], normal);

	if (d1 * d2 > 0.0f) return false;

	float t = d1 / (d1 - d2);
	Vector3 point = {
		segment.origin.x + segment.diff.x * t,
		segment.origin.y + segment.diff.y * t,
		segment.origin.z + segment.diff.z * t
	};

	Vector3 v0 = edge1;
	Vector3 v1 = edge2;
	Vector3 v2 = math_.Subtract(point, triangle.vertices[0]);

	float d00 = math_.Dot(v0, v0);
	float d01 = math_.Dot(v0, v1);
	float d11 = math_.Dot(v1, v1);
	float d20 = math_.Dot(v2, v0);
	float d21 = math_.Dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	if (denom == 0.0f) return false;

	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	const float epsilon = 1e-5f;
	return (u >= -epsilon && v >= -epsilon && w >= -epsilon);
}



void DrawTriangle(const Triangle &triangle, const Matrix4x4 &viewportMatrix, uint32_t color) {
	// 3点をビューポート変換
	Vector3 p0 = math_.Transform(triangle.vertices[0], viewportMatrix);
	Vector3 p1 = math_.Transform(triangle.vertices[1], viewportMatrix);
	Vector3 p2 = math_.Transform(triangle.vertices[2], viewportMatrix);

	Novice::DrawTriangle(
		static_cast<int>(p0.x), static_cast<int>(p0.y),
		static_cast<int>(p1.x), static_cast<int>(p1.y),
		static_cast<int>(p2.x), static_cast<int>(p2.y),
		color, kFillModeWireFrame
	);
}
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };


	Vector3 cameraTranslate{ 0.0f,0.5f,-7.49f };
	Vector3 cameraRotate{ -0.2f,0.0f,0.0f };
	Segment segment = { {-1.0f, 1.0f, -2.0f}, {2.0f, -2.0f, 4.0f}, 0xFFFFFFFF };
	Triangle triangle = {
	{
		{ -0.5f, 0.0f, 0.0f },  // v0
		{  0.5f, 0.0f, 0.0f },  // v1
		{  0.0f, -1.0f, 0.0f }   // v2
	}
	};

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

		

		if (IsCollision(triangle,segment)) {
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
		ImGui::DragFloat3("Traiangle.v0", &triangle.vertices[0].x, 0.01f);
		ImGui::DragFloat3("Traiangle.v1", &triangle.vertices[1].x, 0.01f);
		ImGui::DragFloat3("Traiangle.v2", &triangle.vertices[2].x, 0.01f);
		ImGui::DragFloat3("Segment.Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment.Diff", &segment.diff.x, 0.01f);
		ImGui::End();


		DrawGrid(viewProjectionMatrix, viewportMatrix);
		Matrix4x4 viewProjectionViewportMatrix = math_.Multiply(viewProjectionMatrix, viewportMatrix);
		DrawTriangle(triangle, viewProjectionViewportMatrix, 0xFFFFFF);
		Vector3 segmentEnd = math_.Add(segment.origin, segment.diff); // 終点 = 始点 + 差分

		Vector3 start = math_.Transform(math_.Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = math_.Transform(math_.Transform(segmentEnd, viewProjectionMatrix), viewportMatrix);
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
