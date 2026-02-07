# shared_resources
A header-only library in C++ to conveniently handle shared resources.

## Requirements
- C++20

## Building

### Installing
Example commands to install the library:
```sh
git clone https://github.com/sing-kuro/shared-resources.git
cd shared-resources
mkdir build
cmake -S . -B build -DBUILD_SHARED_RESOURCES_DOCS=ON -DENABLE_TESTING_SHARED_RESOURCES=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
cmake --install build --prefix "/your/install/dir"
```

### Linking
In your project's `CMakeLists.txt`, add:
```cmake
find_package(shared_resources REQUIRED)
target_link_libraries(YourApp PRIVATE shared_resources::shared_resources)
```

If you have installed the library on your machine:
```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="/your/install/dir"
```

If you want to avoid installing the library locally:
```sh
cmake -S . -B build -Dshared_resources_DIR=/path/to/shared_resources/build
```

If you prefer to use CMake's `FetchContent`:
```cmake
include(FetchContent)

FetchContent_Declare(
    shared_resources
    GIT_REPOSITORY https://github.com/sing-kuro/shared-resources.git
    GIT_TAG master # The commit hash of your choice
)

FetchContent_MakeAvailable(shared_resources)

add_executable(YourApp main.cpp)
target_link_libraries(YourApp PUBLIC shared_resources)
```

## Usage

Include the header and use the `srs` namespace.

```cpp
#include <shared_resources/shared_resources.hpp>

using namespace srs;
```

### Defining resource types

Specify which types you want to share with a type_list (no duplicate types allowed):

```cpp
using MyResources = type_list<Config, Logger, Database>;
```

### shared_resources — owning shared instances

`shared_resources<List, Exclude...>` stores one instance of each resource type. Construct it by passing one object per type (order does not matter):

```cpp
Config config;
Logger logger;
Database db;

shared_resources<MyResources> resources(config, logger, db);

// Access by type
Config& c = resources.get<Config>();
Logger& l = resources.get<Logger>();
resources.get<Database>().connect();
```

You can exclude some types from the list so that a type only appears in the list for specification; the excluded types are not stored. This is convenient when you want to pass the `shared_resources` object to a constructor of one of the types listed in `AllResources`:
```cpp
using AllResources = type_list<Config, Logger, Database>;
using ResourcesWithoutDb = shared_resources<AllResources, Database>;

ResourcesWithoutDb sub(config, logger);
// sub.get<Database>();  // ill-formed: Database is excluded
sub.get<Config>();
sub.get<Logger>();
```

You can create a new `shared_resources` object with new types added to the original:

```cpp
shared_resources<MyResources, Logger, Database> base(config);
// The order of logger and db does not matter.
// The listed types (MyResources) does not need to be the same.
shared_resources<MyResources> extended(base, logger, db); 
extended.get<Logger>();  // logger
extended.get<Config>();  // same as base.get<Config>()
```

Copy and move constructors work as usual; you can also construct from another `shared_resources` that has the same effective type list.

### shared_references — non-owning references

`shared_references<List, Exclude...>` holds `std::reference_wrapper`s to existing objects. Use it when you want to pass around a set of references without owning the resources:

```cpp
shared_references<MyResources> refs(config, logger, db);

refs.get<Config>().set("key", "value");  // modifies the original config
refs.get<Logger>().info("message");
```

Construction takes lvalue references. You can also construct from another `shared_references` and additional references to extend the set.

### Summary

| Feature | shared_resources | shared_references |
|--------|-------------------|-------------------|
| Storage | Owns instances | Holds references |
| Construction | Values (e.g. `Args...`) | Lvalue refs (e.g. `Args&...`) |
| get\<T\>() | Reference to stored T | Reference to referred-to T |
| Exclude | Optional Exclude... | Optional Exclude... |

All types live in the header `shared_resources.hpp`; no extra source files are required.

## Issue Report
If you find any issues on this project, please [report it on GitHub](https://github.com/sing-kuro/shared-resources/issues).

## Acknowledgements
- This project utilizes OSS (Open Source Software)
    See [NOTICE.md](NOTICE.md)
