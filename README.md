# 便携式 AvZ 注入器

AvZ 自带 injector 的修改版，使得无需 AvZ 套件便可向用户分发 AvZ 脚本，可以为分发基于 AvZ 的辅助工具提供一定便利。

该注入器会自动查找同一目录下同名的 `.dll` 文件，并注入到 PvZ 中。

## 使用方法

1. 以正常方式运行一次脚本，找到 `AsmVsZombies\bin\libavz_inject.dll` ，复制到某一目录下，重命名为 `[你的应用名].dll`

2. 将本目录下的 `bin\injector.exe` 复制到与 1. 相同的目录下，重命名为 `[你的应用名].exe`

3. 将这两个文件打包分发，直接运行 `.exe` 即可注入脚本

## 编译方法

安装 MSVC C++ 套件，在本目录下 `cl *.cpp /O2`。

`bin` 目录下有预编译的二进制文件。

## 许可

Copyright © 2020-2022  vector-wlc, Reisen

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
