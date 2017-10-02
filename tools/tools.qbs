import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "tools"
        Depends { name: "cpp" }
        cpp.cxxLanguageVersion: "c++17"
        cpp.includePaths: ["."]

        files: [
            "meta/algorithm.h",
            "meta/optional.h",
            "meta/overloaded.h",
            "meta/variant.h",
            "strings/code_point.h",
            "strings/rope.h",
            "strings/utf8_string.h",
            "strings/utf8_view.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.cxxLanguageVersion: "c++17"
            cpp.includePaths: ["."]
        }
    }
}
