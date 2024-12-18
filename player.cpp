//=============================================================================
//
// モデル処理 [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "player.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER		"data/MODEL/cube.obj"			// 読み込むモデル名

#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(7.0f)							// プレイヤーの足元をあわせる


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER				g_Player;						// プレイヤー
static BOOL isDragging = FALSE;
static float startX = 0, startY = 0;
static float currentX = 0, currentY = 0;
static float endX = 0, endY = 0;
static ID3D11Buffer* g_CuttingPlaneVertexBuffer = NULL;	// 頂点バッファ
static Plane cuttingPlane;
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	MakeCuttingPlaneVertex();
	//g_Player.model.modelData = new MODEL;
	LoadModel(MODEL_PLAYER, &g_Player.model);
	g_Player.load = TRUE;

	g_Player.pos = { 0.0f, PLAYER_OFFSET_Y, 0.0f };
	g_Player.rot = { 0.0f, 0.0f, 0.0f };
	g_Player.scl = { 1.0f, 1.0f, 1.0f };

	g_Player.spd  = 0.0f;			// 移動スピードクリア
	g_Player.size = PLAYER_SIZE;	// 当たり判定の大きさ




	g_Player.use = TRUE;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{
	// モデルの解放処理
	if (g_Player.load)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	CAMERA *cam = GetCamera();

	// 移動させちゃう
	if (GetKeyboardPress(DIK_LEFT))
	{	// 左へ移動
		g_Player.spd = VALUE_MOVE;
		g_Player.dir = XM_PI / 2;
	}
	if (GetKeyboardPress(DIK_RIGHT))
	{	// 右へ移動
		g_Player.spd = VALUE_MOVE;
		g_Player.dir = -XM_PI / 2;
	}
	if (GetKeyboardPress(DIK_UP))
	{	// 上へ移動
		g_Player.spd = VALUE_MOVE;
		g_Player.dir = XM_PI;
	}
	if (GetKeyboardPress(DIK_DOWN))
	{	// 下へ移動
		g_Player.spd = VALUE_MOVE;
		g_Player.dir = 0.0f;
	}


#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Player.pos.z = g_Player.pos.x = 0.0f;
		g_Player.rot.y = g_Player.dir = 0.0f;
		g_Player.spd = 0.0f;
	}
#endif


	//	// Key入力があったら移動処理する
	if (g_Player.spd > 0.0f)
	{
		//g_Player.rot.y = g_Player.dir;
		g_Player.rot.y = g_Player.dir + cam->rot.y;

		// 入力のあった方向へプレイヤーを向かせて移動させる
		g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;

		//cam->at = g_Player.pos;
	}


	g_Player.spd *= 0.8f;


#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Player:↑ → ↓ ←　Space\n");
	PrintDebugProc("Player:X:%f Y:%f Z:%f\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
#endif

	UpdateMeshCut();
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);


	// モデル描画
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	material.noTexSampling = TRUE;
	SetMaterial(material);
	DrawModel(&g_Player.model);


	//DrawCuttingPlane();

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

void CutMeshes(DX11_MODEL* model)
{
	SetLocalCuttingPlane();

	unsigned short newStartIdx1 = 0, newStartIdx2 = 0;
	BOOL updateModel = FALSE;
	for (unsigned short i = 0; i < model->modelData->SubsetNum; i++)
	{
		unsigned short startIdx = model->modelData->SubsetArray[i].StartIndex;

		unsigned short newIdxNum1 = 0, newIdxNum2 = 0;
		SimpleArray<VERTEX_3D>* newVertexArray1 = new SimpleArray<VERTEX_3D>;
		SimpleArray<VERTEX_3D>* newVertexArray2 = new SimpleArray<VERTEX_3D>;
		SimpleArray<VERTEX_3D>* oldVertexArray = new SimpleArray<VERTEX_3D>;
		SimpleArray<VERTEX_3D>* cuttingPointArray = new SimpleArray<VERTEX_3D>;
		BOOL potentialCut = FALSE, subsetCut = FALSE;
		for (unsigned short j = startIdx; j < startIdx + model->modelData->SubsetArray[i].IndexNum; j += 3)
		{
			VERTEX_3D v1 = model->modelData->VertexArray[j];
			VERTEX_3D v2 = model->modelData->VertexArray[j + 1];
			VERTEX_3D v3 = model->modelData->VertexArray[j + 2];

			if (potentialCut == TRUE)
			{
				oldVertexArray->push_back(v1);
				oldVertexArray->push_back(v2);
				oldVertexArray->push_back(v3);
				continue;
			}

			float d1 = XMVectorGetX(XMVector3Dot(
				XMLoadFloat3(&XMFLOAT3(
					v1.Position.x - cuttingPlane.localPoint.x,
					v1.Position.y - cuttingPlane.localPoint.y,
					v1.Position.z - cuttingPlane.localPoint.z)),
				XMLoadFloat3(&cuttingPlane.localNormal)));

			float d2 = XMVectorGetX(XMVector3Dot(
				XMLoadFloat3(&XMFLOAT3(
					v2.Position.x - cuttingPlane.localPoint.x,
					v2.Position.y - cuttingPlane.localPoint.y,
					v2.Position.z - cuttingPlane.localPoint.z)),
				XMLoadFloat3(&cuttingPlane.localNormal)));

			float d3 = XMVectorGetX(XMVector3Dot(
				XMLoadFloat3(&XMFLOAT3(
					v3.Position.x - cuttingPlane.localPoint.x,
					v3.Position.y - cuttingPlane.localPoint.y,
					v3.Position.z - cuttingPlane.localPoint.z)),
				XMLoadFloat3(&cuttingPlane.localNormal)));

			VERTEX_3D intersectPoints[2];
			int intersectCount = 0;

			ProcessEdge(v1, v2, d1, d2, intersectPoints, &intersectCount, potentialCut);
			ProcessEdge(v1, v3, d1, d3, intersectPoints, &intersectCount, potentialCut);
			ProcessEdge(v2, v3, d2, d3, intersectPoints, &intersectCount, potentialCut);

			if (potentialCut == TRUE)
			{
				oldVertexArray->push_back(v1);
				oldVertexArray->push_back(v2);
				oldVertexArray->push_back(v3);

				if (newVertexArray1->getSize() != 0)
					newVertexArray1->clear();
				if (newVertexArray2->getSize() != 0)
					newVertexArray2->clear();
				subsetCut = FALSE;
				continue;
			}

			if (intersectCount == 2)
			{
				cuttingPointArray->push_back(intersectPoints[0]);
				cuttingPointArray->push_back(intersectPoints[1]);
				subsetCut = TRUE;
				if ((d1 > 0 && d2 < 0 && d3 < 0) || (d1 < 0 && d2 > 0 && d3 > 0))
				{
					// Vertex 1 is on one side, vertices 2 and 3 are on the other side
					// Vertex 1 is alone
					VERTEX_3D newPoint1, newPoint2;
					int dir = 1;
					if (d1 < 0)
					{
						dir *= -1;
					}
						
					newPoint1 = intersectPoints[0];
					newPoint2 = intersectPoints[1];
					XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));

					if (d1 < 0)
					{
						newVertexArray1->push_back(v1);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(newPoint2);
					}
					else
					{
						newVertexArray2->push_back(v1);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(newPoint2);
					}


					newPoint1 = intersectPoints[0];
					newPoint2 = intersectPoints[1];
					XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));

					if (d2 < 0) // d3 < 0
					{
						newVertexArray1->push_back(v2);
						newVertexArray1->push_back(v3);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(v3);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(newPoint2);
					}
					else
					{
						newVertexArray2->push_back(v2);
						newVertexArray2->push_back(v3);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(v3);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(newPoint2);
					}

				}
				else if ((d2 > 0 && d1 < 0 && d3 < 0) || (d2 < 0 && d1 > 0 && d3 > 0))
				{
					VERTEX_3D newPoint1, newPoint2;
					int dir = 1;
					if (d2 < 0)
						dir *= -1;
					newPoint1 = intersectPoints[0];
					newPoint2 = intersectPoints[1];
					XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));

					// Vertex 2 is alone
					if (d2 < 0)
					{
						newVertexArray1->push_back(v2);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(newPoint2);
					}
					else
					{
						newVertexArray2->push_back(v2);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(newPoint2);
					}

					newPoint1 = intersectPoints[0];
					newPoint2 = intersectPoints[1];
					XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					
					if (d1 < 0) // d3 < 0
					{
						newVertexArray1->push_back(v1);
						newVertexArray1->push_back(v3);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(v3);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(newPoint2);
					}
					else
					{
						newVertexArray2->push_back(v1);
						newVertexArray2->push_back(v3);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(v3);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(newPoint2);
					}
				}
				else if ((d3 > 0 && d1 < 0 && d2 < 0) || (d3 < 0 && d1 > 0 && d2 > 0))
				{
					VERTEX_3D newPoint1, newPoint2;
					int dir = 1;
					if (d3 < 0)
						dir *= -1;
					newPoint1 = intersectPoints[0];
					newPoint2 = intersectPoints[1];
					XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					// Vertex 3 is alone
					if (d3 < 0)
					{
						newVertexArray1->push_back(v3);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(newPoint2);
					}
					else
					{
						newVertexArray2->push_back(v3);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(newPoint2);
					}

					newPoint1 = intersectPoints[0];
					newPoint2 = intersectPoints[1];
					XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
					
					if (d1 < 0) // d2 < 0
					{
						newVertexArray1->push_back(v1);
						newVertexArray1->push_back(v2);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(v2);
						newVertexArray1->push_back(newPoint1);
						newVertexArray1->push_back(newPoint2);
					}
					else
					{
						newVertexArray2->push_back(v1);
						newVertexArray2->push_back(v2);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(v2);
						newVertexArray2->push_back(newPoint1);
						newVertexArray2->push_back(newPoint2);
					}
				}
			}
			else
			{
				int dir = 1;
				if (d1 < 0)
				{
					dir *= -1;
				}
					
				XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
				XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
				XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));

				if (d1 < 0) // d2 < 0, d3 < 0
				{
					newVertexArray1->push_back(v1);
					newVertexArray1->push_back(v2);
					newVertexArray1->push_back(v3);
				}
				else
				{
					newVertexArray2->push_back(v1);
					newVertexArray2->push_back(v2);
					newVertexArray2->push_back(v3);
				}
			}
		}

		SimpleArray<VERTEX_3D>* newCuttingPlane = CheckAndCreateNewPlane(cuttingPointArray);
		if (newCuttingPlane != nullptr)
		{
			int vertexCnt = newCuttingPlane->getSize();
			for (int i = 0; i < vertexCnt; i++)
			{
				VERTEX_3D point = (*newCuttingPlane)[i];

				XMStoreFloat3(&point.Position, XMLoadFloat3(&point.Position) - 1.5f * XMLoadFloat3(&cuttingPlane.localNormal));
				newVertexArray1->push_back(point);

				point = (*newCuttingPlane)[i];
				XMStoreFloat3(&point.Position, XMLoadFloat3(&point.Position) + 1.5f * XMLoadFloat3(&cuttingPlane.localNormal));
				newVertexArray2->push_back(point);
			}
		}

		cuttingPointArray->clear();
		delete cuttingPointArray;

		if (subsetCut == TRUE)
		{
			updateModel = TRUE;
			oldVertexArray->clear();
			delete oldVertexArray;
		}
		else if (oldVertexArray->getSize() == 0)
			delete oldVertexArray;
		else
			model->oldVertexArrays.push_back(oldVertexArray);


		if (newVertexArray1->getSize() == 0)
			delete newVertexArray1;
		else
			model->newVertexArrays.push_back(newVertexArray1);

		if (newVertexArray2->getSize() == 0)
			delete newVertexArray2;
		else
			model->newVertexArrays.push_back(newVertexArray2);

	}


	//for (unsigned short i = 0; i < model->modelData->VertexNum; i += 3)
	//{
	//	VERTEX_3D v1 = model->modelData->VertexArray[i];
	//	VERTEX_3D v2 = model->modelData->VertexArray[i+1];
	//	VERTEX_3D v3 = model->modelData->VertexArray[i+2];

	//	float d1 = XMVectorGetX(XMVector3Dot(
	//		XMLoadFloat3(&XMFLOAT3(
	//			v1.Position.x - cuttingPlane.localPoint.x,
	//			v1.Position.y - cuttingPlane.localPoint.y,
	//			v1.Position.z - cuttingPlane.localPoint.z)),
	//		XMLoadFloat3(&cuttingPlane.localNormal)));

	//	float d2 = XMVectorGetX(XMVector3Dot(
	//		XMLoadFloat3(&XMFLOAT3(
	//			v2.Position.x - cuttingPlane.localPoint.x,
	//			v2.Position.y - cuttingPlane.localPoint.y,
	//			v2.Position.z - cuttingPlane.localPoint.z)),
	//		XMLoadFloat3(&cuttingPlane.localNormal)));

	//	float d3 = XMVectorGetX(XMVector3Dot(
	//		XMLoadFloat3(&XMFLOAT3(
	//			v3.Position.x - cuttingPlane.localPoint.x,
	//			v3.Position.y - cuttingPlane.localPoint.y,
	//			v3.Position.z - cuttingPlane.localPoint.z)),
	//		XMLoadFloat3(&cuttingPlane.localNormal)));

	//	VERTEX_3D intersectPoints[2];
	//	int intersectCount = 0;

	//	ProcessEdge(v1, v2, d1, d2, intersectPoints, &intersectCount);
	//	ProcessEdge(v1, v3, d1, d3, intersectPoints, &intersectCount);
	//	ProcessEdge(v2, v3, d2, d3, intersectPoints, &intersectCount);

	//	if (intersectCount == 2)
	//	{
	//		if ((d1 > 0 && d2 < 0 && d3 < 0) || (d1 < 0 && d2 > 0 && d3 > 0))
	//		{
	//			// Vertex 1 is on one side, vertices 2 and 3 are on the other side
	//			// Vertex 1 is alone
	//			VERTEX_3D newPoint1, newPoint2;
	//			int dir = 1;
	//			if (d1 < 0)
	//				dir *= -1;
	//			newPoint1 = intersectPoints[0];
	//			newPoint2 = intersectPoints[1];
	//			XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			model->newVertex.push_back(v1);
	//			model->newVertex.push_back(newPoint1);
	//			model->newVertex.push_back(newPoint2);

	//			unsigned short index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);


	//			newPoint1 = intersectPoints[0];
	//			newPoint2 = intersectPoints[1];
	//			XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));

	//			model->newVertex.push_back(v2);
	//			model->newVertex.push_back(v3);
	//			model->newVertex.push_back(newPoint1);

	//			index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);

	//			model->newVertex.push_back(v3);
	//			model->newVertex.push_back(newPoint1);
	//			model->newVertex.push_back(newPoint2);

	//			index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);
	//		}
	//		else if ((d2 > 0 && d1 < 0 && d3 < 0) || (d2 < 0 && d1 > 0 && d3 > 0))
	//		{
	//			VERTEX_3D newPoint1, newPoint2;
	//			int dir = 1;
	//			if (d2 < 0)
	//				dir *= -1;
	//			newPoint1 = intersectPoints[0];
	//			newPoint2 = intersectPoints[1];
	//			XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));

	//			// Vertex 2 is alone
	//			model->newVertex.push_back(v2);
	//			model->newVertex.push_back(newPoint1);
	//			model->newVertex.push_back(newPoint2);

	//			unsigned short index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);

	//			newPoint1 = intersectPoints[0];
	//			newPoint2 = intersectPoints[1];
	//			XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			model->newVertex.push_back(v1);
	//			model->newVertex.push_back(v3);
	//			model->newVertex.push_back(newPoint1);

	//			index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);

	//			model->newVertex.push_back(v3);
	//			model->newVertex.push_back(newPoint1);
	//			model->newVertex.push_back(newPoint2);

	//			index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);
	//		}
	//		else if ((d3 > 0 && d1 < 0 && d2 < 0) || (d3 < 0 && d1 > 0 && d2 > 0))
	//		{
	//			VERTEX_3D newPoint1, newPoint2;
	//			int dir = 1;
	//			if (d3 < 0)
	//				dir *= -1;
	//			newPoint1 = intersectPoints[0];
	//			newPoint2 = intersectPoints[1];
	//			XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			// Vertex 3 is alone
	//			model->newVertex.push_back(v3);
	//			model->newVertex.push_back(newPoint1);
	//			model->newVertex.push_back(newPoint2);

	//			unsigned short index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);

	//			newPoint1 = intersectPoints[0];
	//			newPoint2 = intersectPoints[1];
	//			XMStoreFloat3(&newPoint1.Position, XMLoadFloat3(&intersectPoints[0].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&newPoint2.Position, XMLoadFloat3(&intersectPoints[1].Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) - 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//			model->newVertex.push_back(v1);
	//			model->newVertex.push_back(v2);
	//			model->newVertex.push_back(newPoint1);

	//			index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);

	//			model->newVertex.push_back(v2);
	//			model->newVertex.push_back(newPoint1);
	//			model->newVertex.push_back(newPoint2);

	//			index = model->newIndex.getSize();
	//			model->newIndex.push_back(index);
	//			model->newIndex.push_back(index + 1);
	//			model->newIndex.push_back(index + 2);
	//		}
	//	}
	//	else
	//	{
	//		int dir = 1;
	//		if (d1 < 0)
	//			dir *= -1;
	//		XMStoreFloat3(&v1.Position, XMLoadFloat3(&v1.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//		XMStoreFloat3(&v2.Position, XMLoadFloat3(&v2.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//		XMStoreFloat3(&v3.Position, XMLoadFloat3(&v3.Position) + 1.5f * dir * XMLoadFloat3(&cuttingPlane.localNormal));
	//		model->newVertex.push_back(v1);
	//		model->newVertex.push_back(v2);
	//		model->newVertex.push_back(v3);

	//		unsigned short index = model->newIndex.getSize();
	//		model->newIndex.push_back(index);
	//		model->newIndex.push_back(index + 1);
	//		model->newIndex.push_back(index + 2);
	//	}
	//}

	if (updateModel == TRUE)
	{
		UpdateModel(model);
	}
	else
	{
		int oldVertexCnt = model->oldVertexArrays.getSize();
		for (int i = 0; i < oldVertexCnt; i++)
			delete model->oldVertexArrays[i];
	}
	model->newVertexArrays.clear();
	model->oldVertexArrays.clear();
}

SimpleArray<VERTEX_3D>* CheckAndCreateNewPlane(SimpleArray<VERTEX_3D>* cuttingPoints)
{
	if (cuttingPoints->getSize() <= 2) return nullptr;
	
	XMVECTOR p0 = XMLoadFloat3(&(*cuttingPoints)[0].Position);
	XMVECTOR direction = XMVectorSubtract(XMLoadFloat3(&(*cuttingPoints)[1].Position), p0);
	XMVECTOR vec;

	for (int i = 2; i < cuttingPoints->getSize(); i++)
	{
		vec = XMVectorSubtract(XMLoadFloat3(&(*cuttingPoints)[i].Position), p0);
		XMVECTOR crossProduct = XMVector3Cross(direction, vec);
		if (!XMVector3Equal(crossProduct, XMVectorZero())) 
		{
			SimpleArray<VERTEX_3D>* newPlane = new SimpleArray<VERTEX_3D>;
			CreateNewPlane(newPlane, cuttingPoints);
			return newPlane;
		}
	}
	return nullptr;
}

void CreateNewPlane(SimpleArray<VERTEX_3D>* newPlane, SimpleArray<VERTEX_3D>* cuttingPoints)
{
	VERTEX_3D center = (*cuttingPoints)[0];
	SimpleArray<VertexWithAngle> verticesWithAngles;
	int cuttingPointsNum = cuttingPoints->getSize();
	for (int i = 1; i < cuttingPointsNum; i++)
	{
		XMFLOAT3 refVec;
		XMVECTOR U, V;

		// 法線ベクトルから垂直ベクトルを計算
		if (cuttingPlane.localNormal.x == 0 && cuttingPlane.localNormal.y == 0)
		{
			refVec = XMFLOAT3(1, 0, 0);
		}
		else
		{
			refVec = XMFLOAT3(-cuttingPlane.localNormal.y, cuttingPlane.localNormal.x, 0);
		}

		// ローカル座標系を構築
		U = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&cuttingPlane.localNormal), XMLoadFloat3(&refVec))); // U = N x R
		V = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&cuttingPlane.localNormal), U));						// V = N x U

		VERTEX_3D point = (*cuttingPoints)[i];

		// 頂点の投影を計算
		float pu = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&point.Position), U));
		float pv = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&point.Position), V));
		// 角度を計算
		float angle = atan2(pv, pu);

		verticesWithAngles.push_back({point, angle});
	}

	InsertionSort(verticesWithAngles);

	for (int i = 1; i < cuttingPointsNum - 1; i++)
	{
		newPlane->push_back(center);
		newPlane->push_back((*cuttingPoints)[i]);
		newPlane->push_back((*cuttingPoints)[i + 1]);
	}
}

void InsertionSort(SimpleArray<VertexWithAngle>& vertices)
{
	for (int i = 1; i < vertices.getSize(); i++) 
	{
		VertexWithAngle key = vertices[i];
		int j = i - 1;

		while (j >= 0 && vertices[j].angle > key.angle) 
		{
			vertices[j + 1] = vertices[j];
			j--;
		}
		vertices[j + 1] = key;
	}
}

void SetLocalCuttingPlane(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);

	XMMATRIX worldMatrix = XMLoadFloat4x4(&g_Player.mtxWorld);
	XMMATRIX invWorldMatrix = XMMatrixInverse(nullptr, worldMatrix);

	XMVECTOR planeNormal = XMVectorSet(cuttingPlane.normal.x, cuttingPlane.normal.y, cuttingPlane.normal.z, 0);
	XMVECTOR planePoint = XMVectorSet(cuttingPlane.point.x, cuttingPlane.point.y, cuttingPlane.point.z, 1);

	XMVECTOR localPlaneNormal = XMVector3Normalize(XMVector3TransformNormal(planeNormal, invWorldMatrix));
	XMVECTOR localPlanePoint = XMVector3TransformCoord(planePoint, invWorldMatrix);

	//if (cuttingPlane.localNormal.y < 0)
	//	XMStoreFloat3(&cuttingPlane.localNormal, XMLoadFloat3(&cuttingPlane.localNormal) * -1);

	XMStoreFloat3(&cuttingPlane.localNormal, localPlaneNormal);
	XMStoreFloat3(&cuttingPlane.localPoint, localPlanePoint);
	XMStoreFloat4x4(&cuttingPlane.localmtxWorld, worldMatrix);
}

void ProcessEdge(VERTEX_3D v1, VERTEX_3D v2, float d1, float d2, VERTEX_3D *intersectionPoint, int *count, BOOL& potentialCut)
{
	PrintDebugProc("d1: %f d2: %f\n", d1, d2);

	if ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) 
	{
		PrintDebugProc("Different Point1X: %f, Point1Y: %f, Point1Z: %f\n", v1.Position.x, v1.Position.y, v1.Position.z);
		PrintDebugProc("Different Point2X: %f, Point2Y: %f, Point2Z: %f\n", v2.Position.x, v2.Position.y, v2.Position.z);

		float t = -d1 / (d2 - d1);
		XMFLOAT3 intersection;
		XMStoreFloat3(&intersection, XMLoadFloat3(&v1.Position) + t * (XMLoadFloat3(&XMFLOAT3(v2.Position.x - v1.Position.x, v2.Position.y - v1.Position.y, v2.Position.z - v1.Position.z))));

		XMVECTOR localPoint = XMVectorSet(intersection.x, intersection.y, intersection.z, 1.0);
		XMVECTOR worldPoint = XMVector3TransformCoord(localPoint, XMLoadFloat4x4(&cuttingPlane.localmtxWorld));

		CAMERA* camera = GetCamera();
		XMMATRIX viewMatrix = XMLoadFloat4x4(&camera->mtxView);
		XMVECTOR viewPoint = XMVector3TransformCoord(worldPoint, viewMatrix);

		XMMATRIX projectionMatrix = XMLoadFloat4x4(&camera->mtxProjection);
		XMVECTOR clipPoint = XMVector3TransformCoord(viewPoint, projectionMatrix);

		clipPoint /= XMVectorGetW(clipPoint);

		float screenX = (XMVectorGetX(clipPoint) + 1.0f) * 0.5f * SCREEN_WIDTH;
		float screenY = (1.0f - XMVectorGetY(clipPoint)) * 0.5f * SCREEN_HEIGHT;
		if (screenX <= max(startX, endX)
			&& screenX >= min(startX, endX)
			&& screenY <= max(startY, endY)
			&& screenY >= min(startY, endY))
		{
			PrintDebugProc("intersection X: %f, Y: %f, Z: %f\n", intersection.x, intersection.y, intersection.z);
			if (*count < 2)
			{
				intersectionPoint[*count] = v1;
				intersectionPoint[*count].Position = intersection;
				(*count)++;
			}
		}
		else
		{
			potentialCut = TRUE;
		}


	}
	else 
	{
		PrintDebugProc("Same Point1X: %f, Point1Y: %f, Point1Z: %f\n", v1.Position.x, v1.Position.y, v1.Position.z);
		PrintDebugProc("Same Point2X: %f, Point2Y: %f, Point2Z: %f\n", v2.Position.x, v2.Position.y, v2.Position.z);
	}
}

//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER *GetPlayer(void)
{
	return &g_Player;
}

void UpdateMeshCut(void)
{
	if (IsMouseLeftTriggered())
	{
		isDragging = TRUE;
		startX = GetMousePosX();
		startY = GetMousePosY();
		currentX = startX;
		currentY = startY;
		endX = 0.0f;
		endY = 0.0f;
	}
	else if (IsMouseLeftPressed() && isDragging == TRUE)
	{
		long newX = GetMousePosX();
		long newY = GetMousePosY();
		currentX = newX;
		currentY = newY;

	}
	else if (!IsMouseLeftPressed() && isDragging == TRUE)
	{
		isDragging = FALSE;
		endX = GetMousePosX();
		endY = GetMousePosY();

		UpdateCuttingPlane();
	}
	//UpdateCuttingPlane();
	PrintDebugProc("StartX: %f, StartY: %f\n", startX, startY);
	PrintDebugProc("CurrentX: %f, CurrentY: %f\n", currentX, currentY);
	PrintDebugProc("EndX: %f, EndY: %f\n", endX, endY);

	
}

void MakeCuttingPlaneVertex()
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_CuttingPlaneVertexBuffer);
}

XMVECTOR CreateRayFromScreenPoint(float sx, float sy, const XMMATRIX& invVP) {
	float vx = (2.0f * sx / SCREEN_WIDTH - 1.0f);
	float vy = (1.0f - 2.0f * sy / SCREEN_HEIGHT);
	XMVECTOR rayClip = XMVectorSet(vx, vy, 1.0f, 1.0f);
	XMVECTOR rayWorld = XMVector3TransformCoord(rayClip, invVP);
	return rayWorld;
}

void UpdateCuttingPlane()
{
	CAMERA* camera = GetCamera();
	XMMATRIX P = XMLoadFloat4x4(&camera->mtxProjection);

	// Compute picking ray in view space.
	float vx = (+2.0f * startX / SCREEN_WIDTH - 1.0f) / P.r[0].m128_f32[0];
	float vy = (-2.0f * startY / SCREEN_HEIGHT + 1.0f) / P.r[1].m128_f32[1];

	// Ray definition in view space.
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDirStart = XMVectorSet(vx, vy, 1.0f, 0.0f);

	vx = (+2.0f * endX / SCREEN_WIDTH - 1.0f) / P.r[0].m128_f32[0];
	vy = (-2.0f * endY / SCREEN_HEIGHT + 1.0f) / P.r[1].m128_f32[1];
	XMVECTOR rayDirEnd = XMVectorSet(vx, vy, 1.0f, 0.0f);

	// Tranform ray to local space of Mesh.
	XMMATRIX V = XMLoadFloat4x4(&camera->mtxView);
	XMMATRIX invView = XMLoadFloat4x4(&camera->mtxInvView);

	//XMMATRIX W = XMLoadFloat4x4(&enemy->mtxWorld);
	//XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	//XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, invView);
	rayDirStart = XMVector3TransformNormal(rayDirStart, invView);
	rayDirEnd = XMVector3TransformNormal(rayDirEnd, invView);

	rayDirStart = XMVector3Normalize(rayDirStart);
	rayDirEnd = XMVector3Normalize(rayDirEnd);

	XMFLOAT3 start, end, origin;
	XMStoreFloat3(&origin, rayOrigin);
	XMStoreFloat3(&start, rayDirStart);
	XMStoreFloat3(&end, rayDirEnd);
	//PrintDebugProc("rayOrigin X: %f Y: %f Z: %f\n", origin.x, origin.y, origin.z);
	//PrintDebugProc("rayStart X: %f Y: %f Z: %f\n", start.x, start.y, start.z);
	//PrintDebugProc("rayEnd X: %f Y: %f Z: %f\n", end.x, end.y, end.z);

	XMMATRIX invVP = XMMatrixInverse(nullptr, XMMatrixMultiply(V, P));
	XMVECTOR ray1Origin = CreateRayFromScreenPoint(startX, startY, invVP);
	XMVECTOR ray2Origin = CreateRayFromScreenPoint(endX, endY, invVP);

	XMVECTOR commonDirection = XMVector3Normalize(XMVectorSubtract(ray1Origin, XMLoadFloat3(&camera->pos)));

	float distance = 15.0f;
	XMFLOAT3 point1, point2, point3, point4;
	//XMStoreFloat3(&point1, ray1Origin);
	//XMStoreFloat3(&point2, ray2Origin);
	//XMVECTOR pointAlongRay = ray1Origin + distance * commonDirection;
	//XMStoreFloat3(&point3, pointAlongRay);
	//pointAlongRay = ray2Origin + distance * commonDirection;
	//XMStoreFloat3(&point4, pointAlongRay);

	XMVECTOR pointAlongRay = rayOrigin + distance * rayDirStart;
	XMStoreFloat3(&point1, pointAlongRay);

	pointAlongRay = rayOrigin + distance * 20 * rayDirStart;
	XMStoreFloat3(&point2, pointAlongRay);

	pointAlongRay = rayOrigin + distance * rayDirEnd;
	XMStoreFloat3(&point3, pointAlongRay);

	pointAlongRay = rayOrigin + distance * 20 * rayDirEnd;
	XMStoreFloat3(&point4, pointAlongRay);

	//// Calculate screen space vector
	//float dx = endX - startX;
	//float dy = endY - startY;

	//// Calculate normal vector
	//float nx = -dy;
	//float ny = dx;

	//// Normalize the normal vector
	//float length = sqrt(nx * nx + ny * ny);
	//nx /= length;
	//ny /= length;

	//// Convert to XMVECTOR
	//XMVECTOR normalVecScreen = XMVectorSet(nx, ny, 0.0f, 0.0f);

	//// Transform to world space using the inverse view matrix
	//XMVECTOR normalVecWorld = XMVector3TransformNormal(normalVecScreen, invView);

	//XMStoreFloat3(&cuttingPlane.normal, normalVecWorld);

	//cuttingPlane.point = point1;
	XMStoreFloat3(&cuttingPlane.point, rayOrigin);
	XMVECTOR pointAcrossRay = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(point3.x - point1.x, point3.y - point1.y, point3.z - point1.z)));
	XMFLOAT3 alongRay, acrossRay;
	XMStoreFloat3(&alongRay, pointAlongRay - rayOrigin);
	XMStoreFloat3(&acrossRay, pointAcrossRay);

	XMStoreFloat3(&cuttingPlane.normal, XMVector3Normalize(XMVector3Cross(pointAlongRay - rayOrigin, pointAcrossRay)));



	CutMeshes(&g_Player.model);

	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_CuttingPlaneVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	// 頂点座標の設定
	vertex[0].Position = point1;
	vertex[1].Position = point2;
	vertex[2].Position = point3;
	vertex[3].Position = point4;

	// 法線の設定
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_CuttingPlaneVertexBuffer, 0);
}

void DrawCuttingPlane()
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_CuttingPlaneVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.noTexSampling = TRUE;
	SetMaterial(material);

	// ワールドマトリックスの初期化
	XMMATRIX mtxWorld = XMMatrixIdentity();

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	GetDeviceContext()->Draw(4, 0);
}