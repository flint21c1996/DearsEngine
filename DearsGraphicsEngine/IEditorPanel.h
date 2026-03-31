#pragma once

class IEditorPanel
{
public:
	virtual ~IEditorPanel() = default;
	virtual void Draw() = 0;
	virtual const char* GetName() const = 0;
};
