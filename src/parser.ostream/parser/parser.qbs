import qbs

Project {
    name: "parser.ostream"
    minimumQbsVersion: "1.7.1"

    Product {
        name: "parser.ostream"

        files: [
            "Tree.ostream.h",
            "TypeTree.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "parser.data" }
            Depends { name: "nesting.ostream" }
        }
    }
}
