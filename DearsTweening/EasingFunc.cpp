#include "EasingFunc.h"

EasingFunc::EasingFunc()
{
	Initialize();
}

EasingFunc::~EasingFunc()
{

}

void EasingFunc::Initialize()
{
	EasingFuncMap["easeInSine"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * (1 - std::cos((time * PI) / 2));
			}
		};

	EasingFuncMap["easeOutSine"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * std::sin((time * PI) / 2);
			}
		};
	EasingFuncMap["easeInOutSine"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * (-(std::cos(PI * time) - 1) / 2);
			}
		};

	EasingFuncMap["easeInCubic"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * (time * time * time);
			}
		};

	EasingFuncMap["easeOutCubic"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * (1 - std::pow(1 - time, 3));
			}
		};

	EasingFuncMap["easeInOutCubic"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				if (time < 0.5)
				{
					*result = start + (end - start) * (4 * time * time * time);
				}
				else
				{
					*result = start + (end - start) * (1 - std::pow(-2 * time + 2, 3) / 2);
				}
			}
		};

	EasingFuncMap["easeInQuint"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * (time * time * time * time * time);
			}
		};

	EasingFuncMap["easeOutQuint"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * (1 - std::pow(1 - time, 5));
			}
		};

	EasingFuncMap["easeInOutQuint"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				if (time < 0.5) {
					double x = 16 * std::pow(time, 5);
					*result = start + (end - start) * x;
				}
				else {
					double x = std::pow(-2 * time + 2, 5) / 2;
					*result = start + (end - start) * (1 - x);
				}
			}
		};

	EasingFuncMap["easeInCirc"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * (1 - std::sqrt(1 - std::pow(time, 2)));
			}
		};

	EasingFuncMap["easeOutCirc"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				*result = start + (end - start) * std::sqrt(1 - std::pow(time - 1, 2));
			}
		};

	EasingFuncMap["easeInOutCirc"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				if (time < 0.5) {
					*result = start + (end - start) * ((1 - std::sqrt(1 - std::pow(2 * time, 2))) / 2);
				}
				else {
					*result = start + (end - start) * ((std::sqrt(1 - std::pow(-2 * time + 2, 2)) + 1) / 2);
				}
			}
		};

	EasingFuncMap["easeInElastic"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double c4 = (2 * PI) / 3;

				if (time == 0) {
					*result = start;
				}
				else if (time == 1) {
					*result = end;
				}
				else {
					*result = start + (end - start) * (-std::pow(2, 10 * time - 10) * std::sin((time * 10 - 10.75) * c4));
				}
			}
		};

	EasingFuncMap["easeOutElastic"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double c4 = (2 * PI) / 3;

				if (time == 0) {
					*result = start;
				}
				else if (time == 1) {
					*result = end;
				}
				else {
					*result = start + (end - start) * (std::pow(2, -10 * time) * std::sin((time * 10 - 0.75) * c4) + 1);
				}
			}
		};

	EasingFuncMap["easeInOutElastic"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double c5 = (2 * PI) / 4.5;

				if (time == 0) {
					*result = start;
				}
				else if (time == 1) {
					*result = end;
				}
				else if (time < 0.5) {
					double x = std::pow(2, 20 * time - 10) * std::sin((20 * time - 11.125) * c5);
					*result = start - (end - start) * x / 2;
				}
				else {
					double x = std::pow(2, -20 * time + 10) * std::sin((20 * time - 11.125) * c5);
					*result = start + (end - start) * x / 2 + (end - start);
				}
			}
		};

	EasingFuncMap["easeInBack"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double c1 = 1.70158;
				const double c3 = c1 + 1;

				// easeInBack 이징 계산
				*result = start + (end - start) * (c3 * std::pow(time, 3) - c1 * std::pow(time, 2));
			}
		};

	EasingFuncMap["easeOutBack"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double c1 = 1.70158;
				const double c3 = c1 + 1;

				// easeOutBack 이징 계산
				*result = start + (end - start) * (1 + c3 * std::pow(time - 1, 3) + c1 * std::pow(time - 1, 2));
			}
		};

	EasingFuncMap["easeInOutBack"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double c1 = 1.70158;
				const double c2 = c1 * 1.525;

				if (time < 0.5) {
					double x = 2 * time;
					*result= start + (end - start) * (std::pow(x, 2) * ((c2 + 1) * x - c2)) / 2;
				}
				else {
					double x = 2 * time - 2;
					*result = start + (end - start) * (std::pow(x, 2) * ((c2 + 1) * x + c2) + 2) / 2;
				}
			}
		};

	EasingFuncMap["easeInBounce"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{

				const double n1 = 7.5625;
				const double d1 = 2.75;
				double x =1- time;

				double tempResult;

				if (x < 1 / d1) {
					tempResult = n1 * x * x;
				}
				else if (x < 2 / d1) {
					x -= 1.5 / d1;
					tempResult = n1 * x * x + 0.75;
				}
				else if (x < 2.5 / d1) {
					x -= 2.25 / d1;
					tempResult = n1 * x * x + 0.9375;
				}
				else {
					x -= 2.625 / d1;
					tempResult = n1 * x * x + 0.984375;
				}

				// 시작점과 끝점 사이의 값으로 변환
				*result = start + (end - start) * (1 - tempResult);
			}
		};
	EasingFuncMap["easeOutBounce"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double n1 = 7.5625;
				const double d1 = 2.75;
				double x = time;

				double tempResult;

				if (x < 1 / d1) {
					tempResult = n1 * x * x;
				}
				else if (x < 2 / d1) {
					x -= 1.5 / d1;
					tempResult = n1 * x * x + 0.75;
				}
				else if (x < 2.5 / d1) {
					x -= 2.25 / d1;
					tempResult = n1 * x * x + 0.9375;
				}
				else {
					x -= 2.625 / d1;
					tempResult = n1 * x * x + 0.984375;
				}

				// 시작점과 끝점 사이의 값으로 변환
				*result = start + (end - start) * tempResult;
			}
		};
	EasingFuncMap["easeInOutBounce"] = [this](float start, float end, float time, float* result)
		{
			if (TimeCheckHelper(time, end, result))
			{
				const double n1 = 7.5625;
				const double d1 = 2.75;

				if (time < 0.5)
				{
					double x = 1 - 2 * time;
					double tempResult;

					if (x < 1 / d1) {
						tempResult = n1 * x * x;
					}
					else if (x < 2 / d1)
					{
						x -= 1.5 / d1;
						tempResult = n1 * x * x + 0.75;
					}
					else if (x < 2.5 / d1)
					{
						x -= 2.25 / d1;
						tempResult = n1 * x * x + 0.9375;
					}
					else
					{
						x -= 2.625 / d1;
						tempResult = n1 * x * x + 0.984375;
					}

					*result = start + (end - start) * (1 - tempResult) / 2;
				}
				else
				{
					double x = 2 * time - 1;
					double tempResult;

					if (x < 1 / d1)
					{
						tempResult = n1 * x * x;
					}
					else if (x < 2 / d1)
					{
						x -= 1.5 / d1;
						tempResult = n1 * x * x + 0.75;
					}
					else if (x < 2.5 / d1)
					{
						x -= 2.25 / d1;
						tempResult = n1 * x * x + 0.9375;
					}
					else {
						x -= 2.625 / d1;
						tempResult = n1 * x * x + 0.984375;
					}
					*result = start + (end - start) * (1 + tempResult) / 2;
				}
			}
		};

}



bool EasingFunc::TimeCheckHelper(float time, float end, float* result)
{
	if (time <= 1)
	{
		return true;
	}
	else
	{
		*result = end;
		return false;
	}
}
