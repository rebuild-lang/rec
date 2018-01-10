import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "intrinsic"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "instance" }

        files: [
            "Function.cpp",
            "Function.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "instance" }
        }
    }

    Application {
        name: "intrinsic.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "intrinsic" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "RegisterTest.cpp",
        ]
    }
}
