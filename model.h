//=============================================================================
//
// モデルの処理 [model.h]
// Author :
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "SimpleArray.h"
//*********************************************************
// 構造体
//*********************************************************

// マテリアル構造体

#define MODEL_MAX_MATERIAL		(16)		// １モデルのMaxマテリアル数


// マテリアル構造体
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

// 描画サブセット構造体
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

// モデル構造体
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

// 描画サブセット構造体
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
// プロトタイプ宣言
//*****************************************************************************
void LoadModel( char *FileName, DX11_MODEL *Model );
void UnloadModel( DX11_MODEL *Model );
void DrawModel( DX11_MODEL *Model );

// モデルのマテリアルのディフューズを取得する。Max16個分にしてある
void GetModelDiffuse(DX11_MODEL *Model, XMFLOAT4 *diffuse);

// モデルの指定マテリアルのディフューズをセットする。
void SetModelDiffuse(DX11_MODEL *Model, int mno, XMFLOAT4 diffuse);
void UpdateModel(DX11_MODEL* Model);


