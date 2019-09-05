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

    Product {
        name: "parser.builder"

        Depends { name: "parser.ostream" }
        Depends { name: "instance.data" }

        files: [
            "Tree.builder.h",
        ]

        Export {
            Depends { name: "parser.data" }
            Depends { name: "instance.data" }
        }
    }
}
