import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "api"
        Depends { name: "cpp" }
        cpp.includePaths: ["..."]

        Depends { name: "instance" }
        Depends { name: "intrinsic" }

        files: [
            "Basic.cpp",
            "Basic.h",
            "Context.cpp",
            "Context.h",
            "Instance.cpp",
            "Instance.h",
            "Literal.cpp",
            "Literal.h",
            "basic/flags.h",
            "basic/list.h",
            "basic/str.h",
            "basic/u64.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "intrinsic" }
            Depends { name: "instance" }
        }
    }

}
