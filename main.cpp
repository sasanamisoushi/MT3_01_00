#include <Novice.h>
#include <cstdint>
#include <numbers>
#include <imgui.h>
#include "Math.h"

using std::numbers::pi_v;

const char kWindowTitle[] = "LE2C_07_sasnami_sousi";

struct Sphere {
	Vector3 center; //中心点
	float radius;   //半径
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

			Novice::DrawLine(int(a.x),int(a.y),int(b.x),int(b.y),color);

			Novice::DrawLine(int(a.x),int(a.y),int(c.x),int(c.y),color);
		}
	}
}



// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};


	Vector3 cameraTranslate{ 0.0f,0.5f,-7.49f };
	Vector3 cameraRotate{ 0.0f,0.0f,0.0f };
	Sphere sphere{ {0.0f, 0.0f, 0.5f}, 1.0f };

	
	
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
		

		Matrix4x4 viewMatrix = math_.MakeAffineMatrix({ 1.0f, 1.0f, 1.0f },  cameraRotate, cameraTranslate);
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
		ImGui::DragFloat3("SphereCenter", &sphere.center.x, 0.01f);
		ImGui::DragFloat("SphereRadius", &sphere.radius, 0.01f);
		ImGui::End();


		DrawGrid(viewProjectionMatrix, viewportMatrix);
		DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);
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
