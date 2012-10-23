set DST_DIR=D:\dev\cgsf\trunk\Common\SFPacketStore
set SRC_DIR=D:\dev\cgsf\trunk\Common\SFPacketStore
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/SFPacketStore.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/ChatPacket.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/PacketCore.proto
pause