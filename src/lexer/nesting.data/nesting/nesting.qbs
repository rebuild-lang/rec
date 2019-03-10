import qbs

Project {
    name: "nesting.data"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "nesting.data"
        Depends { name: "cpp" }

        Depends { name: "filter.data" }

        files: [
            "Token.cpp",
            "Token.h",
            "Token.builder.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "filter.data" }
        }
    }
}
