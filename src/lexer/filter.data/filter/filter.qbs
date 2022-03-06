import qbs

Project {
    name: "filter.data"

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
