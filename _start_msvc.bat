@if exist "%~dp0\.set_sln.bat" @call "%~dp0\.set_sln.bat"
@call "%~dp0\.bat\_start_msvc_impl.bat" %SLN%
