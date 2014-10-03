The proto file is the architecture of the messages sent by the plugin
Before being used, the proto file must be compiled, using :

cd ~/Documents/Xcode/SignalProcessor/Source 
protoc --cpp_out=. --java_out=. --python_out=. SignalMessages.proto

This generates the C++ files SignalMessages.pb.cc and SignalMessages.pb.h which are included in the Xcode project