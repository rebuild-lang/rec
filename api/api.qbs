import qbs

Project {
    minimumQbsVersion: "1.7.1"



    StaticLibrary {
        name: "api"
        Depends { name: "cpp" }
        cpp.includePaths: ["..."]

        Depends { name: "intrinsic" }
        Depends { name: "instance" }

        files: [
            "Basic.cpp",
            "flags.h",
            "list.h",
            "str.h",
            "u64.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "intrinsic" }
            Depends { name: "instance" }
        }
    }

}
