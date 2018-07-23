import qbs

Project {
    name: "filter.ostream"
    minimumQbsVersion: "1.7.1"

    Product {
        name: "filter.ostream"

        Depends { name: "filter.data" }
        Depends { name: "scanner.ostream" }

        files: [
            "Token.ostream.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "filter.data" }
            Depends { name: "scanner.ostream" }
        }
    }
}
