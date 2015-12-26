set DST_DIR=C:\경로를 입력\GitHub\CGSF\Common\SFPacketStore
set SRC_DIR=C:\경로를 입력\GitHub\CGSF\Common\SFPacketStore

protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%\LoginNetPacket.proto

ProtoGenCS %SRC_DIR%\LoginNetPacket.proto --proto_path=%DST_DIR%
pause