import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "strings"
        Depends { name: "cpp" }
        cpp.combineCxxSources: true
        cpp.includePaths: [".."]

        Depends { name: "meta" }

        files: [
            "CodePoint.cpp",
            "CodePoint.h",
            "Output.cpp",
            "Output.h",
            "Rope.cpp",
            "Rope.h",
            "String.cpp",
            "String.h",
            "View.cpp",
            "View.h",
            "join.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "meta" }
        }
    }
}
