import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "tools"
        Depends { name: "cpp" }
        cpp.cxxLanguageVersion: "c++17"
        cpp.includePaths: ["."]
        cpp.cxxFlags: {
            if (qbs.toolchain.contains('msvc')) return "/await";
            if (qbs.toolchain.contains('clang')) return ["-fcoroutines-ts"];
        }
        cpp.cxxStandardLibrary: {
            if (qbs.toolchain.contains('clang')) return "libc++";
        }

        files: [
            "meta/algorithm.h",
            "meta/co_enumerator.h",
            "meta/flags.h",
            "meta/optional.h",
            "meta/overloaded.h",
            "meta/type_list.h",
            "meta/value_list.h",
            "meta/variant.h",
            "meta/vector_range.h",
            "strings/code_point.cpp",
            "strings/code_point.h",
            "strings/join.h",
            "strings/rope.h",
            "strings/utf8_string.h",
            "strings/utf8_view.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.cxxLanguageVersion: "c++17"
            cpp.includePaths: ["."]
            cpp.cxxFlags: {
                if (qbs.toolchain.contains('msvc')) return "/await";
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
}
