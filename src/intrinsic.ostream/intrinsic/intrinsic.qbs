import qbs

Project {
    name: "intrinsic.ostream"
    minimumQbsVersion: "1.7.1"

    Product {
        name: "intrinsic.ostream"

        Depends { name: "intrinsic.data" }

        files: [
            "ModuleOutput.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "intrinsic.data" }
        }
    }
}
