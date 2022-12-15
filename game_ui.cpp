//=============================================================================
//
// UI���� [gema_ui.cpp]
// Author : GP11A132_31_�����D��
//
//=============================================================================
#include "main.h"
#include "model.h"
#include "renderer.h"
#include "sprite.h"
#include "input.h"
#include "game_ui.h"
#include "player.h"
#include "enemy.h"
#include "score.h"
#include "sound.h"


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(80)	// �L�����T�C�Y�i���j
#define TEXTURE_HEIGHT				(80)	// �L�����T�C�Y�i�����j
#define TEXTURE_MAX					(5)		// �e�N�X�`���̐�


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// ���_���
static ID3D11ShaderResourceView	*g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����q

//�ǂݍ��ރt�@�C����
static char *g_TexturName[TEXTURE_MAX] = {
	"data/UI/megaphone.png",
	"data/UI/megaphone_moji1.png",
	"data/UI/megaphone_moji2.png",
	"data/UI/megaphone_moji3.png",
	"data/UI/megaphone_moji4.png",
};

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static GAME_UI		g_UI[UI_MAX];		// UI�\����

int cnthp, cntbullet;					// �\�����邽�߂̃v���C���[HP�A�o���b�g�J�E���^
int timecnt;							// �ً}�̃e�N�X�`����\�����邽�߂̃^�C���J�E���^

int mojinumber;						// �����Ă�G�l�~�[�̐�

static float	alpha;
static BOOL	flag_alpha;


//=============================================================================
// ����������
//=============================================================================
HRESULT InitGameUI(void)
{
	ID3D11Device *pDevice = GetDevice();

	//�e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// UI�\���̂̏�����
	for (int i = 0; i < UI_MAX; i++)
	{

		g_UI[i].use = TRUE;
		g_UI[i].pos = XMFLOAT3(10.0f, 10.0f, 0.0f);	// ���S�_����\��
		g_UI[i].w = TEXTURE_WIDTH;
		g_UI[i].h = TEXTURE_HEIGHT;
		g_UI[i].texNo = i;


		//switch (i)
		//{
		//case 0:	//���C�t
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(10.0f, 10.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = i;
		//	break;

		//case 1:	//�̂ǃX�v���[
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(10.0f, 100.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = i;
		//	break;

		//case 2:	//�ً}
		//	g_UI[i].use = FALSE;
		//	g_UI[i].pos = XMFLOAT3(SCREEN_CENTER_X, SCREEN_CENTER_Y, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = 400.0f;
		//	g_UI[i].h = 400.0f;
		//	g_UI[i].texNo = i;
		//	g_UI[i].kinkyuflag = FALSE;
		//	break;

		//case 3:	//o
		//	g_UI[i].use = FALSE;
		//	g_UI[i].pos = XMFLOAT3(500.0f, 120.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = i;
		//	break;

		//case 4:	//ti
		//	g_UI[i].use = FALSE;
		//	g_UI[i].pos = XMFLOAT3(540.0f, 120.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = i;
		//	break;

		//case 5:	//n
		//	g_UI[i].use = FALSE;
		//	g_UI[i].pos = XMFLOAT3(580.0f, 120.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = i;
		//	break;

		//case 6:	//ti
		//	g_UI[i].use = FALSE;
		//	g_UI[i].pos = XMFLOAT3(620.0f, 120.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 4;
		//	break;

		//case 7:	//n
		//	g_UI[i].use = FALSE;
		//	g_UI[i].pos = XMFLOAT3(660.0f, 120.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 5;
		//	break;


		//case 8:	//a
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(700.0f, 30.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 6;
		//	break;

		//case 9:	//i
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(750.0f, 30.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 7;
		//	break;

		//case 10:	//u
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(800.0f, 30.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 8;
		//	break;

		//case 11:	//e
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(850.0f, 30.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 9;
		//	break;

		//case 12:	//o2
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(900.0f, 30.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 10;
		//	break;

		//case 13:	//o2
		//	g_UI[i].use = TRUE;
		//	g_UI[i].pos = XMFLOAT3(710.0f, 80.0f, 0.0f);	// ���S�_����\��
		//	g_UI[i].w = TEXTURE_WIDTH;
		//	g_UI[i].h = TEXTURE_HEIGHT;
		//	g_UI[i].texNo = 11;
		//	break;
		//}
	}

	g_UI[1].w = 120;
	g_UI[1].h = 45;

	g_UI[2].w = 120;
	g_UI[2].h = 45;

	g_UI[3].w = 120;
	g_UI[3].h = 45;

	mojinumber = 0;

	BOOL g_OptinUse = FALSE;	// �I�v�V�������g���Ă��邩

	alpha = 1.0f;				// �ً}�e�N�X�`���̓����Ɏg���ϐ�
	flag_alpha = TRUE;			// �ً}�e�N�X�`���̓����Ɏg���ϐ�

	g_Load = TRUE;	// �f�[�^�̏��������s����
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitGameUI(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateGameUI(void)
{
	// �v���C���[�̃��C�t�A�o���b�g���J�E���g
	PLAYER *player = GetPlayer();


	// �I���J�[�\���̈ړ�
	if ((GetKeyboardTrigger(DIK_UP)) || (IsButtonTriggered(0, BUTTON_UP)))
	{
		mojinumber--;

		if (mojinumber < 0)
		{
			mojinumber = 2;
		}
		// ���ʉ��Đ�
		//PlaySound(SOUND_LABEL_SE_selectkey);
	}
	if ((GetKeyboardTrigger(DIK_DOWN)) || (IsButtonTriggered(0, BUTTON_DOWN)))
	{
		mojinumber++;

		if (mojinumber > 2)
		{
			mojinumber = 0;
		}
		// ���ʉ��Đ�
		//PlaySound(SOUND_LABEL_SE_selectkey);
	}

	g_UI[1].use = FALSE;
	g_UI[2].use = FALSE;
	g_UI[3].use = FALSE;

	g_UI[mojinumber+1].use = TRUE;

	//cnthp = player->life;
	//cntbullet = player->cbullet;

	//if (!player->cankukyokubullet)
	//{
	//	if (player->kukyokubullet[0] == TRUE)
	//	{
	//		g_UI[3].use = TRUE;
	//	}
	//	else
	//	{
	//		g_UI[3].use = FALSE;
	//	}
	//	if (player->kukyokubullet[1] == TRUE)
	//	{
	//		g_UI[4].use = TRUE;
	//	}
	//	else
	//	{
	//		g_UI[4].use = FALSE;
	//	}

	//	if (player->kukyokubullet[2] == TRUE)
	//	{
	//		g_UI[5].use = TRUE;
	//	}
	//	else
	//	{
	//		g_UI[5].use = FALSE;
	//	}

	//	if (player->kukyokubullet[3] == TRUE)
	//	{
	//		g_UI[6].use = TRUE;
	//	}
	//	else
	//	{
	//		g_UI[6].use = FALSE;
	//	}

	//	if (player->kukyokubullet[4] == TRUE)
	//	{
	//		g_UI[7].use = TRUE;
	//	}
	//	else
	//	{
	//		g_UI[7].use = FALSE;
	//	}
	//}
	//else
	//{
	//	g_UI[3].use = FALSE;
	//	g_UI[4].use = FALSE;
	//	g_UI[5].use = FALSE;
	//	g_UI[6].use = FALSE;
	//	g_UI[7].use = FALSE;

	//}

	//g_UI[13].pos = XMFLOAT3(710.0f + 50.0f * (GetNowBullet() % BULLET_MAX), 80.0f, 0.0f);	// ���S�_����\��

}


//=============================================================================
// �`�揈��
//=============================================================================
void DrawGameUI(void)
{

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	//for (int i = 0; i < TEXTURE_MAX-3; i++)
	//{
	//	switch (i)
	//	{
	//	case 0:
	//		for (int j = 0; j < cnthp; j++)
	//		{
	//			g_UI[i].pos = XMFLOAT3(30.0f + 50.0f * j, 30.0f, 0.0f);	// ���S�_����\��

	//			// �e�N�X�`���ݒ�
	//			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_UI[i].texNo]);

	//			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	//			SetSpriteColor(g_VertexBuffer,
	//				g_UI[i].pos.x, g_UI[i].pos.y, g_UI[i].w, g_UI[i].h,
	//				0.0f, 0.0f, 1.0f, 1.0f,
	//				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	//			// �|���S���`��
	//			GetDeviceContext()->Draw(4, 0);
	//		}
	//		break;

	//	case 1:
	//		for (int j = 0; j < cntbullet; j++)
	//		{
	//			g_UI[i].pos = XMFLOAT3(30.0f + 40.0f * j, 80.0f, 0.0f);	// ���S�_����\��

	//			// �e�N�X�`���ݒ�
	//			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_UI[i].texNo]);

	//			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	//			SetSpriteColor(g_VertexBuffer,
	//				g_UI[i].pos.x, g_UI[i].pos.y, g_UI[i].w, g_UI[i].h,
	//				0.0f, 0.0f, 1.0f, 1.0f,
	//				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	//			// �|���S���`��
	//			GetDeviceContext()->Draw(4, 0);
	//		}
	//		break;
	//	}
	//}

	//if (g_UI[2].use)
	//{
	//	// �e�N�X�`���ݒ�
	//	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_UI[2].texNo]);

	//	// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
	//	SetSpriteColor(g_VertexBuffer,
	//		g_UI[2].pos.x, g_UI[2].pos.y, g_UI[2].w, g_UI[2].h,
	//		0.0f, 0.0f, 1.0f, 1.0f,
	//		XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

	//	// �|���S���`��
	//	GetDeviceContext()->Draw(4, 0);
	//}

	for (int i = 3; i < UI_MAX; i++)
	{
		if (g_UI[i].use)
		{
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_UI[i].texNo]);

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColor(g_VertexBuffer,
				g_UI[i].pos.x, g_UI[i].pos.y, g_UI[i].w, g_UI[i].h,
				0.0f, 0.0f, 1.0f, 1.0f,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}
}