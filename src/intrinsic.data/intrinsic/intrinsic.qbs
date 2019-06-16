import qbs

Project {
    name: "intrinsic.data"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "intrinsic.data"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "parser.data" }

        files: [
            "Function.cpp",
            "Function.h",
            "Module.h",
            "Parameter.h",
            "Type.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "parser.data" }
        }
    }
}
