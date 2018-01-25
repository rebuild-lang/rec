import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "intrinsic"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "instance" }

        files: [
            "Argument.h",
            "Function.cpp",
            "Function.h",
            "Module.h",
            "ModuleOutput.h",
            "Type.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance" }
        }
    }

    StaticLibrary {
        name: "intrinsicAdapter"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "intrinsic" }

        files: [
            "Adapter.cpp",
            "Adapter.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "intrinsic" }
        }
    }

    Application {
        name: "intrinsic.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "intrinsicAdapter" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "RegisterTest.cpp",
        ]
    }
}
