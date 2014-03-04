Casual Game Server Framework 
=============

This project purpose rapid development of online casual game server using open source code. 

Features
-------

Documentation
-------

Build Instruction
-------
To build project, we need visual studio 2013

In ThirdParty Folder
* Open ThirdParty.sln and Compile debug and release both.
* We must build external libraries.
  ACE 6.24
  CEGUI 0.8.3
  GLog 0.3.3
  Google BreakPad
  Google Protocol Buffer 2.5.0
  Directx SDK June 2010
  
  in vs 2013 some libraries occurs error, because std::min, std::max function
  in error position, add following headers => #include <algorithm>
  
* Additional Include Path
  $(AdditionalLibrary)\boost_1_55_0;
  $(AdditionalLibrary)\ACE-6.2.4\ACE_wrappers;
  $(AdditionalLibrary)\protobuf-2.5.0\src;
  $(AdditionalLibrary)\glog-0.3.3\src\windows;
  $(AdditionalLibrary)\google-breakpad\src\client\windows\handler;
  $(AdditionalLibrary)\google-breakpad\src;
 
* Additional Library Debug Path
  $(AdditionalLibrary)\ACE-6.2.4\ACE_wrappers\lib;
  $(AdditionalLibrary)\boost_1_55_0\lib32-msvc-12.0;
  $(AdditionalLibrary)\protobuf-2.5.0\vsprojects\Debug;
  $(AdditionalLibrary)\glog-0.3.3\Debug;
  $(AdditionalLibrary)\google-breakpad\src\client\windows\Debug\lib; 
  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\lib\x86;  
 
* Additional Library Release Path
$(AdditionalLibrary)\ACE-6.2.4\ACE_wrappers\lib;
  $(AdditionalLibrary)\boost_1_55_0\lib32-msvc-12.0;
  $(AdditionalLibrary)\protobuf-2.5.0\vsprojects\Release;
  $(AdditionalLibrary)\glog-0.3.3\Release;
  $(AdditionalLibrary)\google-breakpad\src\client\windows\Release\lib; 
  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\lib\x86;   
  
Finally, In VSProject Folder
* Open AllProject.sln and Build it

Open Source
-------
* [ACE Library](http://download.dre.vanderbilt.edu/)
* [Google Protocol Buffer] (http://code.google.com/p/protobuf/)
* [Google Log] (https://code.google.com/p/google-glog/)
* [CEGUI] (http://cegui.org.uk/)
* [Google BreakPad] (https://code.google.com/p/google-breakpad/)
