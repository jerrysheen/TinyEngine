
// todo： 根据 系统 linux windows android
// 选择默认的API
#define D3D12_API
#define EDITOR

// ========== Windows API（新增） ==========
#ifdef _WIN32
#define NOMINMAX                // 禁用 Windows 的 min/max 宏，避免与 std::min/max 冲突
#define WIN32_LEAN_AND_MEAN     // 排除一些不常用的 Windows API，加快编译
#include <Windows.h>
#endif

#include "CoreAssert.h"
#include "iostream"
#include <unordered_map>
#include <map>
#include <utility>
#include <functional>
#include <string>
#include <variant>
#include <cmath> 
#include <algorithm>
#include "Core/Profiler.h"

using std::unordered_map;
using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::function;
