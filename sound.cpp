//=============================================================================
//
// �T�E���h���� [sound.cpp]
//
//=============================================================================
#include "sound.h"

//*****************************************************************************
// �p�����[�^�\���̒�`
//*****************************************************************************
typedef struct
{
	char *pFilename;	// �t�@�C����
	int nCntLoop;		// ���[�v�J�E���g
	BOOL UseFilter;		//�G�t�F�N�g�g�����g��Ȃ����H�H
} SOUNDPARAM;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD *pChunkSize, DWORD *pChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
IXAudio2 *g_pXAudio2 = NULL;								// XAudio2�I�u�W�F�N�g�ւ̃C���^�[�t�F�C�X
IXAudio2MasteringVoice *g_pMasteringVoice = NULL;			// �}�X�^�[�{�C�X

IXAudio2SubmixVoice *g_apSubmixVoice;				//�T�u�~�b�N�X�{�C�X

IXAudio2SourceVoice *g_apSourceVoice[SOUND_LABEL_MAX] = {};	// �\�[�X�{�C�X
BYTE *g_apDataAudio[SOUND_LABEL_MAX] = {};					// �I�[�f�B�I�f�[�^
DWORD g_aSizeAudio[SOUND_LABEL_MAX] = {};					// �I�[�f�B�I�f�[�^�T�C�Y

// �e���f�ނ̃p�����[�^
SOUNDPARAM g_aParam[SOUND_LABEL_MAX] =
{
	{ (char*)"data/BGM/sample000.wav", -1 ,FALSE},	// BGM0
	{ (char*)"data/BGM/bgm_maoudamashii_neorock73.wav", -1 ,TRUE},	// BGM1
	{ (char*)"data/BGM/sample001.wav", -1 ,FALSE},	// BGM2
	{ (char*)"data/SE/bomb000.wav", 0 ,FALSE},		// �e���ˉ�
	{ (char*)"data/SE/defend000.wav", 0 ,FALSE},		// �e���ˉ�
	{ (char*)"data/SE/defend001.wav", 0 ,FALSE},		// �e���ˉ�
	{ (char*)"data/SE/hit000.wav", 0 ,FALSE},			// �e���ˉ�
	{ (char*)"data/SE/laser000.wav", 0 ,FALSE},		// �e���ˉ�
	{ (char*)"data/SE/lockon000.wav", 0 ,FALSE},		// �e���ˉ�
	{ (char*)"data/SE/shot000.wav", 0 ,FALSE},		// �e���ˉ�
	{ (char*)"data/SE/shot001.wav", 0 ,FALSE},		// �q�b�g��
};




//=============================================================================
// ����������
//=============================================================================
BOOL InitSound(HWND hWnd)
{
	HRESULT hr;

	// COM���C�u�����̏�����
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// XAudio2�I�u�W�F�N�g�̍쐬
	hr = XAudio2Create(&g_pXAudio2, 0);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "XAudio2�I�u�W�F�N�g�̍쐬�Ɏ��s�I", "�x���I", MB_ICONWARNING);

		// COM���C�u�����̏I������
		CoUninitialize();

		return FALSE;
	}
	
	// �}�X�^�[�{�C�X�̐���
	hr = g_pXAudio2->CreateMasteringVoice(&g_pMasteringVoice);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "�}�X�^�[�{�C�X�̐����Ɏ��s�I", "�x���I", MB_ICONWARNING);

		if(g_pXAudio2)
		{
			// XAudio2�I�u�W�F�N�g�̊J��
			g_pXAudio2->Release();
			g_pXAudio2 = NULL;
		}

		// COM���C�u�����̏I������
		CoUninitialize();

		return FALSE;
	}

	//�T�u�~�b�N�X�{�C�X�̍쐬
	hr = g_pXAudio2->CreateSubmixVoice(&g_apSubmixVoice, 2, 44800);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "�T�u�~�b�N�X�̐����Ɏ��s�I", "�x���I", MB_ICONWARNING);
		return FALSE;
	}

	// �T�E���h�f�[�^�̏�����
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		HANDLE hFile;
		DWORD dwChunkSize = 0;
		DWORD dwChunkPosition = 0;
		DWORD dwFiletype;
		WAVEFORMATEXTENSIBLE wfx;
		XAUDIO2_BUFFER buffer;

		// �o�b�t�@�̃N���A
		memset(&wfx, 0, sizeof(WAVEFORMATEXTENSIBLE));
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

		// �T�E���h�f�[�^�t�@�C���̐���
		hFile = CreateFile(g_aParam[nCntSound].pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(hWnd, "�T�E���h�f�[�^�t�@�C���̐����Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		if(SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{// �t�@�C���|�C���^��擪�Ɉړ�
			MessageBox(hWnd, "�T�E���h�f�[�^�t�@�C���̐����Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
	
		// WAVE�t�@�C���̃`�F�b�N
		hr = CheckChunk(hFile, 'FFIR', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVE�t�@�C���̃`�F�b�N�Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &dwFiletype, sizeof(DWORD), dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "WAVE�t�@�C���̃`�F�b�N�Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		if(dwFiletype != 'EVAW')
		{
			MessageBox(hWnd, "WAVE�t�@�C���̃`�F�b�N�Ɏ��s�I(3)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
	
		// �t�H�[�}�b�g�`�F�b�N
		hr = CheckChunk(hFile, ' tmf', &dwChunkSize, &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�t�H�[�}�b�g�`�F�b�N�Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		hr = ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�t�H�[�}�b�g�`�F�b�N�Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}

		// �I�[�f�B�I�f�[�^�ǂݍ���
		hr = CheckChunk(hFile, 'atad', &g_aSizeAudio[nCntSound], &dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�I�[�f�B�I�f�[�^�ǂݍ��݂Ɏ��s�I(1)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
		g_apDataAudio[nCntSound] = (BYTE*)malloc(g_aSizeAudio[nCntSound]);
		hr = ReadChunkData(hFile, g_apDataAudio[nCntSound], g_aSizeAudio[nCntSound], dwChunkPosition);
		if(FAILED(hr))
		{
			MessageBox(hWnd, "�I�[�f�B�I�f�[�^�ǂݍ��݂Ɏ��s�I(2)", "�x���I", MB_ICONWARNING);
			return FALSE;
		}
	
		// �\�[�X�{�C�X�̐���
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
				MessageBox(hWnd, "�\�[�X�{�C�X�̐����Ɏ��s�I", "�x���I", MB_ICONWARNING);
				return FALSE;
			}
		}
		// �o�b�t�@�̒l�ݒ�
		memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
		buffer.AudioBytes = g_aSizeAudio[nCntSound];
		buffer.pAudioData = g_apDataAudio[nCntSound];
		buffer.Flags      = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount  = g_aParam[nCntSound].nCntLoop;

		// �I�[�f�B�I�o�b�t�@�̓o�^
		g_apSourceVoice[nCntSound]->SubmitSourceBuffer(&buffer);
	}

	return TRUE;
}

//=============================================================================
// �I������
//=============================================================================
void UninitSound(void)
{
	// �ꎞ��~
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// �ꎞ��~
			g_apSourceVoice[nCntSound]->Stop(0);
	
			// �\�[�X�{�C�X�̔j��
			g_apSourceVoice[nCntSound]->DestroyVoice();
			g_apSourceVoice[nCntSound] = NULL;
	
			// �I�[�f�B�I�f�[�^�̊J��
			free(g_apDataAudio[nCntSound]);
			g_apDataAudio[nCntSound] = NULL;
		}
	}
	
	// �}�X�^�[�{�C�X�̔j��
	g_pMasteringVoice->DestroyVoice();
	g_pMasteringVoice = NULL;
	
	if(g_pXAudio2)
	{
		// XAudio2�I�u�W�F�N�g�̊J��
		g_pXAudio2->Release();
		g_pXAudio2 = NULL;
	}
	
	// COM���C�u�����̏I������
	CoUninitialize();
}

//=============================================================================
// �Z�O�����g�Đ�(�Đ����Ȃ��~)
//=============================================================================
void PlaySound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;
	XAUDIO2_BUFFER buffer;

	// �o�b�t�@�̒l�ݒ�
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));
	buffer.AudioBytes = g_aSizeAudio[label];
	buffer.pAudioData = g_apDataAudio[label];
	buffer.Flags      = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount  = g_aParam[label].nCntLoop;

	// ��Ԏ擾
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// �Đ���
		// �ꎞ��~
		g_apSourceVoice[label]->Stop(0);

		// �I�[�f�B�I�o�b�t�@�̍폜
		g_apSourceVoice[label]->FlushSourceBuffers();
	}

	//�\�[�X�{�C�X�̏o�͂��T�u�~�b�N�X�{�C�X�ɐ؂�ւ�
	XAUDIO2_SEND_DESCRIPTOR send = {0, g_apSubmixVoice};
	XAUDIO2_VOICE_SENDS sendlist = { 1, &send };
	g_apSourceVoice[label]->SetOutputVoices(&sendlist);

	// �I�[�f�B�I�o�b�t�@�̓o�^
	g_apSourceVoice[label]->SubmitSourceBuffer(&buffer);

	// �Đ�
	g_apSourceVoice[label]->Start(0);

}

//=============================================================================
// �Z�O�����g��~(���x���w��)
//=============================================================================
void StopSound(int label)
{
	XAUDIO2_VOICE_STATE xa2state;

	// ��Ԏ擾
	g_apSourceVoice[label]->GetState(&xa2state);
	if(xa2state.BuffersQueued != 0)
	{// �Đ���
		// �ꎞ��~
		g_apSourceVoice[label]->Stop(0);

		// �I�[�f�B�I�o�b�t�@�̍폜
		g_apSourceVoice[label]->FlushSourceBuffers();
	}
}

//=============================================================================
// �Z�O�����g��~(�S��)
//=============================================================================
void StopSound(void)
{
	// �ꎞ��~
	for(int nCntSound = 0; nCntSound < SOUND_LABEL_MAX; nCntSound++)
	{
		if(g_apSourceVoice[nCntSound])
		{
			// �ꎞ��~
			g_apSourceVoice[nCntSound]->Stop(0);
		}
	}
}

//=============================================================================
// �`�����N�̃`�F�b�N
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
	{// �t�@�C���|�C���^��擪�Ɉړ�
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	while(hr == S_OK)
	{
		if(ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
		{// �`�����N�̓ǂݍ���
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if(ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
		{// �`�����N�f�[�^�̓ǂݍ���
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch(dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize  = dwChunkDataSize;
			dwChunkDataSize = 4;
			if(ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
			{// �t�@�C���^�C�v�̓ǂݍ���
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if(SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{// �t�@�C���|�C���^���`�����N�f�[�^���ړ�
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
// �`�����N�f�[�^�̓ǂݍ���
//=============================================================================
HRESULT ReadChunkData(HANDLE hFile, void *pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset)
{
	DWORD dwRead;
	
	if(SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// �t�@�C���|�C���^���w��ʒu�܂ňړ�
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if(ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL) == 0)
	{// �f�[�^�̓ǂݍ���
		return HRESULT_FROM_WIN32(GetLastError());
	}
	
	return S_OK;
}

//=============================================================================
// ���[�p�X�t�B���^�[
//=============================================================================
void LowPassFilterON(int label)
{
	XAUDIO2_FILTER_PARAMETERS FilterParams;
	FilterParams.Type = XAUDIO2_FILTER_TYPE::LowPassFilter;
	FilterParams.Frequency = 0.5f;		//3546Hz����̎��g�����J�b�g�i44.1khz�̃f�[�^�̏ꍇ�j0.7f���炢�܂łŋC���������Ƃ��뎩���Œ������Ă���������
	FilterParams.OneOverQ = 1.4142f;	//��������1.0f
	g_apSourceVoice[label]->SetFilterParameters(&FilterParams); //�G�t�F�N�g���\�[�X�{�C�X�ɃZ�b�g
	return;
}


//=============================================================================
// �X�V����
//=============================================================================
void UpdateSound(void)
{
	SetReverb();
}



//=============================================================================
// ���o�[�u
//=============================================================================
void SetReverb(void)
{
	//���o�[�u�̖{�̂��쐬
	IUnknown *apXPO_Reverb;
	XAudio2CreateReverb(&apXPO_Reverb);

	//EFFECT_DESCRIPTOR�̍쐬
	XAUDIO2_EFFECT_DESCRIPTOR Descriptior_Reverb;
	Descriptior_Reverb.InitialState = true;					//�L����Ԃ�
	Descriptior_Reverb.OutputChannels = 2;					//2ch�̃G�t�F�N�g
	Descriptior_Reverb.pEffect = apXPO_Reverb;				//�G�t�F�N�g�{��

	//EFFECT_CHAIN�̍쐬
	XAUDIO2_EFFECT_CHAIN Chain_Reverb;
	Chain_Reverb.EffectCount = 1;							//�}���̂�1��
	Chain_Reverb.pEffectDescriptors = &Descriptior_Reverb;	//�������̍\���̂��w��

	//�{�C�X��EFFECT_CHAIN��}��
	g_apSubmixVoice->SetEffectChain(&Chain_Reverb);

	//Release
	apXPO_Reverb->Release();

	////I3DL2�`���̃v���Z�b�g���烊�o�[�u�p�����[�^�[�̍쐬
	//XAUDIO2FX_REVERB_I3DL2_PARAMETERS i3dl2Param = XAUDIO2FX_I3DL2_PRESET_BATHROOM;

	////�p�����[�^�̒���
	//XAUDIO2FX_REVERB_PARAMETERS reverbParam;

	//// I3DL2����REVERB_PARAM�ɕϊ�
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

	//�G�t�F�N�g�ɒʒm����
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

	//�G�t�F�N�g�ɒʒm����
	//g_apSubmixVoice->SetEffectParameters(0, &EQParam, sizeof(EQParam));
	return;

}