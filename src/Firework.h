#pragma once

#include <Windows.h>
#include <chrono>
#include <random>
#include <vector>

#include "resource.h"

struct HSB
{
	float h, s, b;
};
HSB operator+(const HSB& l, const HSB& r);
HSB operator-(const HSB& l, const HSB& r);
HSB operator*(const HSB& l, float f);
COLORREF HSB2RGB(HSB hsb);

class Firework
{
private:
	//eΪ��̬�������������ʱʱ��һ������һ���������
	static std::default_random_engine e;

	//����������������̻�������ʾ��
	bool dead;

	//true:��ʾ���Ӷ���
	//false:��ʾ�������
	bool boom;

	//�������
	float W, H;//��Ļ���
	float mr = 5;//����뾶
	float r = 50;//����������Ļ����
	float x0,y0;//������ʼ��
	float vx0,vy0;//���ٶ�
	float g = -40;//�������ٶ�
	HSB rocketHSB;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;//�������ʱ��

	//���Ӳ���
	struct Ball
	{
		float vx0, vy0;//���ٶ�
		float x0, y0;//��ʼ��
		float life;//���� ��
		float rb;//���뾶
		HSB startHSB, endHSB;//��ĩ��ɫ
		
	};
	std::vector<Ball> balls;
	std::chrono::time_point<std::chrono::high_resolution_clock> boomTime;//���ӱ�ըʱ��
public:
	Firework(float width, float height) :W(width), H(height), boom(false), dead(false)
	{
		using namespace std::chrono;
		startTime = high_resolution_clock::now();
		e.seed(e() + GetTickCount64());

		//�������
		x0 = randRange(r, W - r);
		y0 = 0;

		//x������ٶ�
		vx0 = randRange(-50.0, 50.0);

		if (vx0<2 * r || vx0>W - 2 * r)
			vx0 = -vx0;

		//ʹy�����յ�λ�� [H/2,H-r] ��Ҫ�ĳ��ٶȷ�Χ
		float vy0min = sqrt(2 * g * y0 - g * H);
		float vy0max = sqrt(2 * g * y0 - 2 * g * (H - r));

		float ymin = (2 * g * y0 - vy0min * vy0min) / (2.0 * g);
		float ymax = (2 * g * y0 - vy0max * vy0max) / (2.0 * g);

		//�ڷ�Χ�ڲ���y���ٶ�
		vy0 = randRange(vy0min, vy0max);

		float h = randRange(0, 360);
		float s = 1.0;
		float b = randRange(0.7, 1.0);//bԽ�󱥺Ͷ�Խ��
		rocketHSB = { h,s,b };
		//PlaySound((LPCSTR)IDR_WAVE_LAUNCH, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
	}

	float randRange(float a, float b)
	{
		std::uniform_real_distribution<> dis(a, b);
		return dis(e);
	}

	bool IsDead()
	{
		return dead;
	}



	void Draw(HDC hdc)
	{
		using namespace std;
		using namespace std::chrono;

		auto now = high_resolution_clock::now();
		if (boom == false)
		{
			//�ӷ�������ʱ��
			float t = duration_cast<milliseconds>(now - startTime).count() / 1000.0;

			//���y�����ٶ�
			float vy = vy0 + g * t;

			//���λ��
			float x = x0 + vx0 * t;
			float y = y0 + vy0 * t + 1.0 / 2.0 * g * t * t;

			//�����
			HBRUSH hBrush = CreateSolidBrush(HSB2RGB(rocketHSB));
			SelectObject(hdc, hBrush);

			HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
			SelectObject(hdc, hPen);

			int sy = H - y;
			Ellipse(hdc, x - mr, sy - mr, x + mr, sy + mr);

			DeleteObject(hPen);
			DeleteObject(hBrush);

			//����Ѿ����ﶥ��
			if (vy <= 0)
			{
				//��ը��Ч
				PlaySound((LPCSTR)IDR_WAVE_FIREWORK, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

				//���ÿ�ʼ��ը
				boom = true;

				//��¼��ը����ʱ��
				boomTime = now;
				balls.clear();
				
				//��������
				int i = randRange(40,80);
				while (i--)
				{
					Ball ball;
					ball.rb = randRange(3,5);
					ball.life = randRange(0.1,4);

					float v0 = randRange(20, 80);
					float pi = 3.1415926;
					float angle = randRange(-pi,pi);//���ٶȽǶ���360�ȷ�Χ��
					ball.vx0 = v0 * cos(angle);
					ball.vy0 = v0 * sin(angle);
					ball.x0 = x;
					ball.y0 = y;

					ball.startHSB = rocketHSB;

					//����ɫ������ƫ�Ƹ�45��
					//sԽСԽ�ף������������Ȼ����
					//bԽ�󱥺Ͷ�Խ��
					ball.endHSB = { rocketHSB.h+randRange(-45,45),rocketHSB.s+randRange(-0.7,-0.2),rocketHSB.b };
					balls.push_back(ball);
				}
			}
		}
		else
		{
			//��������

			bool allIsDead = true;
			float t = duration_cast<milliseconds>(now - boomTime).count() / 1000.0;
			for (auto& ball : balls)
			{
				if (t >= ball.life)
					continue;

				//��һ����û������������������
				allIsDead = false;

				//rbnowΪt�Ķ��κ���������Ϊrb
				float a = -4.0 * ball.rb / (ball.life * ball.life);
				float b = 4.0 * ball.rb / ball.life;
				float rbnow = a * t * t + b * t;

				//x��������ֱ��
				float xb = ball.x0 + ball.vx0 * t;

				//y�����ܵ�����
				float yb = ball.y0 + ball.vy0 * t + 1.0 / 2.0 * g * t * t;

				//���㵱ǰ��ɫ
				HSB hsb = (ball.endHSB - ball.startHSB) * (t / ball.life)+ball.startHSB;

				//��������
				HBRUSH hBrush = CreateSolidBrush(HSB2RGB(hsb));
				SelectObject(hdc, hBrush); 

				HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
				SelectObject(hdc, hPen);

				int drawY = H - yb;
				Ellipse(hdc, xb - rbnow, drawY - rbnow, xb + rbnow, drawY + rbnow);

				DeleteObject(hPen);
				DeleteObject(hBrush);
			}

			//�����������
			if (allIsDead)
				dead = true;
		}
	}
};

