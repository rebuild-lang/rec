import qbs

Project {
    name: "parser.ostream"

    Product {
        name: "parser.ostream"

        Depends { name: "parser.data" }
        Depends { name: "nesting.ostream" }
        Depends { name: "instance.data" }

        files: [
            "Expression.ostream.h",
            "Type.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "parser.data" }
            Depends { name: "nesting.ostream" }
            Depends { name: "instance.data" }
        }
    }
}
