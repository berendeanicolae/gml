	@IF "%1"=="" GOTO HELP
	@call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\amd64\vcvars64.bat"
	@python gml_builder.py win64 %*
	@GOTO EXIT
:HELP
	@echo "Use: %0 <output-folder>"
:EXIT
