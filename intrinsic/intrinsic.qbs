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
        cpp.cxxLanguageVersion: "c++17"

        Depends { name: "tools" }
        Depends { name: "instance" }

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]
            cpp.cxxLanguageVersion: "c++17"
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
