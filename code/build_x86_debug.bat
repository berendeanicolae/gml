@call "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
@IF "%1"=="" GOTO FULL_COMPILE
	@python gml_builder_debug.py -platform:win32 -out:../Bin/ -plugin:%1
	@GOTO EXIT
:FULL_COMPILE
	@python gml_builder_debug.py -platform:win32 -out:../Bin/ -full
	@GOTO EXIT	
:EXIT
