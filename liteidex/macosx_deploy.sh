#!/bin/sh

cp -v ../README.md liteide/bin
cp -v LICENSE.LGPL liteide/bin
cp -v LGPL_EXCEPTION.TXT liteide/bin

cp -r -v deploy/* liteide/bin/LiteIDE.app/Contents/Resources
cp -r -v os_deploy/macosx/* liteide/bin/LiteIDE.app/Contents/Resources
cp -R -v src/liteide/Info.plist liteide/bin/LiteIDE.app/Contents

cp -v bin/gotools liteide/bin/LiteIDE.app/Contents/MacOS
cp -v bin/gocode liteide/bin/LiteIDE.app/Contents/MacOS
cp -v bin/gomodifytags liteide/bin/LiteIDE.app/Contents/MacOS
