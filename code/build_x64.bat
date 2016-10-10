@call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"
@IF "%1"=="" GOTO FULL_COMPILE
	@python gml_builder.py -platform:win64 -out:../Bin/ -plugin:%1
	@GOTO EXIT
:FULL_COMPILE
	@python gml_builder.py -platform:win64 -out:../Bin/ -full
	@GOTO EXIT	
:EXIT
