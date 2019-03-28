#include <WinHttp.au3>


func wyslij($sFileToUpload = @ScriptFullPath)
 ;<- Set correct file location here (current choice uploads this script)
$sAddress = "http://192.168.43.161/update" ;<- Set correct address here.
; Form to fill (page doesn't have transparent form)
Local $sForm = _
        '<form action="' & $sAddress &'" method="post" enctype="multipart/form-data">' & _
        '    <input type="file" name="update"/>' & _ ;
        '<input type="submit" value="Wyœlij plik"/></form>'
  ; <input type="submit" value="Wyœlij plik"/>


; Initialize and get session handle
$hOpen = _WinHttpOpen()
$hConnect = $sForm ; will pass form as string so this is for coding correctness because $hConnect goes in byref

; Fill form
$sHTML = _WinHttpSimpleFormFill($hConnect, $hOpen, _
        Default, _
        "name:update", $sFileToUpload)

;~ If @error Then
;~     MsgBox(4096, "Error", "Error number = " & @error)
;~ Else
;~     MsgBox(64, "Success", "Result: " & @CRLF & $sHTML)
;~ EndIf

; Close handles
_WinHttpCloseHandle($hConnect)
_WinHttpCloseHandle($hOpen)

EndFunc

while 1
	$nazwa_pliku = "mqtt.ino.nodemcu.bin"
if FileExists($nazwa_pliku) Then
	ConsoleWrite("znalaz³em plik, wysy³am")
	wyslij($nazwa_pliku)
	FileDelete($nazwa_pliku)
EndIf
Sleep(100)
WEnd