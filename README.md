platform-pdm-proto-wrapper
===
> A C++ library to wrap platform-pdm output in protobuf

### Acquiring

platform-pdm-proto-wrapper depends on the platform-pdm git submodule. So you should clone the repository with the following command:
```
git clone git@github.com:carbonengine/pdm-proto-wrapper.git --recurse-submodules
```

### Building

CMake and git should take care of downloading dependencies. So building is as trivial as this:
```
cmake -S . -B .build
cmake --build .build
```

### Using

#### with cmake

1. Place the source code somewhere in your project, f.e. a `vendor/pdm-protobuf` folder
2. Use `add_subdirectory(vendor/pdm-protobuf)` to add it to your project
3. Link this with your project: `target_link_libraries(mytarget PUBLIC platform_pdm_proto_wrapper)`

#### without cmake

1. add the `include/` directory to your compilers include directories
2. somewhere in your code do this:
```c++
#include <stringstream>
#include <pdm/protobuf.h>

void foo() {
    std::stringstream buffer;
    // `GetData` and `GetEVEPublicData` return data from the eve_public proto domain.
    // `GetEVELauncherData` can be used to return data from the deprecated eve_launcher proto domain.
    auto can_serialize = pdm_proto::GetData(&buffer);
    if (can_serialize) {
        // buffer now contains an instance of the protobuf encoded details.
        // Use `ParseFromIStream(&buffer)` to retrieve the actual protobuf structure
    }
}
```
3. Add `pdm_protobuf.lib` to your linker libraries

## 🤝 Contributing
Contribution follows the standard GIT PR model.

By submitting a pull request or otherwise contributing to this project, you agree to license your contribution under the MIT License, and you confirm that you have the right to do so.

## 📄 License and Legal Notices

© 2026 CCP Games 

This software is provided by CCP Games and does not include or distribute any third-party libraries or frameworks. 

This software is a PDM proto wrapper, a C++ library to wrap PDM output in protobuf.

Trademark Notice: CCP Games is a trademark of CCP ehf. 

This project is licensed under the [MIT License](LICENSE.md). Nothing in the [MIT License](LICENSE.md) grants any rights to CCP Games' trademarks or game content.
