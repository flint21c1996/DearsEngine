#include <vector>
#include <functional>
#include <iostream>

class FunctionManager 
{
public:
	// 다양한 시그니처의 함수를 저장하기 위한 템플릿 메소드
	template<typename Func>
	void addFunction(Func func) 
	{
		functions.emplace_back([func](auto&&... args) {
			return func(std::forward<decltype(args)>(args)...);
			});
	}

	// 함수 호출 메소드
	template<typename... Args>
	void callFunctions(Args&&... args) {
		for (auto& func : functions) {
			func(std::forward<Args>(args)...);
		}
	}

private:
	std::vector<std::function<void()>> functions;
};

int main() {
	FunctionManager manager;

	// 다양한 시그니처의 함수 추가
	manager.addFunction([]() {
		std::cout << "No argument function\n";
		});

	manager.addFunction([](int x) {
		std::cout << "Function with one int argument: " << x << "\n";
		});

	manager.addFunction([](int x, double y) {
		std::cout << "Function with int and double arguments: " << x << ", " << y << "\n";
		});

	// 함수 호출
	manager.callFunctions();        // No argument function 호출
	manager.callFunctions(10);      // Function with one int argument 호출
	manager.callFunctions(10, 20.5); // Function with int and double arguments 호출

	return 0;
}