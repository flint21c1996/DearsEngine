#pragma once
#include<unordered_map>
#include<string>
#include<functional>

#define PI 3.141592

class EasingFunc
{
public:
	EasingFunc();
	~EasingFunc();
public:
	void Initialize();
	std::unordered_map<std::string, std::function<void(float start, float end, float time, float* result)>> EasingFuncMap;

	bool TimeCheckHelper(float time, float end, float* result);
};

/// 클라에서 cpp를 볼 수 없으므로 이름을 적어둠[Riun]
// 사이트 참조 : https://www.researchgate.net/figure/Image-of-set-of-Penners-easing-functions_fig3_308007569
// 속도의 변화 정도 : Sine < Cubic < Quint < Circ < Back
// easeInSine : 점점 가속
// easeOutSine : 점점 감속
// easeInOutSine : 점점 가속하다 점점 감속
// easeInCubic : 점점 가속
// easeOutCubic : 점점 감속
// easeInOutCubic : 점점 가속하다 점점 감속
// easeInQuint : 점점 가속
// easeOutQuint : 점점 감속
// easeInOutQuint : 점점 가속하다 점점 감속
// easeInCirc : 점점 가속
// easeOutCirc : 점점 감속
// easeInOutCirc : 점점 가속하다 점점 감속
// easeInElastic : 속도의 변경 폭이 작다가 마지막에 증가하며 커짐
// easeOutElastic : 초반에 증가하고, 속도의 변경 폭이 크다가 작아짐
// easeInOutElastic : 중반에 증가하고, 속도의 변경 폭이 작다 크다 작아짐
// easeInBack : 점점 가속(끝 점이 최고점이 아님)
// easeOutBack : 점점 감속(끝 점이 최고점이 아님)
// easeInOutBack : 점점 가속하다 점점 감속(끝 점이 최고점이 아님)
// easeInBounce : 점점 더 튀어 오름
// easeOutBounce : 튀어오르는 수준이 반대인 것
// easeInOutBounce : 중반이 가장 많이 튀어오르는 것