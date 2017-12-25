import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        files: [
            "Function.cpp",
            "Function.h",
        ]
        name: "intrinsic"
        Depends { name: "cpp" }

        Depends { name: "tools" }
        Depends { name: "instance" }

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]
        }
    }

    Application {
        name: "intrinsic.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "tools" }
        Depends { name: "intrinsic" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "RegisterTest.cpp",
        ]
    }
}
