//=============================================================================
//
// ���f���̏��� [model.h]
// Author :
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "SimpleArray.h"
//*********************************************************
// �\����
//*********************************************************

// �}�e���A���\����

#define MODEL_MAX_MATERIAL		(16)		// �P���f����Max�}�e���A����


// �}�e���A���\����
struct MODEL_MATERIAL
{
	char						Name[256];
	MATERIAL					Material;
	char						TextureName[256];
	ID3D11ShaderResourceView* Texture;

	MODEL_MATERIAL()
	{
		Material.noTexSampling = 1;
		Material.LoadMaterial = FALSE;
		Material.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Material.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		Material.Shininess = 0.0f;
		Texture = NULL;
	}
};

// �`��T�u�Z�b�g�\����
struct SUBSET
{
	unsigned short	StartIndex;
	unsigned short	IndexNum;
	MODEL_MATERIAL	Material;

	SUBSET()
	{
		StartIndex = 0;
		IndexNum = 0;
	}
};

// ���f���\����
struct MODEL
{
	VERTEX_3D* VertexArray;
	unsigned short	VertexNum;
	unsigned short* IndexArray;
	unsigned short	IndexNum;
	SUBSET* SubsetArray;
	unsigned short	SubsetNum;
};


struct DX11_MODEL_MATERIAL
{
	MATERIAL					Material;
	ID3D11ShaderResourceView	*Texture;
};

// �`��T�u�Z�b�g�\����
struct DX11_SUBSET
{
	unsigned short	StartIndex;
	unsigned short	IndexNum;
	DX11_MODEL_MATERIAL	Material;
};

struct DX11_MODEL
{
	ID3D11Buffer*	VertexBuffer;
	ID3D11Buffer*	IndexBuffer;

	//DX11_SUBSET		*SubsetArray;
	//unsigned short	SubsetNum;

	MODEL*			modelData;
	SimpleArray<SimpleArray<VERTEX_3D>*> newVertexArrays;
	SimpleArray<SimpleArray<VERTEX_3D>*> oldVertexArrays;

	DX11_MODEL() 
	{
		modelData = new MODEL;
		VertexBuffer = NULL;
		IndexBuffer = NULL;

	}
};



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void LoadModel( char *FileName, DX11_MODEL *Model );
void UnloadModel( DX11_MODEL *Model );
void DrawModel( DX11_MODEL *Model );

// ���f���̃}�e���A���̃f�B�t���[�Y���擾����BMax16���ɂ��Ă���
void GetModelDiffuse(DX11_MODEL *Model, XMFLOAT4 *diffuse);

// ���f���̎w��}�e���A���̃f�B�t���[�Y���Z�b�g����B
void SetModelDiffuse(DX11_MODEL *Model, int mno, XMFLOAT4 diffuse);
void UpdateModel(DX11_MODEL* Model);


