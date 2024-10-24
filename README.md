# Genshin Impact FPS unlocker

一个用于解锁修改原神帧率上限的工具。  
A gui tool for modify the Genshin Impact game FPS upper limitation.

> 原神支持版本 Surpport Version ***5.1***

项目处于测试阶段 GUI功能尚未完善，正在更新。
### 说明 Statement  

+ 核心代码来源 [Github@winTEuser](github.com/winTEuser)  
  我只是加了一层UI框架，没什么技术性而言。写入内存需要的shellcode及其补丁注入思路都是从原项目来的。
 
+ 原项目：[winTEuser的原神&崩铁 FPS解锁工具](https://github.com/winTEuser/Genshin_StarRail_fps_unlocker)  

### 开发 Dev-Env

+ 需要的编译器： `用于VS2022 Community的 MSVC 17.11.4`
  
+ 库与框架： `Qt 6.8.0社区版(VS2022)` 与 `Win32API`

+ 生成和构建： `CMake` 与 `Ninja`