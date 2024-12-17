//=============================================================================
//
// ���f������ [enemy.cpp]
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
// �}�N����`
//*****************************************************************************
#define	MODEL_ENEMY		"data/MODEL/scaleCursor.obj"			// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(2.0f)							// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// ��]��

#define ENEMY_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define ENEMY_OFFSET_Y		(7.0f)							// �v���C���[�̑��������킹��
#define ENEMY_OFFSET_X		(17.0f)							

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ENEMY				g_Enemy[MAX_ENEMY];						// �v���C���[

//=============================================================================
// ����������
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

		g_Enemy[i].spd = 0.0f;			// �ړ��X�s�[�h�N���A
		g_Enemy[i].size = ENEMY_SIZE;	// �����蔻��̑傫��

		g_Enemy[i].use = TRUE;
	}


	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		// ���f���̉������
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
//	CAMERA *cam = GetCamera();
//
//	// �ړ��������Ⴄ
//	if (GetKeyboardPress(DIK_LEFT))
//	{	// ���ֈړ�
//		g_Enemy.spd = VALUE_MOVE;
//		g_Enemy.dir = XM_PI / 2;
//	}
//	if (GetKeyboardPress(DIK_RIGHT))
//	{	// �E�ֈړ�
//		g_Enemy.spd = VALUE_MOVE;
//		g_Enemy.dir = -XM_PI / 2;
//	}
//	if (GetKeyboardPress(DIK_UP))
//	{	// ��ֈړ�
//		g_Enemy.spd = VALUE_MOVE;
//		g_Enemy.dir = XM_PI;
//	}
//	if (GetKeyboardPress(DIK_DOWN))
//	{	// ���ֈړ�
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
//	//	// Key���͂���������ړ���������
//	if (g_Enemy.spd > 0.0f)
//	{
//		//g_Enemy.rot.y = g_Enemy.dir;
//		g_Enemy.rot.y = g_Enemy.dir + cam->rot.y;
//
//		// ���͂̂����������փv���C���[���������Ĉړ�������
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
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// ���f���`��
		MATERIAL material;
		ZeroMemory(&material, sizeof(material));
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material.noTexSampling = TRUE;
		SetMaterial(material);
		//DrawModel(&g_Enemy[i].model);
	}


	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
ENEMY *GetEnemy(void)
{
	return g_Enemy;
}

