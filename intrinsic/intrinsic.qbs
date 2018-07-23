import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "intrinsic"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "expressionTree" }

        files: [
            "Argument.h",
            "Context.cpp",
            "Context.h",
            "Function.cpp",
            "Function.h",
            "Module.h",
            "ModuleOutput.h",
            "Type.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "expressionTree" }
        }
    }

    StaticLibrary {
        name: "intrinsicAdapter"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "instance" }
        Depends { name: "intrinsic" }

        files: [
            "Adapter.cpp",
            "Adapter.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance" }
            Depends { name: "intrinsic" }
        }
    }

    Application {
        name: "intrinsic.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "intrinsicAdapter" }
        Depends { name: "googletest.lib" }
        googletest.lib.useMain: true

        files: [
            "RegisterTest.cpp",
        ]
    }
}
