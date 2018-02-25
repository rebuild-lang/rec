import qbs

Project {
    minimumQbsVersion: "1.7.1"

    references: [
        "filter",
        "block",
        "expression",
    ]

    StaticLibrary {
        name: "parser"

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "filter" }
            Depends { name: "block" }
            Depends { name: "expression" }
        }
    }
}
