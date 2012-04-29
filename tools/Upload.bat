@echo on
set ADDON_PATH=%cd%\Sora
set PUBLISH_GIT_PATH=%cd%
set TEMP_PATH=%cd%\temp
set git=cmd /u /c git --git-dir=%PUBLISH_GIT_PATH%\.git --work-tree=%TEMP_PATH%

if exist %TEMP_PATH% (
	rd /s /q %TEMP_PATH%
	if not %errorlevel%==0 goto err
)

md %TEMP_PATH%
if not %errorlevel%==0 goto err

%git% rm %TEMP_PATH%
rem if not %errorlevel%==0 goto err

FileListGenerator %ADDON_PATH% %TEMP_PATH%
if not %errorlevel%==0 goto err

echo "Add files to git repository."
%git% add %TEMP_PATH%

echo "Commiting..."
%git% commit -m "Update addons."
if not %errorlevel%==0 goto err

echo "Pushing to server..."
%git% push -u origin master
if not %errorlevel%==0 goto err

echo "Cleaning..."
if exist %TEMP_PATH% rd /s /q %TEMP_PATH%
if not %errorlevel%==0 goto err

echo "Done."

goto exit

:err
echo "Something goes wrong..."

:exit
pause
