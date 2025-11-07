@echo off
echo === Running Bison and Flex ===
"C:\Users\user\source\repos\knu\SP\Lab3\win_flex_bison-2.5.25\win_bison.exe" -d parser.y -o parser.cpp
if %errorlevel% neq 0 exit /b %errorlevel%
"C:\Users\user\source\repos\knu\SP\Lab3\win_flex_bison-2.5.25\win_flex.exe" -o lexer.cpp lexer.l
if %errorlevel% neq 0 exit /b %errorlevel%
echo === Done ===
exit /b 0
