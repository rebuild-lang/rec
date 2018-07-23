import qbs

Project {
    name: "api.lib"
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "api.lib"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "instance.data" }
        Depends { name: "intrinsic.data" }

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

            Depends { name: "intrinsic.data" }
            Depends { name: "instance.data" }
        }
    }

}
