//=============================================================================
//
// モデル処理 [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "SimpleArray.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define MAX_PLAYER		(1)					// プレイヤーの数

#define	PLAYER_SIZE		(5.0f)				// 当たり判定の大きさ


//*****************************************************************************
// 構造体定義
//*****************************************************************************
struct PLAYER
{
	XMFLOAT3		pos;		// ポリゴンの位置
	XMFLOAT3		rot;		// ポリゴンの向き(回転)
	XMFLOAT3		scl;		// ポリゴンの大きさ(スケール)

	XMFLOAT4X4		mtxWorld;	// ワールドマトリックス

	BOOL			load;
	DX11_MODEL		model;		// モデル情報

	float			spd;		// 移動スピード
	float			dir;		// 向き
	float			size;		// 当たり判定の大きさ
	int				shadowIdx;	// 影のIndex
	BOOL			use;


};

struct Plane
{
	XMFLOAT3	point;
	XMFLOAT3	normal;
	XMFLOAT3	localPoint;
	XMFLOAT3	localNormal;
	XMFLOAT4X4	localmtxWorld;
};

struct VertexWithAngle 
{
	VERTEX_3D vertex;
	float angle;
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitPlayer(void);
void UninitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);

PLAYER *GetPlayer(void);
void UpdateMeshCut(void);
void DrawCuttingPlane(void);
void MakeCuttingPlaneVertex(void);
void UpdateCuttingPlane(void);
void CutMeshes(DX11_MODEL* model);
void ProcessEdge(VERTEX_3D v1, VERTEX_3D v2, float d1, float d2, VERTEX_3D* intersectionPoint, int* count, BOOL& potentialCut);
void SetLocalCuttingPlane(void);
SimpleArray<VERTEX_3D>* CheckAndCreateNewPlane(SimpleArray<VERTEX_3D>* cuttingPoints);
void CreateNewPlane(SimpleArray<VERTEX_3D>* newPlane, SimpleArray<VERTEX_3D>* cuttingPoints);
void InsertionSort(SimpleArray<VertexWithAngle>& vertices);