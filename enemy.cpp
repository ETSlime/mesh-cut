//=============================================================================
//
// モデル処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "enemy.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY		"data/MODEL/scaleCursor.obj"			// 読み込むモデル名

#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// 回転量

#define ENEMY_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define ENEMY_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる
#define ENEMY_OFFSET_X		(17.0f)							

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY				g_Enemy[MAX_ENEMY];						// プレイヤー

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = {ENEMY_OFFSET_X * i, ENEMY_OFFSET_Y * i, 0.0f};
		g_Enemy[i].rot = {0.0f, 0.0f, 0.0f};
		g_Enemy[i].scl = {1.0f, 1.0f, 1.0f};

		g_Enemy[i].spd = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ

		g_Enemy[i].use = TRUE;
	}


	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		// モデルの解放処理
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
//	CAMERA *cam = GetCamera();
//
//	// 移動させちゃう
//	if (GetKeyboardPress(DIK_LEFT))
//	{	// 左へ移動
//		g_Enemy.spd = VALUE_MOVE;
//		g_Enemy.dir = XM_PI / 2;
//	}
//	if (GetKeyboardPress(DIK_RIGHT))
//	{	// 右へ移動
//		g_Enemy.spd = VALUE_MOVE;
//		g_Enemy.dir = -XM_PI / 2;
//	}
//	if (GetKeyboardPress(DIK_UP))
//	{	// 上へ移動
//		g_Enemy.spd = VALUE_MOVE;
//		g_Enemy.dir = XM_PI;
//	}
//	if (GetKeyboardPress(DIK_DOWN))
//	{	// 下へ移動
//		g_Enemy.spd = VALUE_MOVE;
//		g_Enemy.dir = 0.0f;
//	}
//
//
//#ifdef _DEBUG
//	if (GetKeyboardPress(DIK_R))
//	{
//		g_Enemy.pos.z = g_Enemy.pos.x = 0.0f;
//		g_Enemy.rot.y = g_Enemy.dir = 0.0f;
//		g_Enemy.spd = 0.0f;
//	}
//#endif
//
//
//	//	// Key入力があったら移動処理する
//	if (g_Enemy.spd > 0.0f)
//	{
//		//g_Enemy.rot.y = g_Enemy.dir;
//		g_Enemy.rot.y = g_Enemy.dir + cam->rot.y;
//
//		// 入力のあった方向へプレイヤーを向かせて移動させる
//		g_Enemy.pos.x -= sinf(g_Enemy.rot.y) * g_Enemy.spd;
//		g_Enemy.pos.z -= cosf(g_Enemy.rot.y) * g_Enemy.spd;
//
//		cam->at = g_Enemy.pos;
//	}
//
//
//	g_Enemy.spd *= 0.8f;
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// モデル描画
		MATERIAL material;
		ZeroMemory(&material, sizeof(material));
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material.noTexSampling = TRUE;
		SetMaterial(material);
		//DrawModel(&g_Enemy[i].model);
	}


	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
ENEMY *GetEnemy(void)
{
	return g_Enemy;
}

