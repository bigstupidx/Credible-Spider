#include "CardTurnOverAnimation.h"

#include "ValueAnimation.h"
#include "SettingAnimation.h"

namespace CardTurnOverAnimation
{
	std::vector<AbstractAnimation*> AddBackToFrontAnimation(Card& card)
	{
		std::vector<AbstractAnimation*> ret;
		//��ʾ����
		card.SetShow(false);

		//���淭������ʾ
		ret.push_back(new ValueAnimation<TImage, double>(&card.GetBackImage(), 25, &TImage::SetIWidth, 1.0, 0.0));

		//��������ʾ������
		ret.push_back(new SettingAnimation<Card, bool>(&card, 0, &Card::SetShow, true));

		//���淭����
		ret.push_back(new ValueAnimation<TImage, double>(&card.GetImage(), 25, &TImage::SetIWidth, 0.0, 1.0));

		//�����Ѿ���ʾ�����Իָ������iWidth
		ret.push_back(new SettingAnimation<TImage, double>(&card.GetBackImage(), 0, &TImage::SetIWidth, 1.0));

		return ret;
	}

	std::vector<AbstractAnimation*> AddFrontToBackAnimation(Card& card)
	{
		std::vector<AbstractAnimation*> ret;
		//��ʾ����
		card.SetShow(true);

		//���淭������ʾ
		ret.push_back(new ValueAnimation<TImage, double>(&card.GetImage(), 25, &TImage::SetIWidth, 1.0, 0.0));

		//��������ʾ�Ʊ���
		ret.push_back(new SettingAnimation<Card, bool>(&card, 0, &Card::SetShow, false));

		//���淭����
		ret.push_back(new ValueAnimation<TImage, double>(&card.GetBackImage(), 25, &TImage::SetIWidth, 0.0, 1.0));

		//�����Ѿ���ʾ�����Իָ������iWidth
		ret.push_back(new SettingAnimation<TImage, double>(&card.GetImage(), 0, &TImage::SetIWidth, 1.0));

		return ret;

	}
}