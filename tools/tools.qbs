import qbs

Project {
    minimumQbsVersion: "1.7.1"

    Product {
        name: "cpp17"

        Export {
            Depends { name: "cpp" }
            cpp.cxxLanguageVersion: "c++17"
            cpp.cxxFlags: {
                if (qbs.toolchain.contains('msvc')) return ["/await", "/permissive-", "/Zc:__cplusplus"];
                if (qbs.toolchain.contains('clang')) return ["-fcoroutines-ts"];
            }
            cpp.cxxStandardLibrary: {
                if (qbs.toolchain.contains('clang')) return "libc++";
            }
            cpp.staticLibraries: {
                if (qbs.toolchain.contains('clang')) return ["c++", "c++abi"];
            }
        }
    }

    references: [
        "meta",
        "strings",
    ]

    Product {
        name: "tools"
        Export {
            Depends { name: "meta" }
            Depends { name: "strings" }
        }
    }
}
