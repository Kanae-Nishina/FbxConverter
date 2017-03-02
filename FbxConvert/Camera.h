/*
	@file			Camera.h
	@brief		カメラ設定
	@date		2017/02/22
	@author	仁科香苗
*/
#pragma once
#include "Common.h"

class Camera
{
public:
	Camera();
	~Camera();

	void Init();				//初期化
	void Update();		//更新
	void Render();		//描画
	void Destroy();		//解放

	//ゲッター関数
	static D3DXMATRIX GetView() { return m_view; };
	static D3DXMATRIX GetProj() { return m_proj; };
	//セッター関数
	void SetPivotPos(D3DXVECTOR3 pivot) { m_pivotPos = pivot; };
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; };
	void SetLookAtPos(D3DXVECTOR3 look) { m_lookAtPos = look; };
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; };
private:
	float m_fovY;
	float m_nearZ;
	float m_farZ;
	D3DXVECTOR3 m_pivotPos;	//基準座標
	D3DXVECTOR3 m_pos;			//基準座標からの座標
	D3DXVECTOR3 m_lookAtPos;	//注視点
	D3DXVECTOR3 m_rot;			//回転
	static D3DXMATRIX m_view;	//ビュートランスフォーム
	static D3DXMATRIX m_proj;	//射影変換

};

