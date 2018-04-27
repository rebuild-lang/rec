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

            Depends { name: "filterParser" }
            Depends { name: "blockParser" }
            Depends { name: "expressionParser" }
        }
    }
}
