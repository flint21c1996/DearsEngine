// Match DemoScene.h: IScene brings in Windows before the editor panel.
#include "IScene.h"
#if !defined(min) || !defined(max)
#error This regression check requires the engine's Windows min/max macros
#endif
#include "AStarDemoPanel.h"
