import qbs

Project {
    name: "filter.data"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "filter.data"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "scanner.data" }

        files: [
            "Token.cpp",
            "Token.h",
            "Token.builder.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "scanner.data" }
        }
    }
}
