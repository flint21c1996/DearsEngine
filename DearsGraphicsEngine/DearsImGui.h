#pragma once
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h> //占쏙옙占쏙옙占?占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙킬占?ImGui占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占싹댐옙 占쏙옙荑?占쏙옙占?
#include <vector>
#include "UIEnum.h"
#include "GraphicsResourceManager.h"
#include "IEditorPanel.h"

/// <summary>
/// 占쌩곤옙占쌔억옙 占쏙옙 占쏙옙
/// 占쏙옙占쏙옙화
/// </summary>
class InputManager;

/// <summary>
/// ImGui占쏙옙 占쏙옙占쏙옙歐占?占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占?클占쏙옙占쏙옙
/// </summary>
class DearsImGui
{
public:
	DearsImGui(HWND _hWnd, ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11DeviceContext>& m_pDeviceContext, int _endScreenWidth, int _endScreenHeight, GraphicsResourceManager* _pResourceManager);
	~DearsImGui();

	InputManager* m_pInputManager;

private:
	ComPtr<ID3D11Device> m_pDevice;					   // 占쌓뤄옙占싫쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙決占쏙옙占?占쌨아온댐옙.
	GraphicsResourceManager* m_pResourceManager;	   // 占쏙옙占쌀쏙옙 占신댐옙占쏙옙占쏙옙 占쏙옙占쏙옙占싶몌옙 占쏙옙占쏙옙決占쏙옙占쏙옙占?占쌨아온댐옙 -> 占쏙옙占쏙옙占싱너울옙 占쏙옙占쏙옙占싹깍옙 占쏙옙占쏙옙 호占쏙옙
	int m_endScreenWidth;							   // 占쏙옙크占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	int m_endScreenHeight;							   // 占쏙옙크占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	ImGuiWindow* window;							   // ImGui占쏙옙 화占쏙옙占쏙옙 占쏙옙占쏙옙키占쏙옙 占쏙옙占쏙옙
	ImGuiIO* io;									   // 창占쏙옙 占십깍옙화占싹거놂옙 占쏙옙占쏙옙占싹는듸옙 占쏙옙占?

	Vector2 t_imagePos = Vector2(150, 600);			   // 占쏙옙占쏙옙 占쏙옙占쏙옙

public:
	/// 占십깍옙화占쏙옙 占쏙옙占쏙옙占싹댐옙 占싸븝옙 : 占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙 (占쏙옙占쏙옙占쏙옙占?占쏙옙占쏙옙)
	void UILoadFonts(std::string _basePath, std::string _fileName, float _size, bool _isKorean);	// 占쏙옙트占쏙옙 占싸듸옙占싼댐옙. (占쏙옙트 占쏙옙占쏙옙 占쏙옙占? 占쏙옙트 占쏙옙占쏙옙占쏙옙, 占싼깍옙 占쏙옙占쏙옙)
	void UIBuildFonts();			// 占쏙옙트占쏙옙 占싸듸옙占쏙옙 占쏙옙, 占쏙옙트 占쌔쏙옙처占쏙옙 占쌕쏙옙 占쏙옙占쏙옙

	/// D3D 占쏙옙占쏙옙 占쏙옙占쏙옙 占쌍깍옙 (占쏙옙占쏙옙占쏙옙占?占쏙옙占쏙옙)
	void UIBeginRender(); // BeginRender 占쏙옙占쏙옙 호占쏙옙
	void UISetting();		 // 占쌓몌옙 UI占쏙옙 占쏙옙占쏙옙
	void UIRender();		 // ImGui 占쏙옙占쏙옙占쏙옙

	///  D3D EndRender 占쏙옙占쏙옙 占쌍깍옙
	void UIEndRender();	 // EndRender 占쏙옙占쏙옙 호占쏙옙

	/// 캔占쏙옙占쏙옙 占쏙옙占쏙옙
	void UICanvasSet(Vector2 _posXY, Vector2 _sizeWH);	// 캔占쏙옙占쏙옙 占쏙옙占쏙옙 (占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占싸쇽옙占쏙옙)
	void UICanvasSetFin();										// 캔占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
	void NewUISetWindow(Vector2 _posXY, Vector2 _sizeWH, const std::string _windowName); // 占쏙옙占싸울옙 창占쏙옙 占쏙옙占쏙옙 占쏙옙 占쏙옙占?(占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占싸쇽옙占쏙옙, 占쏙옙占쏙옙창占쏙옙 占싱몌옙(占쌩븝옙x))
	void SetUICurrentWindow();							// 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙, 占썩본 占쌓몌옙占쏙옙 占쌉쇽옙占쏙옙 占쌕뤄옙 占쏙옙占쏙옙 호占쏙옙

	/// 占썩본 占쌓몌옙占쏙옙
	// 占쌔쏙옙트
	void UIDrawText(Vector2 _posXY, const std::string _text, Vector4 _rgba = Vector4(0.0f, 0.0f, 0.0f, 1.0f));				// 占쌔쏙옙트占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙) 
	// 占쏙옙占쌘곤옙 占쌍댐옙 占쌔쏙옙트
	template<typename ...Args>
	void UIDrawTextWithNum(Vector2 _posXY, const std::string _formatText, Vector4 _rgba, Args&&... args);
	// 占썹각占쏙옙
	void UIDrawRect(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0F);				// 占썹각占쏙옙占쏙옙 占쌓두몌옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占싸쇽옙占쏙옙, RGBA, 占쏜서몌옙 占쌌깍옙 占쏙옙占쏙옙, 占싸뀐옙)
	void UIDrawRectFilled(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f);							// 채占쏙옙占쏙옙 占썹각占쏙옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占싸쇽옙占쏙옙, RGBA, 占쏜서몌옙 占쌌깍옙 占쏙옙占쏙옙)
	void UIDrawRectwithBorder(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0F);	// 占썹각占쏙옙占쏙옙 占쌓두몌옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙(占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占싸쇽옙占쏙옙, RGBA, 占쏜서몌옙 占쌌깍옙 占쏙옙占쏙옙, 占싸뀐옙)
	void UIFreeRect(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness);	// 占쏙옙占쏙옙占쏙옙 占썹각占쏙옙占쏙옙 占쌓두몌옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (point 占시곤옙占쏙옙占쏙옙占쏙옙占?4占쏙옙, RGBA, 占싸뀐옙)
	void UIFreeRectFilled(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba);				// 채占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占썹각占쏙옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (point 占시곤옙占쏙옙占쏙옙占쏙옙占?4占쏙옙, RGBA)
	void UIFreeRectwithBorder(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness, Vector4 _borderRgba); // 占쏙옙占쏙옙占쏙옙 占썹각占쏙옙占쏙옙 占쌓두몌옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (point 占시곤옙占쏙옙占쏙옙占쏙옙占?4占쏙옙, RGBA, 占싸뀐옙, 占싸뀐옙 RGBA)
	// 占쏙옙
	void UIDrawLine(Vector2 _sPosXY, Vector2 _ePosXY, Vector4 _rgba);		// 占쏙옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (占쏙옙占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占쏙옙 占쏙옙치xy, RGBA)
	// 占쏙옙
	void UIDrawCircle(Vector2 _posXY, float _radius, Vector4 _rgba);		// 占쏙옙占쏙옙 占쌓몌옙占쏙옙 占쌉쇽옙 (占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占쏙옙占쏙옙, RGBA)
	
	/// 占싱뱄옙占쏙옙 占쌓몌옙占쏙옙
	void UIDrawImage(Vector2 _posXY, Vector2 _sizeWH, ComPtr<ID3D11ShaderResourceView> _srv, Vector4 _rgba = Vector4(1.0f, 1.0f, 1.0f, 1.0f)); // 占싱뱄옙占쏙옙占쏙옙 占쌓몌옙占쏙옙 占쏙옙占쏙옙 占썩본占쏙옙占쏙옙 占쌉쇽옙 (占쏙옙占쏙옙 占쏙옙치xy, 占쏙옙占싸쇽옙占쏙옙, 占쌔쏙옙처, 占쌔쏙옙처 占쌩곤옙 占쏙옙占쏙옙)

	/// 占쏙옙占?
	bool Hovering();			// 占쏙옙占쎌스占쏙옙 占쏙옙占쏙옙 占쏙옙占싱듸옙 占쏙옙占쏙옙占쏙옙트 占쏙옙占쏙옙 占쏙옙占쏙옙占싹댐옙占쏙옙 확占쏙옙
	bool Dragging();			// 占쏙옙占쎌스占쏙옙 占쏙옙占쏙옙 占썲래占쏙옙 占쏙옙占쏙옙占쏙옙 확占쏙옙
	void ResetDragDelta();		// 占썲래占쏙옙 占쏙옙占쏙옙 占쏙옙치占쏙옙占쏙옙 占쏙옙占쏙옙

	/// 占쏙옙占쏙옙 占쏙옙占싱듸옙 占쌩곤옙占쏙옙 占쏙옙占쏙옙
	// ImGui占쏙옙占쏙옙 占쏙옙트占쏙옙 占쏙옙占쏙옙求占?占쏙옙占쏙옙占?占싹뱄옙占쏙옙占쏙옙 占쌨몌옙 占쌀댐옙占?占쌕몌옙占쏙옙.
	// ImFont 占쏙옙체占쏙옙 ImGuiIO占쏙옙 占쏙옙트 占쏙옙占시울옙 占쏙옙占쏙옙퓔占? 占쏙옙占쏙옙占쏙옙 占쌀댐옙占싱놂옙 占쏙옙占쏙옙占쏙옙 占십요가 占쏙옙占쏙옙.
	void UIDrawImageStart();	// 占쏙옙占쏙옙占쏙옙트 占쏙옙占쏙옙 占쏙옙占싱듸옙 占쌩곤옙
	void UIDrawImageFin();			// 占쏙옙占쏙옙占쏙옙트 占쏙옙占쏙옙 占쏙옙占싱듸옙 占쏙옙占쏙옙
	void UIStartFontID(ImFont* _imFont);	// 占쏙옙트 占쏙옙占싱듸옙 占쌩곤옙
	void UIEndFontID();			// 占쏙옙트 占쏙옙占싱듸옙 占쏙옙占쏙옙

	/// C++20占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 UTF-8 占쏙옙占쌘울옙占쏙옙 char8_t 占쏙옙占쌉울옙 占쏙옙占쏙옙 占쌘뤄옙占쏙옙占쏙옙占쏙옙 占쌕뀐옙占?占쏙옙占쏙옙占쏙옙 占싱몌옙 占쌕쏙옙 占쏙옙占쌘울옙占쏙옙 占쏙옙환占싹댐옙 占쌉쇽옙
	std::string ConvertUTF8String(const std::u8string& _text); // u8 占쌔쏙옙트占쏙옙 String 占쏙옙占쏙옙占쏙옙 占쏙옙환

	/// ?먮뵒???⑤꼸 愿由?
	void AddPanel(IEditorPanel* panel);

private:
	std::vector<IEditorPanel*> m_panels;
	void Drag(Vector2& _posXY);
};

template<typename ...Args>
void DearsImGui::UIDrawTextWithNum(Vector2 _posXY, const std::string _formatText, Vector4 _rgba, Args&&... args)
{
	char buffer[256]; // 占쏙옙占쏙옙占?큰 占쏙옙占쌜몌옙 확占쏙옙 -> 占쏙옙占쏙옙 占쏙옙획占쏙옙占쏙옙 占쏙옙占쏙옙占싹댐옙 占싹몌옙 512占쏙옙 占쌕뀐옙 占쏙옙
	sprintf_s(buffer, sizeof(buffer), _formatText.c_str(), std::forward<Args>(args)...);

	UIDrawText(_posXY, std::string(buffer)/*fullText*/, _rgba);
}

