import qbs

Project {
    name: "api.lib"

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
            "Parser.cpp",
            "Parser.h",
            "basic/flags.h",
            "basic/list.h",
            "basic/pointer.h",
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

    Application {
        name: "api.tests"
        consoleApplication: true
        type: base.concat("autotest")

        Depends { name: "api.lib" }
        Depends { name: "instance.ostream" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "api.test.cpp",
        ]
    }
}
