set MODNAME=QtLZip
set EXPNAME=lziptool
nmake distclean
rd  /s /q lib
rd  /s /q mkspecs
cd src/%MODNAME%
rd  /s /q .moc
rd  /s /q .obj
rd  /s /q .pch
del /s /q *_resource.rc
cd ../../examples/%EXPNAME%/
rd  /s /q .moc
rd  /s /q .obj
rd  /s /q .pch
rd  /s /q debug
rd  /s /q release
cd ../../
