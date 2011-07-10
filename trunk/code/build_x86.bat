	@IF "%1"=="" GOTO HELP
	@call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
	@python gml_builder.py win32 %*
	@GOTO EXIT
:HELP
	@echo "Use: %0 <output-folder>"
:EXIT
