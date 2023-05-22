@set ACC=al-martyn1
@set BASE=github.com


@if "%1"=="SSH"  goto SETUP_SSH
@if "%1"=="GIT"  goto SETUP_SSH
@if "%1"=="HTTP" goto SETUP_HTTP



@rem Default is HTTPS mode

:SETUP_HTTPS
@rem По https 
set PREFIX=https://%BASE%/%ACC%
goto DO_CLONE

:SETUP_HTTP
@rem По https 
set PREFIX=http://%BASE%/%ACC%
goto DO_CLONE

:SETUP_SSH
set PREFIX=git@%BASE%:%ACC%

:DO_CLONE
git clone %PREFIX%/umba_mm_mod_encodings.git   %~dp0\encoding
git clone %PREFIX%/marty_cpp.git          %~dp0\marty_cpp
git clone %PREFIX%/marty_tr.git           %~dp0\marty_tr
git clone %PREFIX%/marty_yaml_toml_json.git    %~dp0\marty_yaml_toml_json
git clone %PREFIX%/umba_mm_mod_sfmt.git   %~dp0\sfmt
git clone %PREFIX%/umba_mm_mod_umba.git   %~dp0\umba
