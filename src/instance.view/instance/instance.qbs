import qbs

Project {
    name: "instance.view"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "instance.view"

        files: [
            "Views.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]
        }
    }
}
