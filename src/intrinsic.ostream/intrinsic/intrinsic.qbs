import qbs

Project {
    name: "intrinsic.ostream"

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
