set DST_DIR=D:\CGSF20132\trunk\Common\SFPacketStore
set SRC_DIR=D:\CGSF20132\trunk\Common\SFPacketStore
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/PacketCore.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/SFPacketStore.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/ProtocolPacket.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/SevenGamePacket.proto
protoc -I=%SRC_DIR% --cpp_out=%DST_DIR% %SRC_DIR%/FPSPacket.proto
pause