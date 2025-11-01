#include "PreCompiledHeader.h"
#include "EditorSettings.h"

/*
 * TinyEngine 编辑器窗口布局设计 (1920 x 1080)
 * 
 * ┌────────────────────────────────────────────────────────────────────────┐
 * │                        EditorSettings::mainBar (1920 x 50)             │  ← 菜单栏：File, Edit, Assets 等
 * │                        [File] [Edit] [Assets]                          │
 * ├──────────────┬──────────────────────────────────────┬─────────────────┤
 * │              │                                      │                 │
 * │ EditorSettings│     EditorSettings::gameView        │ EditorSettings  │
 * │ ::hierarchy  │         (1120 x 630)                 │ ::inspector     │
 * │ (250 x 630)  │     [16:9 游戏视口区域]              │ (550 x 1030)    │
 * │              │                                      │                 │
 * │ - Scene      │      显示游戏画面                     │  显示选中对象    │
 * │   - Heart    │      包括FPS等信息                    │  的详细属性      │
 * │   - Camera   │      保持16:9比例                     │                 │
 * │   - Sun      │                                      │  [Inspector]    │
 * │   - Floor    │                                      │  - Transform    │
 * │   - Spheres  │                                      │  - Components   │
 * │              │                                      │  - ...          │
 * ├──────────────┴───────────────────────┬─────────────────┤                 │
 * │                                    │                 │                 │
 * │  EditorSettings::project           │ EditorSettings  │                 │
 * │      (856.25 x 400)                │ ::console       │                 │
 * │                                    │ (513.75 x 400)  │                 │
 * │ [Audio]                            │                 │                 │
 * │ [Materials]                        │ 显示日志和调试  │                 │
 * │ [Models]                           │ 信息：          │                 │
 * │ [Prefabs]                          │ - Load asset... │                 │
 * │ [Textures]                         │ - Log Warning   │                 │
 * └────────────────────────────────────┴─────────────────┴─────────────────┘
 * 
 * 尺寸分配说明（基于1920x1080窗口）：
 * 
 * 水平方向 (1920):
 *   - EditorSettings::hierarchy (左上):     250px    (~13.0%)  ← 固定宽度
 *   - EditorSettings::gameView  (中上):     1120px   (~58.3%)  ← 16:9比例 (630*16/9)
 *   - EditorSettings::inspector (右侧):     550px    (~28.6%)  ← 更宽的属性面板
 *   - EditorSettings::project   (左下):     856.25px (~44.6%)  ← 占底部5/8
 *   - EditorSettings::console   (中下):     513.75px (~26.8%)  ← 占底部3/8
 * 
 * 垂直方向 (1080):
 *   - EditorSettings::mainBar             (顶部): 50px   (~4.6%)
 *   - EditorSettings::hierarchy/gameView  (中上): 630px  (~58.3%)
 *   - EditorSettings::project/console     (中下): 400px  (~37.0%)
 *   - EditorSettings::inspector 占据整个右侧:     1030px (上下两层合计)
 * 
 * 面板位置坐标：
 *   EditorSettings::mainBar:    (0,       0)   - (1920,    50)
 *   EditorSettings::hierarchy:  (0,       50)  - (250,     680)
 *   EditorSettings::gameView:   (250,     50)  - (1370,    680)   ← 16:9比例视口
 *   EditorSettings::inspector:  (1370,    50)  - (1920,    1080)
 *   EditorSettings::project:    (0,       680) - (856.25,  1080)  ← 占底部5/8
 *   EditorSettings::console:    (856.25,  680) - (1370,    1080)  ← 占底部3/8
 * 
 * 比例配置（用于自适应不同窗口大小）：
 *   EditorSettings::mainBarHeightRatio       = 50.0    / 1080.0  ≈ 0.0463
 *   EditorSettings::hierarchyWidthRatio      = 250.0   / 1920.0  ≈ 0.1302
 *   EditorSettings::gameViewWidthRatio       = 1120.0  / 1920.0  ≈ 0.5833  (16:9比例)
 *   EditorSettings::inspectorWidthRatio      = 550.0   / 1920.0  ≈ 0.2865
 *   EditorSettings::projectWidthRatio        = 856.25  / 1920.0  ≈ 0.4460  (底部5/8)
 *   EditorSettings::consoleWidthRatio        = 513.75  / 1920.0  ≈ 0.2676  (底部3/8)
 *   EditorSettings::gameViewHeightRatio      = 630.0   / 1030.0  ≈ 0.6117
 * 
 * 设计要点：
 *   ✓ EditorSettings::gameView (1120x630) 保持16:9黄金比例，不变形
 *   ✓ EditorSettings::inspector (550px) 更宽，可以更舒适地显示组件属性
 *   ✓ EditorSettings::project 占据底部更多空间 (5:3)，方便浏览资源
 *   ✓ EditorSettings::console 获得更合理的空间，可以更好地显示日志信息
 *   ✓ 整体布局更加平衡，视口比例正确
 */

namespace EngineEditor
{
    // Hierarchy 面板 (左上) - 250x630
    Vector2  EditorSettings::hierarchyStartPos = Vector2{0, 50.0f/1080.0f};
    Vector2  EditorSettings::hierarchySize = Vector2{250.0f/1920.0f, 630.0f/1080.0f};
    
    // Console 面板 (中下) - 513.75x400，占底部3/8
    Vector2  EditorSettings::consoleStartPos = Vector2{856.25f/1920.0f, 680.0f/1080.0f};
    Vector2  EditorSettings::consoleSize = Vector2{513.75f/1920.0f, 400.0f/1080.0f};
    
    // Project 面板 (左下) - 856.25x400，占底部5/8
    Vector2  EditorSettings::projectStartPos = Vector2{0, 680.0f/1080.0f};
    Vector2  EditorSettings::projectSize = Vector2{856.25f/1920.0f, 400.0f/1080.0f};
    
    // Inspector 面板 (右侧) - 550x1030
    Vector2  EditorSettings::inspectorStartPos = Vector2{1370.0f/1920.0f, 50.0f/1080.0f};
    Vector2  EditorSettings::inspectorSize = Vector2{550.0f/1920.0f, 1030.0f/1080.0f};
    
    // MainBar 面板 (顶部) - 1920x50
    Vector2  EditorSettings::mainBarStartPos = Vector2{0, 0};
    Vector2  EditorSettings::mainBarSize = Vector2{1920.0f/1920.0f, 50.0f/1080.0f};
    
    // GameView 面板 (中上) - 1120x630，保持16:9比例
    Vector2  EditorSettings::gameViewStartPos = Vector2{250.0f/1920.0f, 50.0f/1080.0f};
    Vector2  EditorSettings::gameViewSize = Vector2{1120.0f/1920.0f, 630.0f/1080.0f};


    Vector2  EditorSettings::currentWindowSize = Vector2{1920.0f, 1080.0f};

}