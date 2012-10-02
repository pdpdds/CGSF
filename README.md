Casual Game Server Framework 
=============

This project purpose rapid development of online casual game server using open source code. 

Features
-------

Documentation
-------

Build Instruction
-------
To build project, we need visual studio 2012
* Uncompress CGSF/External/ThirdParty.zip and copy it to ../../

In ThirdParty Folder
* Open ThirdParty.sln and Compile
* Compile ACE Project
* Compile GoogleProtocolBuffer(protobuf-2.4.1) and copy (libprotobuf.lib, libprotobuf-lite.lib, libprotoc.lib) to lib/debug(or release)
  (this project's compile is not perfect. we need protobuf's 2012 version)
  
 In CGSF Folder
* Open OnlineSystem.sln and Build it

Open Source
-------
* [ACE Library](http://download.dre.vanderbilt.edu/)
* [Google Protocol Buffer] (http://code.google.com/p/protobuf/)