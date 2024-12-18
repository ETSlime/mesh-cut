//=============================================================================
//
// ���f���̏��� [model.cpp]
// Author : 
//
//=============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include "main.h"
#include "model.h"
#include "camera.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	VALUE_MOVE_MODEL	(0.50f)					// �ړ����x
#define	RATE_MOVE_MODEL		(0.20f)					// �ړ������W��
#define	VALUE_ROTATE_MODEL	(XM_PI * 0.05f)			// ��]���x
#define	RATE_ROTATE_MODEL	(0.20f)					// ��]�����W��
#define	SCALE_MODEL			(10.0f)					// ��]�����W��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void LoadObj( char *FileName, MODEL *Model );
void LoadMaterial( char *FileName, MODEL_MATERIAL **MaterialArray, unsigned short *MaterialNum );




//=============================================================================
// ����������
//=============================================================================
void LoadModel( char *FileName, DX11_MODEL *Model )
{
	//MODEL model;

	LoadObj( FileName, Model->modelData);



	// ���_�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( VERTEX_3D ) * Model->modelData->VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = Model->modelData->VertexArray;

		GetDevice()->CreateBuffer( &bd, &sd, &Model->VertexBuffer );
	}


	// �C���f�b�N�X�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( unsigned short ) * Model->modelData->IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = Model->modelData->IndexArray;

		GetDevice()->CreateBuffer( &bd, &sd, &Model->IndexBuffer );
	}

	// �T�u�Z�b�g�ݒ�
	//{
	//	Model->SubsetArray = new DX11_SUBSET[Model->model->SubsetNum ];
	//	Model->SubsetNum = Model->model->SubsetNum;

		for( unsigned short i = 0; i < Model->modelData->SubsetNum; i++ )
		{
			//Model->SubsetArray[i].StartIndex = Model->model->SubsetArray[i].StartIndex;
			//Model->SubsetArray[i].IndexNum = Model->model->SubsetArray[i].IndexNum;

			//Model->SubsetArray[i].Material.Material = Model->model->SubsetArray[i].Material.Material;

			D3DX11CreateShaderResourceViewFromFile( GetDevice(),
													Model->modelData->SubsetArray[i].Material.TextureName,
													NULL,
													NULL,
													&Model->modelData->SubsetArray[i].Material.Texture,
													NULL );
		}
	//}

	//delete[] model.VertexArray;
	//delete[] model.IndexArray;
	//delete[] model.SubsetArray;


}


//=============================================================================
// �I������
//=============================================================================
void UnloadModel( DX11_MODEL *Model )
{

	if( Model->VertexBuffer )		Model->VertexBuffer->Release();
	if( Model->IndexBuffer )		Model->IndexBuffer->Release();
	//if( Model->SubsetArray )		delete[] Model->SubsetArray;

	delete[] Model->modelData->VertexArray;
	delete[] Model->modelData->IndexArray;
	delete[] Model->modelData->SubsetArray;
}


//=============================================================================
// �`�揈��
//=============================================================================
void DrawModel( DX11_MODEL *Model )
{

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof( VERTEX_3D );
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers( 0, 1, &Model->VertexBuffer, &stride, &offset );

	// �C���f�b�N�X�o�b�t�@�ݒ�
	GetDeviceContext()->IASetIndexBuffer( Model->IndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	for( unsigned short i = 0; i < Model->modelData->SubsetNum; i++ )
	{
		// �}�e���A���ݒ�
		if (Model->modelData->SubsetArray[i].Material.Material.LoadMaterial)
			SetMaterial( Model->modelData->SubsetArray[i].Material.Material );

		// �e�N�X�`���ݒ�
		if (Model->modelData->SubsetArray[i].Material.Material.noTexSampling == 0)
		{
			GetDeviceContext()->PSSetShaderResources(0, 1, &Model->modelData->SubsetArray[i].Material.Texture);
		}

		// �|���S���`��
		GetDeviceContext()->DrawIndexed( Model->modelData->SubsetArray[i].IndexNum, Model->modelData->SubsetArray[i].StartIndex, 0 );
	}


}





//���f���Ǎ�////////////////////////////////////////////
void LoadObj( char *FileName, MODEL *Model )
{

	XMFLOAT3	*positionArray;
	XMFLOAT3	*normalArray;
	XMFLOAT2	*texcoordArray;

	unsigned short	positionNum = 0;
	unsigned short	normalNum = 0;
	unsigned short	texcoordNum = 0;
	unsigned short	vertexNum = 0;
	unsigned short	indexNum = 0;
	unsigned short	in = 0;
	unsigned short	subsetNum = 0;

	MODEL_MATERIAL	*materialArray = NULL;
	unsigned short	materialNum = 0;

	char str[256];
	char *s;
	char c;


	FILE *file;
	file = fopen( FileName, "rt" );
	if( file == NULL )
	{
		printf( "�G���[:LoadModel %s \n", FileName );
		return;
	}



	//�v�f���J�E���g
	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "v" ) == 0 )
		{
			positionNum++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			normalNum++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			texcoordNum++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			subsetNum++;
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			in = 0;

			do
			{
				fscanf( file, "%s", str );
				vertexNum++;
				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c!= '\r' );

			//�l�p�͎O�p�ɕ���
			if( in == 4 )
				in = 6;

			indexNum += in;
		}
	}


	//�������m��
	positionArray = new XMFLOAT3[ positionNum ];
	normalArray   = new XMFLOAT3[ normalNum ];
	texcoordArray = new XMFLOAT2[ texcoordNum ];


	Model->VertexArray = new VERTEX_3D[ vertexNum ];
	Model->VertexNum = vertexNum;

	Model->IndexArray = new unsigned short[ indexNum ];
	Model->IndexNum = indexNum;

	Model->SubsetArray = new SUBSET[ subsetNum ];
	Model->SubsetNum = subsetNum;

	


	//�v�f�Ǎ�
	XMFLOAT3 *position = positionArray;
	XMFLOAT3 *normal = normalArray;
	XMFLOAT2 *texcoord = texcoordArray;

	unsigned short vc = 0;
	unsigned short ic = 0;
	unsigned short sc = 0;


	fseek( file, 0, SEEK_SET );

	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "mtllib" ) == 0 )
		{
			//�}�e���A���t�@�C��
			fscanf( file, "%s", str );

			char path[256];
		//	strcpy( path, "data/model/" );

			//----------------------------------- �t�H���_�[�Ή�
			strcpy(path, FileName);
			char* adr = path;
			char* ans = adr;
			while (1)
			{
				adr = strstr(adr, "/");
				if (adr == NULL) break;
				else ans = adr;
				adr++;
			}
			if (path != ans) ans++;
			*ans = 0;
			//-----------------------------------

			strcat( path, str );

			LoadMaterial( path, &materialArray, &materialNum );
		}
		else if( strcmp( str, "o" ) == 0 )
		{
			//�I�u�W�F�N�g��
			fscanf( file, "%s", str );
		}
		else if( strcmp( str, "v" ) == 0 )
		{
			//���_���W
			fscanf( file, "%f", &position->x );
			fscanf( file, "%f", &position->y );
			fscanf( file, "%f", &position->z );
			position->x *= SCALE_MODEL;
			position->y *= SCALE_MODEL;
			position->z *= SCALE_MODEL;
			position++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			//�@��
			fscanf( file, "%f", &normal->x );
			fscanf( file, "%f", &normal->y );
			fscanf( file, "%f", &normal->z );
			normal++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			//�e�N�X�`�����W
			fscanf( file, "%f", &texcoord->x );
			fscanf( file, "%f", &texcoord->y );
			texcoord->y = 1.0f - texcoord->y;
			texcoord++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			//�}�e���A��
			fscanf( file, "%s", str );

			if( sc != 0 )
				Model->SubsetArray[ sc - 1 ].IndexNum = ic - Model->SubsetArray[ sc - 1 ].StartIndex;

			Model->SubsetArray[ sc ].StartIndex = ic;

			for( unsigned short i = 0; i < materialNum; i++ )
			{
				if( strcmp( str, materialArray[i].Name ) == 0 )
				{
					Model->SubsetArray[ sc ].Material.Material = materialArray[i].Material;
					Model->SubsetArray[sc].Material.Material.LoadMaterial = TRUE;
					strcpy( Model->SubsetArray[ sc ].Material.TextureName, materialArray[i].TextureName );
					strcpy( Model->SubsetArray[ sc ].Material.Name, materialArray[i].Name );

					break;
				}
			}

			sc++;
			
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			//��
			in = 0;

			do
			{
				fscanf( file, "%s", str );

				s = strtok( str, "/" );	
				Model->VertexArray[vc].Position = positionArray[ atoi( s ) - 1 ];
				if( s[ strlen( s ) + 1 ] != '/' )
				{
					//�e�N�X�`�����W�����݂��Ȃ��ꍇ������
					s = strtok( NULL, "/" );
					Model->VertexArray[vc].TexCoord = texcoordArray[ atoi( s ) - 1 ];
				}
				s = strtok( NULL, "/" );	
				Model->VertexArray[vc].Normal = normalArray[ atoi( s ) - 1 ];

				Model->VertexArray[vc].Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );

				Model->IndexArray[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' );

			//�l�p�͎O�p�ɕ���
			if( in == 4 )
			{
				Model->IndexArray[ic] = vc - 4;
				ic++;
				Model->IndexArray[ic] = vc - 2;
				ic++;
			}
		}
	}


	if( sc != 0 )
		Model->SubsetArray[ sc - 1 ].IndexNum = ic - Model->SubsetArray[ sc - 1 ].StartIndex;





	delete[] positionArray;
	delete[] normalArray;
	delete[] texcoordArray;
	delete[] materialArray;

	fclose(file);
}




//�}�e���A���ǂݍ���///////////////////////////////////////////////////////////////////
void LoadMaterial( char *FileName, MODEL_MATERIAL **MaterialArray, unsigned short *MaterialNum )
{
	char str[256];

	FILE *file;
	file = fopen( FileName, "rt" );
	if( file == NULL )
	{
		printf( "�G���[:LoadMaterial %s \n", FileName );
		return;
	}

	MODEL_MATERIAL *materialArray;
	unsigned short materialNum = 0;

	//�v�f���J�E���g
	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			materialNum++;
		}
	}


	//�������m��
	materialArray = new MODEL_MATERIAL[ materialNum ];
	ZeroMemory(materialArray, sizeof(MODEL_MATERIAL)*materialNum);


	//�v�f�Ǎ�
	int mc = -1;

	fseek( file, 0, SEEK_SET );

	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			//�}�e���A����
			mc++;
			fscanf( file, "%s", materialArray[ mc ].Name );
			strcpy( materialArray[ mc ].TextureName, "" );
			materialArray[mc].Material.noTexSampling = 1;
		}
		else if( strcmp( str, "Ka" ) == 0 )
		{
			//�A���r�G���g
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.z );
			materialArray[ mc ].Material.Ambient.w = 1.0f;
		}
		else if( strcmp( str, "Kd" ) == 0 )
		{
			//�f�B�t���[�Y
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.z );

			// Maya�Ńe�N�X�`����\���0.0f�ɂȂ����Ⴄ�݂����Ȃ̂�
			if ((materialArray[mc].Material.Diffuse.x + materialArray[mc].Material.Diffuse.y + materialArray[mc].Material.Diffuse.z) == 0.0f)
			{
				materialArray[mc].Material.Diffuse.x = materialArray[mc].Material.Diffuse.y = materialArray[mc].Material.Diffuse.z = 1.0f;
			}

			materialArray[ mc ].Material.Diffuse.w = 1.0f;
		}
		else if( strcmp( str, "Ks" ) == 0 )
		{
			//�X�y�L����
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.z );
			materialArray[ mc ].Material.Specular.w = 1.0f;
		}
		else if( strcmp( str, "Ns" ) == 0 )
		{
			//�X�y�L�������x
			fscanf( file, "%f", &materialArray[ mc ].Material.Shininess );
		}
		else if( strcmp( str, "d" ) == 0 )
		{
			//�A���t�@
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.w );
		}
		else if( strcmp( str, "map_Kd" ) == 0 )
		{
			//�e�N�X�`��
			fscanf( file, "%s", str );

			char path[256];
		//	strcpy( path, "data/model/" );

			//----------------------------------- �t�H���_�[�Ή�
			strcpy(path, FileName);
			char* adr = path;
			char* ans = adr;
			while (1)
			{
				adr = strstr(adr, "/");
				if (adr == NULL) break;
				else ans = adr;
				adr++;
			}
			if (path != ans) ans++;
			*ans = 0;
			//-----------------------------------

			strcat( path, str );

			strcat( materialArray[ mc ].TextureName, path );
			materialArray[mc].Material.noTexSampling = 0;
		}
	}


	*MaterialArray = materialArray;
	*MaterialNum = materialNum;

	fclose(file);
}


// ���f���̑S�}�e���A���̃f�B�t���[�Y���擾����BMax16���ɂ��Ă���
void GetModelDiffuse(DX11_MODEL *Model, XMFLOAT4 *diffuse)
{
	int max = (Model->modelData->SubsetNum < MODEL_MAX_MATERIAL) ? Model->modelData->SubsetNum : MODEL_MAX_MATERIAL;

	for (unsigned short i = 0; i < max; i++)
	{
		// �f�B�t���[�Y�ݒ�
		diffuse[i] = Model->modelData->SubsetArray[i].Material.Material.Diffuse;
	}
}


// ���f���̎w��}�e���A���̃f�B�t���[�Y���Z�b�g����B
void SetModelDiffuse(DX11_MODEL *Model, int mno, XMFLOAT4 diffuse)
{
	// �f�B�t���[�Y�ݒ�
	Model->modelData->SubsetArray[mno].Material.Material.Diffuse = diffuse;
}


void UpdateModel(DX11_MODEL* Model)
{
	UnloadModel(Model);

	unsigned short vertexCnt = 0, indexCnt = 0, newSubsetCnt = 0, oldSubsetCnt = 0;
	int newVertexArraySize = Model->newVertexArrays.getSize();
	int oldVertexArraySize = Model->oldVertexArrays.getSize();
	for (int i = 0; i < newVertexArraySize; i++)
	{
		SimpleArray<VERTEX_3D>* subsetVertex = Model->newVertexArrays[i];
		int subsetVertexCnt = subsetVertex->getSize();
		if (subsetVertexCnt != 0)
		{
			newSubsetCnt++;
			vertexCnt += subsetVertexCnt;
		}
	}
	for (int i = 0; i < oldVertexArraySize; i++)
	{
		SimpleArray<VERTEX_3D>* subsetVertex = Model->oldVertexArrays[i];
		int subsetVertexCnt = subsetVertex->getSize();
		if (subsetVertexCnt != 0)
		{
			oldSubsetCnt++;
			vertexCnt += subsetVertexCnt;
		}
	}

	Model->modelData->SubsetNum = newSubsetCnt + oldSubsetCnt;
	Model->modelData->SubsetArray = new SUBSET[Model->modelData->SubsetNum];

	Model->modelData->VertexNum = vertexCnt;
	Model->modelData->IndexNum = vertexCnt;
	Model->modelData->VertexArray = new VERTEX_3D[Model->modelData->VertexNum];
	Model->modelData->IndexArray = new unsigned short[Model->modelData->IndexNum];

	vertexCnt = 0;

	for (int i = 0; i < newSubsetCnt; i++)
	{
		SUBSET subset;
		SimpleArray<VERTEX_3D>* subsetVertex = Model->newVertexArrays[i];
		int subsetVertexCnt = subsetVertex->getSize();
		subset.StartIndex = indexCnt;
		subset.IndexNum = subsetVertexCnt;
		Model->modelData->SubsetArray[i] = subset;
		for (int j = 0; j < subsetVertexCnt; j++)
		{
			Model->modelData->VertexArray[vertexCnt++] = (*subsetVertex)[j];
			Model->modelData->IndexArray[indexCnt++] = indexCnt;
		}

		delete Model->newVertexArrays[i];
	}

	for (int i = 0; i < oldSubsetCnt; i++)
	{
		SUBSET subset;
		SimpleArray<VERTEX_3D>* subsetVertex = Model->oldVertexArrays[i];
		int subsetVertexCnt = subsetVertex->getSize();
		subset.StartIndex = indexCnt;
		subset.IndexNum = subsetVertexCnt;
		Model->modelData->SubsetArray[i + newSubsetCnt] = subset;
		for (int j = 0; j < subsetVertexCnt; j++)
		{
			Model->modelData->VertexArray[vertexCnt++] = (*subsetVertex)[j];
			Model->modelData->IndexArray[indexCnt++] = indexCnt;
		}

		delete Model->oldVertexArrays[i];
	}

	//Model->modelData->VertexNum = Model->newVertex.getSize();
	//Model->modelData->IndexNum = Model->newIndex.getSize();
	//Model->modelData->SubsetNum = Model->newSubsetArray.getSize();

	//for (int i = 0; i < Model->modelData->VertexNum; i++)
	//{
	//	Model->modelData->VertexArray[i] = Model->newVertex[i];
	//}

	//for (int i = 0; i < Model->modelData->IndexNum; i++)
	//{
	//	Model->modelData->IndexArray[i] = Model->newIndex[i];
	//}

	//Model->modelData->SubsetArray[0].IndexNum = Model->modelData->IndexNum;

	// ���_�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * Model->modelData->VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = Model->modelData->VertexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer);
	}


	// �C���f�b�N�X�o�b�t�@����
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned short) * Model->modelData->IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = Model->modelData->IndexArray;

		GetDevice()->CreateBuffer(&bd, &sd, &Model->IndexBuffer);
	}


}



