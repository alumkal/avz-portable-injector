# 便携式 AvZ 注入器

AvZ 自带 injector 的修改版，使得无需 AvZ 套件便可向用户分发 AvZ 脚本，可以为发布基于 AvZ 的辅助工具提供一定便利。

## 使用方法

注入器：

有以下三种使用方式：

- 在文件资源管理器内将 DLL 文件拖动到注入器 `.exe` 文件上
- 设置 DLL 的打开方式为注入器（不推荐）
- 直接运行注入器，在弹出的文件选择窗口中选择 DLL

一键打包：

运行该程序，选择待打包的 DLL，程序会在 DLL 所在目录自动生成同名 `.exe`。该 `.exe` 可以不依赖其他文件独立运行。

以上两个程序都支持以命令行形式传入文件名。

注入完成后注入器会自动关闭，这并非错误。

## 构建

注入器：`clang++ -std=c++20 -m32 -O2 main.cpp -o injector.exe`

一键打包：`clang++ -std=c++20 -O2 assets.res main.cpp -o 一键打包.exe`

`bin/` 下有预编译的二进制文件。

## 许可

Copyright © 2022-2023 vector-wlc, Reisen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
