#include <WinHttp.au3>


func wyslij($sFileToUpload = @ScriptFullPath)
 ;<- Set correct file location here (current choice uploads this script)
$sAddress = "http://192.168.1.101/update" ;<- Set correct address here.
; Form to fill (page doesn't have transparent form)
Local $sForm = _
        '<form action="' & $sAddress &'" method="post" enctype="multipart/form-data">' & _
        '    <input type="file" name="update"/>' & _ ;
        '<input type="submit" value="Wy�lij plik"/></form>'
  ; <input type="submit" value="Wy�lij plik"/>


; Initialize and get session handle
$hOpen = _WinHttpOpen()
$hConnect = $sForm ; will pass form as string so this is for coding correctness because $hConnect goes in byref

; Fill form
$sHTML = _WinHttpSimpleFormFill($hConnect, $hOpen, _
        Default, _
        "name:update", $sFileToUpload)

If @error Then
    ConsoleWrite("Error number = " & @error)
Else
    ConsoleWrite("Result: " & @CRLF & $sHTML)
EndIf

; Close handles
_WinHttpCloseHandle($hConnect)
_WinHttpCloseHandle($hOpen)

EndFunc

while 1
	$nazwa_pliku = "lamp1.ino.nodemcu.bin"
if FileExists($nazwa_pliku) Then
	ConsoleWrite("znalaz�em plik, wysy�am")
	wyslij($nazwa_pliku)
	FileDelete($nazwa_pliku)
EndIf
Sleep(100)
WEnd