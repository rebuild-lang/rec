import qbs

Project {
    name: "nesting.ostream"
    minimumQbsVersion: "1.7.1"

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
