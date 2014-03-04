set DST_DIR=D:\Dev\CGSFFinal\Common\SFPacketStore
set SRC_DIR=D:\Dev\CGSFFinal\Common\SFPacketStore
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/SFPacketStore.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/ChatPacket.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/PacketCore.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/SevenGamePacket.proto
pause