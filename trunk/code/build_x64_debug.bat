@call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\amd64\vcvars64.bat"
@IF "%1"=="" GOTO FULL_COMPILE
	@C:\Python27\python.exe gml_builder_debug.py -platform:win64 -out:../Bin/ -plugin:%1
	@GOTO EXIT
:FULL_COMPILE
	@C:\Python27\python.exe gml_builder_debug.py -platform:win64 -out:../Bin/ -full
	@GOTO EXIT	
:EXIT
