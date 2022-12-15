//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "model.h"
#include "enemy.h"
#include "shadow.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY			"data/MODEL/enemy.obj"		// 読み込むモデル名

#define	MODEL_CUBE0			"data/MODEL/enemy3.obj"		// 読み込むモーフィング名
#define	MODEL_CUBE1			"data/MODEL/enemy3_2.obj"		// 読み込むモーフィング名



#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define ENEMY_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define ENEMY_OFFSET_Y		(7.0f)						// エネミーの足元をあわせる


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];				// エネミー

static BOOL				g_Load = FALSE;

//モーフィング用
static ENEMY			g_Cube[2];						// エネミー
static MODEL			g_Cube_Vertex[2];				//頂点情報の入ったデータ配列
static VERTEX_3D		*g_Vertex = NULL;				//途中経過を記録する場所

static float			g_time;
static int				g_mof;



static INTERPOLATION_DATA move_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(   0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*1 },
	{ XMFLOAT3(-200.0f, ENEMY_OFFSET_Y, 200.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*0.5f },
	{ XMFLOAT3(   0.0f, ENEMY_OFFSET_Y,  20.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60*2 },

};


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = TRUE;

		g_Enemy[i].pos = XMFLOAT3(-50.0f + i * 30.0f, ENEMY_OFFSET_Y, 20.0f);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Enemy[i].spd = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);

		g_Enemy[i].move_time = 0.0f;	// 線形補間用のタイマーをクリア
		g_Enemy[i].tbl_adr = NULL;		// 再生するアニメデータの先頭アドレスをセット
		g_Enemy[i].tbl_size = 0;		// 再生するアニメデータのレコード数をセット

		g_Enemy[i].use = TRUE;			// TRUE:生きてる

	}

	{
		LoadModel(MODEL_CUBE0, &g_Cube[0].model);
		g_Cube[0].load = TRUE;

		g_Cube[0].pos = XMFLOAT3(-50.0f, ENEMY_OFFSET_Y, 20.0f);
		g_Cube[0].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Cube[0].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Cube[0].spd = 0.0f;			// 移動スピードクリア
		g_Cube[0].size = ENEMY_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Cube[0].model, &g_Cube[0].diffuse[0]);
		
		g_Cube[0].use = TRUE;			// TRUE:生きてる

	}



	//モーフィングするデータの準備
	LoadObj(MODEL_CUBE0, &g_Cube_Vertex[0]);
	LoadObj(MODEL_CUBE1, &g_Cube_Vertex[1]);



	//中身を配列として使用できるように
	g_Vertex = new VERTEX_3D[g_Cube_Vertex[0].VertexNum];

	//差分の初期化
	for (int i = 0; i < g_Cube_Vertex[0].VertexNum; i++)
	{
		g_Vertex[i].Position = g_Cube_Vertex[0].VertexArray[i].Position;
		g_Vertex[i].Diffuse = g_Cube_Vertex[0].VertexArray[i].Diffuse;
		g_Vertex[i].Normal = g_Cube_Vertex[0].VertexArray[i].Normal;
		g_Vertex[i].TexCoord = g_Cube_Vertex[0].VertexArray[i].TexCoord;
	}

	g_time = 0.0f;
	g_mof = 0;

	// 0番だけ線形補間で動かしてみる
	g_Enemy[0].move_time = 0.0f;		// 線形補間用のタイマーをクリア
	g_Enemy[0].tbl_adr = move_tbl;		// 再生するアニメデータの先頭アドレスをセット
	g_Enemy[0].tbl_size = sizeof(move_tbl) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	if (g_Load == FALSE) return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = FALSE;
		}
	}

	UnloadModel(&g_Cube[0].model);

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			if (g_Enemy[i].tbl_adr != NULL)	// 線形補間を実行する？
			{								// 線形補間の処理
				// 移動処理
				int		index = (int)g_Enemy[i].move_time;
				float	time = g_Enemy[i].move_time - index;
				int		size = g_Enemy[i].tbl_size;

				float dt = 1.0f / g_Enemy[i].tbl_adr[index].frame;	// 1フレームで進める時間
				g_Enemy[i].move_time += dt;							// アニメーションの合計時間に足す

				if (index > (size - 2))	// ゴールをオーバーしていたら、最初へ戻す
				{
					g_Enemy[i].move_time = 0.0f;
					index = 0;
				}

				// 座標を求める	X = StartX + (EndX - StartX) * 今の時間
				XMVECTOR p1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].pos);	// 次の場所
				XMVECTOR p0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].pos);	// 現在の場所
				XMVECTOR vec = p1 - p0;
				XMStoreFloat3(&g_Enemy[i].pos, p0 + vec * time);

				// 回転を求める	R = StartX + (EndX - StartX) * 今の時間
				XMVECTOR r1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].rot);	// 次の角度
				XMVECTOR r0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].rot);	// 現在の角度
				XMVECTOR rot = r1 - r0;
				XMStoreFloat3(&g_Enemy[i].rot, r0 + rot * time);

				// scaleを求める S = StartX + (EndX - StartX) * 今の時間
				XMVECTOR s1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].scl);	// 次のScale
				XMVECTOR s0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].scl);	// 現在のScale
				XMVECTOR scl = s1 - s0;
				XMStoreFloat3(&g_Enemy[i].scl, s0 + scl * time);

			}

			// 影もプレイヤーの位置に合わせる
			XMFLOAT3 pos = g_Enemy[i].pos;
			pos.y -= (ENEMY_OFFSET_Y - 0.1f);
			SetPositionShadow(g_Enemy[i].shadowIdx, pos);
		}


		//モーフィング処理
		{
			int after, before;

			after = (g_mof + 1) % 2;
			before = g_mof % 2;


			for (int i = 0; i < g_Cube_Vertex[0].VertexNum; i++)
			{
				g_Vertex[i].Position.x = g_Cube_Vertex[after].VertexArray[i].Position.x - g_Cube_Vertex[before].VertexArray[i].Position.x;
				g_Vertex[i].Position.y = g_Cube_Vertex[after].VertexArray[i].Position.y - g_Cube_Vertex[before].VertexArray[i].Position.y;
				g_Vertex[i].Position.z = g_Cube_Vertex[after].VertexArray[i].Position.z - g_Cube_Vertex[before].VertexArray[i].Position.z;

				g_Vertex[i].Position.x *= g_time;
				g_Vertex[i].Position.y *= g_time;
				g_Vertex[i].Position.z *= g_time;

				g_Vertex[i].Position.x += g_Cube_Vertex[after].VertexArray[i].Position.x;
				g_Vertex[i].Position.y += g_Cube_Vertex[after].VertexArray[i].Position.y;
				g_Vertex[i].Position.z += g_Cube_Vertex[after].VertexArray[i].Position.z;

			}

			g_time += 0.01f;

			if (g_time > 1.0f)
			{
				g_mof++;
				g_time = 0.0f;
			}

			//頂点情報を上書き
			{
				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(VERTEX_3D) * g_Cube_Vertex[0].VertexNum;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA sd;
				ZeroMemory(&sd, sizeof(sd));
				sd.pSysMem = g_Vertex;

				GetDevice()->CreateBuffer(&bd, &sd, &g_Cube[i].model.VertexBuffer);
			}
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == FALSE) continue;

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
		DrawModel(&g_Enemy[i].model);
	}

	{
		XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Cube[0].scl.x, g_Cube[0].scl.y, g_Cube[0].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Cube[0].rot.x, g_Cube[0].rot.y + XM_PI, g_Cube[0].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Cube[0].pos.x, g_Cube[0].pos.y, g_Cube[0].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Cube[0].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Cube[0].model);
	}




	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}
