//=============================================================================
//
// サウンド処理 [sound.cpp]
//
//=============================================================================
#include "sound.h"

//*****************************************************************************
// パラメータ構造体定義
//*****************************************************************************
typedef struct
{
	char *pFilename;	// ファイル名
	int nCntLoop;		// ループカウント
	BOOL UseFilter;		//エフェクト使うか使わないか？？
} SOUNDPARAM;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
IXAudio2 *g_pXAudio2 = NULL;								// XAudio2オブジェクトへのインターフェイス
IXAudio2MasteringVoice *g_pMasteringVoice = NULL;			// マスターボイス

IXAudio2SubmixVoice *g_apSubmixVoice;				//サブミックスボイス

IXAudio2SourceVoice *g_apSourceVoice[SOUND_LABEL_MAX] = {};	// ソースボイス
BYTE *g_apDataAudio[SOUND_LABEL_MAX] = {};					// オーディオデータ
DWORD g_aSizeAudio[SOUND_LABEL_MAX] = {};					// オーディオデータサイズ

// 各音素材のパラメータ
SOUNDPARAM g_aParam[SOUND_LABEL_MAX] =
{
	{ (char*)"data/BGM/sample000.wav", -1 ,FALSE},	// BGM0
	{ (char*)"data/BGM/bgm_maoudamashii_neorock73.wav", -1 ,TRUE},	// BGM1
	{ (char*)"data/BGM/sample001.wav", -1 ,FALSE},	// BGM2
	{ (char*)"data/SE/bomb000.wav", 0 ,FALSE},		// 弾発射音
	{ (char*)"data/SE/defend000.wav", 0 ,FALSE},		// 弾発射音
	{ (char*)"data/SE/defend001.wav", 0 ,FALSE},		// 弾発射音
	{ (char*)"data/SE/hit000.wav", 0 ,FALSE},			// 弾発射音
	{ (char*)"data/SE/laser000.wav", 0 ,FALSE},		// 弾発射音
	{ (char*)"data/SE/lockon000.wav", 0 ,FALSE},		// 弾発射音
	{ (char*)"data/SE/shot000.wav", 0 ,FALSE},		// 弾発射音
	{ (char*)"data/SE/shot001.wav", 0 ,FALSE},		// ヒット音
};




//=============================================================================
// 初期化処理
//=============================================================================
BOOL InitSound(HWND hWnd)
{
	HRESULT hr;

	// COMライブラリの初期化
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// XAudio2オブジェクトの作成
	hr = XAudio2Create(&g_pXAudio2, 0);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "XAudio2オブジェクトの作成に失敗！", "警告！", MB_ICONWARNING);

		// COMライブラリの終了処理
		CoUninitialize();

		return FALSE;
	}
	
	// マスターボイスの生成
	hr = g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "マスターボイスの生成に失敗！", "警告！", MB_ICONWARNING);

		if(g_pXAudio2)
		{
			// XAudio2オブジェクトの開放
			g_pXAudio2->Release();
			g_pXAudio2 = NULL;
		}

		// COMライブラリの終了処理
		CoUninitialize();

		return FALSE;
	}

	//サブミックスボイスの作成
	hr = g_pXAudio2->CreateSubmixVoice(&g_apSubmixVoice, 2, 44800);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "サブミックスの生成に失敗！", "警告！", MB_ICONWARNING);
		return FALSE;
	}

	// サウンドデータの初期化
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		HANDLE hFile;
		DWORD dwChunkSize = 0;
		DWORD dwChunkPosition = 0;
		DWORD dwFiletype;
		WAVEFORMATEXTENSIBLE wfx;
		XAUDIO2_BUFFER buffer;

		// バッファのクリア
		memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

		// サウンドデータファイルの生成
		hFile = CreateFile(g_aParam[nCntSound].pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{// ファイルポインタを先頭に移動
			MessageBox(hWnd, "サウンドデータファイルの生成に失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// WAVEファイルのチェック
		hr = CheckChunk(hFile, 'FFIR', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &dwFiletype, sizeof(DWORD), dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		if(dwFiletype != 'EVAW')
		{
			MessageBox(hWnd, "WAVEファイルのチェックに失敗！(3)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// フォーマットチェック
		hr = CheckChunk(hFile, ' tmf', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "フォーマットチェックに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}

		// オーディオデータ読み込み
		hr = CheckChunk(hFile, 'atad', &g_aSizeAudio[nCntSound], &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(1)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
		g_apDataAudio[nCntSound] = (BYTE*)malloc(g_aSizeAudio[nCntSound]);
		hr = ReadChunkData(hFile, g_apDataAudio[nCntSound], g_aSizeAudio[nCntSound], dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "オーディオデータ読み込みに失敗！(2)", "警告！", MB_ICONWARNING);
			return FALSE;
		}
	
		// ソースボイスの生成
		{
			if (g_aParam[nCntSound].UseFilter == TRUE)
			{
				hr = g_pXAudio2->CreateSourceVoice(&g_apSourceVoice[nCntSound], &(wfx.Format), XAUDIO2_VOICE_USEFILTER, 16.0f);
			}
			else
			{
				hr = g_pXAudio2->CreateSourceVoice(&g_apSourceVoice[nCntSound], &(wfx.Format), 0, 16.0f);
			}
			if (FAILED(hr))
			{
				MessageBox(hWnd, "ソースボイスの生成に失敗！", "警告！", MB_ICONWARNING);
				return FALSE;
			}
		}
		// バッファの値設定
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
		buffer.AudioBytes = g_aSizeAudio[nCntSound];
		buffer.pAudioData = g_apDataAudio[nCntSound];
		buffer.Flags      = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount  = g_aParam[nCntSound].nCntLoop;

		// オーディオバッファの登録
		g_apSourceVoice[nCntSound]->SubmitSourceBuffer(&buffer);
	}

	return TRUE;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
	
			// ソースボイスの破棄
			g_apSourceVoice[nCntSound]->DestroyVoice();
			g_apSourceVoice[nCntSound] = NULL;
	
			// オーディオデータの開放
			free(g_apDataAudio[nCntSound]);
			g_apDataAudio[nCntSound] = NULL;
		}
	}
	
	// マスターボイスの破棄
	g_pMasteringVoice->DestroyVoice();
	g_pMasteringVoice = NULL;
	
	if(g_pXAudio2)
	{
		// XAudio2オブジェクトの開放
		g_pXAudio2->Release();
		g_pXAudio2 = NULL;
	}
	
	// COMライブラリの終了処理
	CoUninitialize();
}

//=============================================================================
// セグメント再生(再生中なら停止)
//=============================================================================
void PlaySound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;
	XAUDIO2_BUFFER buffer;

	// バッファの値設定
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	buffer.AudioBytes = g_aSizeAudio[label];
	buffer.pAudioData = g_apDataAudio[label];
	buffer.Flags      = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount  = g_aParam[label].nCntLoop;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}

	//ソースボイスの出力をサブミックスボイスに切り替え
	XAUDIO2_SEND_DESCRIPTOR send = {0, g_apSubmixVoice};
	XAUDIO2_VOICE_SENDS sendlist = { 1, &send };
	g_apSourceVoice[label]->SetOutputVoices(&sendlist);

	// オーディオバッファの登録
	g_apSourceVoice[label]->SubmitSourceBuffer(&buffer);

	// 再生
	g_apSourceVoice[label]->Start(0);

}

//=============================================================================
// セグメント停止(ラベル指定)
//=============================================================================
void StopSound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;

	// 状態取得
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// 再生中
		// 一時停止
		g_apSourceVoice[label]->Stop(0);

		// オーディオバッファの削除
		g_apSourceVoice[label]->FlushSourceBuffers();
	}
}

//=============================================================================
// セグメント停止(全て)
//=============================================================================
void StopSound(void)
{
	// 一時停止
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// 一時停止
			g_apSourceVoice[nCntSound]->Stop(0);
		}
	}
}

//=============================================================================
// チャンクのチェック
//=============================================================================
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition)
{
	HRESULT hr = S_OK;
	DWORD dwRead;
	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD dwBytesRead = 0;
	DWORD dwOffset = 0;
	
	if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを先頭に移動
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	while(hr == S_OK)
	{
		if(ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if(ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクデータの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch(dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize  = dwChunkDataSize;
			dwChunkDataSize = 4;
			if(ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
			{// ファイルタイプの読み込み
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if(SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{// ファイルポインタをチャンクデータ分移動
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		dwOffset += sizeof(DWORD) * 2;
		if(dwChunkType == format)
		{
			*pChunkSize         = dwChunkDataSize;
			*pChunkDataPosition = dwOffset;

			return S_OK;
		}

		dwOffset += dwChunkDataSize;
		if(dwBytesRead >= dwRIFFDataSize)
		{
			return S_FALSE;
		}
	}
	
	return S_OK;
}

//=============================================================================
// チャンクデータの読み込み
//=============================================================================
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset)
{
	DWORD dwRead;
	
	if(SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを指定位置まで移動
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if(ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL) == 0)
	{// データの読み込み
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}

//=============================================================================
// ローパスフィルター
//=============================================================================
void LowPassFilterON(int label)
{
	XAUDIO2_FILTER_PARAMETERS FilterParams;
	FilterParams.Type = XAUDIO2_FILTER_TYPE::LowPassFilter;
	FilterParams.Frequency = 0.5f;		//3546Hzより上の周波数をカット（44.1khzのデータの場合）0.7fくらいまでで気持ちいいところ自分で調整してもいいかな
	FilterParams.OneOverQ = 1.4142f;	//もしくは1.0f
	g_apSourceVoice[label]->SetFilterParameters(&FilterParams); //エフェクトをソースボイスにセット
	return;
}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateSound(void)
{
	SetReverb();
}



//=============================================================================
// リバーブ
//=============================================================================
void SetReverb(void)
{
	//リバーブの本体を作成
	IUnknown *apXPO_Reverb;
	XAudio2CreateReverb(&apXPO_Reverb);

	//EFFECT_DESCRIPTORの作成
	XAUDIO2_EFFECT_DESCRIPTOR Descriptior_Reverb;
	Descriptior_Reverb.InitialState = true;					//有効状態に
	Descriptior_Reverb.OutputChannels = 2;					//2chのエフェクト
	Descriptior_Reverb.pEffect = apXPO_Reverb;				//エフェクト本体

	//EFFECT_CHAINの作成
	XAUDIO2_EFFECT_CHAIN Chain_Reverb;
	Chain_Reverb.EffectCount = 1;							//挿すのは1つ
	Chain_Reverb.pEffectDescriptors = &Descriptior_Reverb;	//さっきの構造体を指示

	//ボイスにEFFECT_CHAINを挿す
	g_apSubmixVoice->SetEffectChain(&Chain_Reverb);

	//Release
	apXPO_Reverb->Release();

	////I3DL2形式のプリセットからリバーブパラメーターの作成
	//XAUDIO2FX_REVERB_I3DL2_PARAMETERS i3dl2Param = XAUDIO2FX_I3DL2_PRESET_BATHROOM;

	////パラメータの調整
	//XAUDIO2FX_REVERB_PARAMETERS reverbParam;

	//// I3DL2からREVERB_PARAMに変換
	//ReverbConvertI3DL2ToNative(&i3dl2Param, &reverbParam);

	//g_apSubmixVoice->SetEffectParameters(0, &reverbParam, sizeof(reverbParam));

}

void EchoON(void)
{
	IUnknown *apXPO_Echo;
	CreateFX(_uuidof(FXEcho), &apXPO_Echo);

	XAUDIO2_EFFECT_DESCRIPTOR descriptor_Echo;
	descriptor_Echo.InitialState = true;
	descriptor_Echo.OutputChannels = 2;
	descriptor_Echo.pEffect = apXPO_Echo;


	XAUDIO2_EFFECT_CHAIN chain_Echo;
	chain_Echo.EffectCount = 1;
	chain_Echo.pEffectDescriptors = &descriptor_Echo;

	g_apSubmixVoice->SetEffectChain(&chain_Echo);

	apXPO_Echo->Release();

	FXECHO_PARAMETERS EchoParam;
	EchoParam.WetDryMix = FXECHO_DEFAULT_WETDRYMIX;
	EchoParam.Delay = FXECHO_DEFAULT_DELAY;
	EchoParam.Feedback = FXECHO_DEFAULT_FEEDBACK;

	//エフェクトに通知する
	g_apSubmixVoice->SetEffectParameters(0, &EchoParam, sizeof(EchoParam));
	return;
}





void EQON(void)
{
	IUnknown *apXPO_EQ;
	CreateFX(_uuidof(FXEQ), &apXPO_EQ);

	XAUDIO2_EFFECT_DESCRIPTOR descriptor_EQ;
	descriptor_EQ.InitialState = true;
	descriptor_EQ.OutputChannels = 2;
	descriptor_EQ.pEffect = apXPO_EQ;

	XAUDIO2_EFFECT_CHAIN chain_EQ;
	chain_EQ.EffectCount = 1;
	chain_EQ.pEffectDescriptors = &descriptor_EQ;
	g_apSubmixVoice->SetEffectChain(&chain_EQ);

	apXPO_EQ->Release();

	//FXEQ_PARAMETERS EQParam;
	//EQParam.FrequencyCenter0 = FXEQ_DEFAULT_FREQUENCY_CENTER_0;
	//EQParam.FrequencyCenter1 = FXEQ_DEFAULT_FREQUENCY_CENTER_1;
	//EQParam.FrequencyCenter2 = FXEQ_DEFAULT_FREQUENCY_CENTER_2;
	//EQParam.FrequencyCenter3 = FXEQ_DEFAULT_FREQUENCY_CENTER_3;

	//EQParam.Bandwidth0 = FXEQ_DEFAULT_BANDWIDTH;
	//EQParam.Bandwidth1 = FXEQ_DEFAULT_BANDWIDTH;
	//EQParam.Bandwidth2 = FXEQ_DEFAULT_BANDWIDTH;
	//EQParam.Bandwidth3 = FXEQ_DEFAULT_BANDWIDTH;

	//EQParam.Gain0 = FXEQ_DEFAULT_GAIN;
	//EQParam.Gain1 = FXEQ_DEFAULT_GAIN;
	//EQParam.Gain2 = FXEQ_DEFAULT_GAIN;
	//EQParam.Gain3 = FXEQ_DEFAULT_GAIN;

	//エフェクトに通知する
	//g_apSubmixVoice->SetEffectParameters(0, &EQParam, sizeof(EQParam));
	return;

}
