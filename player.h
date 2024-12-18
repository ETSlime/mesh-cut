//=============================================================================
//
// ���f������ [player.h]
// Author : 
//
//=============================================================================
#pragma once
#include "SimpleArray.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_PLAYER		(1)					// �v���C���[�̐�

#define	PLAYER_SIZE		(5.0f)				// �����蔻��̑傫��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct PLAYER
{
	XMFLOAT3		pos;		// �|���S���̈ʒu
	XMFLOAT3		rot;		// �|���S���̌���(��])
	XMFLOAT3		scl;		// �|���S���̑傫��(�X�P�[��)

	XMFLOAT4X4		mtxWorld;	// ���[���h�}�g���b�N�X

	BOOL			load;
	DX11_MODEL		model;		// ���f�����

	float			spd;		// �ړ��X�s�[�h
	float			dir;		// ����
	float			size;		// �����蔻��̑傫��
	int				shadowIdx;	// �e��Index
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
// �v���g�^�C�v�錾
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