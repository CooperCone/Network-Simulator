@echo off
setlocal

cd tools\unitTest

py -m venv env

call env\Scripts\activate.bat

pip install -r requirements.txt

cd %RootPath%
