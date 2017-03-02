/*
	@file			Camera.h
	@brief		�J�����ݒ�
	@date		2017/02/22
	@author	�m�ȍ��c
*/
#pragma once
#include "Common.h"

class Camera
{
public:
	Camera();
	~Camera();

	void Init();				//������
	void Update();		//�X�V
	void Render();		//�`��
	void Destroy();		//���

	//�Q�b�^�[�֐�
	static D3DXMATRIX GetView() { return m_view; };
	static D3DXMATRIX GetProj() { return m_proj; };
	//�Z�b�^�[�֐�
	void SetPivotPos(D3DXVECTOR3 pivot) { m_pivotPos = pivot; };
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; };
	void SetLookAtPos(D3DXVECTOR3 look) { m_lookAtPos = look; };
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; };
private:
	float m_fovY;
	float m_nearZ;
	float m_farZ;
	D3DXVECTOR3 m_pivotPos;	//����W
	D3DXVECTOR3 m_pos;			//����W����̍��W
	D3DXVECTOR3 m_lookAtPos;	//�����_
	D3DXVECTOR3 m_rot;			//��]
	static D3DXMATRIX m_view;	//�r���[�g�����X�t�H�[��
	static D3DXMATRIX m_proj;	//�ˉe�ϊ�

};

