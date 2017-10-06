::删除库调试文件
del widgets.sdf
del Debug\*.ilk
del Debug\*.pdb
del Release\*.pdb
del libcommon\libcommon.vcxproj.user
rd /s /q libcommon\Debug
rd /s /q libcommon\Release
::删除其他工程调试文件
rd /s /q autoincrement\Debug
rd /s /q autoincrement\Release
rd /s /q config\Debug
rd /s /q config\Release
rd /s /q crypto\Debug
rd /s /q crypto\Release
rd /s /q data2lua\Debug
rd /s /q data2lua\Release
rd /s /q md5\Debug
rd /s /q md5\Release
rd /s /q remote\Debug
rd /s /q remote\Release
rd /s /q traverse\Debug
rd /s /q traverse\Release
rd /s /q write\Debug
rd /s /q write\Release