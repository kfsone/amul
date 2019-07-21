cd libfuncs
autodoc >agllib:AGL.Lib.Doc -a -s -w -I LibTable.I TextPtr.I CommsPrep.I CreateAPort.I Random.I Screen.I
cd /
libtool -lpso proto/AGL.h AGL.fd
;copy libraries/*.i arg:libraries/
;copy libraries/*.h arg:libraries/
;copy proto/*.h include:proto
;copy *.i arg:
;copy *.h include:

