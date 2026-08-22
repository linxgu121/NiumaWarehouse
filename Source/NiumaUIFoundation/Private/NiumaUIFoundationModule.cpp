#include "Modules/ModuleManager.h"

/**
 * Niuma 全局 UI 基础模块。
 *
 * 只提供 Theme、原子组件和通用 UI 模板，
 * 不包含任何仓库或玩法业务。
 * 
 * 注册这个 C++ 模块，让引擎能识别、加载
 */
IMPLEMENT_MODULE(FDefaultModuleImpl,NiumaUIFoundation)