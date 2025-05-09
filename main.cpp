#include <Novice.h>
#include <cmath>

const char kWindowTitle[] = "LE2C_07_sasnami_sousi";

struct Matrix4x4 {
	float m[4][4];
};

//透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {};  

	float f = 1.0f / tanf(fovY / 2.0f);
	

	result.m[0][0] = f / aspectRatio;
	result.m[1][1] = f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip*farClip/(farClip - nearClip);
	result.m[3][3] = 0.0f; 

	return result;
}

//正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};

	float width = right - left;
	float height = top-bottom ;
	float depth = farClip - nearClip;

	result.m[0][0] = 2.0f / width;
	result.m[1][1] = 2.0f / height;
	result.m[2][2] = 1.0f / depth;
	result.m[3][0] = -(right + left) / width;
	result.m[3][1] = -(bottom + top) / height;
	result.m[3][2] = -nearClip / depth;
	result.m[3][3] = 1.0f;

	return result;
}


//ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left,float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 mat = {};

	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;
	float depthRange = maxDepth - minDepth;

	mat.m[0][0] = halfWidth;
	mat.m[1][1] = -halfHeight; // Y軸が上から下へ向かう場合
	mat.m[2][2] = depthRange;
	mat.m[3][0] = left + halfWidth;
	mat.m[3][1] = top + halfHeight;
	mat.m[3][2] = minDepth;
	mat.m[3][3] = 1.0f;

	return mat;
}

static const int kRowHeight = 20;
static const int kColumnWidth = 60;
void MatrixScreenPrintf(int x, int y, const Matrix4x4 &matrix, const char *label) {
	Novice::ScreenPrintf(x, y - kRowHeight, "%s", label); // ラベル表示（上に）
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(x + column * kColumnWidth, y + row * kRowHeight, "%6.02f", matrix.m[row][column]);
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

		Matrix4x4 orthographicMatrix = MakeOrthographicMatrix(-160.0f, 160.0f, 200.0f, 300.0f, 0.0f, 1000.0f);

		Matrix4x4 perspectiveFovMatrix = MakePerspectiveFovMatrix(0.63f, 1.33f, 0.1f, 1000.0f);

		Matrix4x4 viewportMatrix= MakeViewportMatrix(100.0f, 200.0f, 600.0f, 300.0f, 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 20, orthographicMatrix, "orthographicMatrix");
		MatrixScreenPrintf(0, kRowHeight*6, perspectiveFovMatrix, "perspectiveFovMatrix");
		MatrixScreenPrintf(0, kRowHeight * 11, viewportMatrix, "viewportMatrix");

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
