The proto file is the architecture of the messages sent by the plugin
Before being used, the proto file must be compiled, using :

cd ~/Documents/Xcode/SignalProcessor/Source 
protoc --cpp_out=. --java_out=. --python_out=. SignalMessages.proto

This generates the C++ files SignalMessages.pb.cc and SignalMessages.pb.h which are included in the Xcode project

If you want to use the binary data output by SignalProcessor in other custom projects, you can just take the SignalMessages files and include them in your projects - Java, C++ and Python are currently supported