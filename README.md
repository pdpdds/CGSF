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

In ThirdParty Folder
* Open ThirdParty.sln and Compile
* Compile ACE Project
* Compile CEGUI(CGSF/ThirdParty/CEGUI/projects/premake/CEGUI2012.sln)
* Compile glog(CGSF/ThirdParty/glog-0.3.2/google-glog.sln) and copy libglog.lib, libglog.dll to lib/debug(or release), bin32
* Compile GoogleProtocolBuffer(protobuf-2.4.1) and copy (libprotobuf.lib, libprotobuf-lite.lib, libprotoc.lib) to lib/debug(or release)
  (this project's compile is not perfect)
  
In VSProject
* Open AllProject.sln and Build it

Open Source
-------
* [ACE Library](http://download.dre.vanderbilt.edu/)
* [Google Protocol Buffer] (http://code.google.com/p/protobuf/)