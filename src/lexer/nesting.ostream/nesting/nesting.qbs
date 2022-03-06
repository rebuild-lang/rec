import qbs

Project {
    name: "nesting.ostream"

    Product {
        name: "nesting.ostream"

        Depends { name: "nesting.data" }
        Depends { name: "filter.ostream" }

        files: [
            "Token.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "nesting.data" }
            Depends { name: "filter.ostream" }
        }
    }
}
