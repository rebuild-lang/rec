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
            "meta/CoEnumerator.h",
            "meta/Flags.h",
            "meta/Optional.h",
            "meta/Overloaded.h",
            "meta/TypeList.h",
            "meta/ValueList.h",
            "meta/Variant.h",
            "meta/VectorRange.h",
            "meta/algorithm.h",
            "strings/CodePoint.h",
            "strings/Output.h",
            "strings/Rope.h",
            "strings/String.h",
            "strings/View.h",
            "strings/join.h",
            "strings/stringsCodePoint.cpp",
            "strings/stringsOutput.cpp",
            "strings/stringsRope.cpp",
            "strings/stringsString.cpp",
            "strings/stringsView.cpp",
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
