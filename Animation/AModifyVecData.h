#pragma once
#include "Animation.h"

class AModifyVecData :
	public Animation
{
private:
	enum modifyType { Add, Remove } mType;
	std::vector<POINT> *vecPt;
	POINT pt;
public:
	//���
	AModifyVecData(std::vector<POINT> *vecPt, POINT pt) :vecPt(vecPt), pt(pt){ mType = Add; };

	//ɾ��
	AModifyVecData(std::vector<POINT> *vecPt) :vecPt(vecPt){ mType = Remove; };
	void AModifyVecData::Do(HDC hdc) override;
};

