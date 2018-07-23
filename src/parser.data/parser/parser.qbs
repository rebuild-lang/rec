import qbs

Project {
    name: "parser.data"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "parser.data"

        Depends { name: "nesting.data" }
        Depends { name: "instance.view" }

        files: [
            "Tree.cpp",
            "Tree.h",
            "Tree.builder.h",
            "TypeTree.cpp",
            "TypeTree.h",
            "Value.cpp",
            "Value.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "nesting.data" }
            Depends { name: "instance.view" }
        }
    }
}
