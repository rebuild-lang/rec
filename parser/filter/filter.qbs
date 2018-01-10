import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "filterToken"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "token" }

        files: [
            "Token.cpp",
            "Token.h",
            "TokenBuilder.h",
            "TokenOutput.cpp",
            "TokenOutput.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "token" }
        }
    }

    StaticLibrary {
        name: "filterParser"
        Depends { name: "cpp" }
        cpp.includePaths: [".."]

        Depends { name: "token" }
        Depends { name: "filterToken" }

        files: [
            "Parser.cpp",
            "Parser.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "filterToken" }
        }
    }

    Application {
        name: "filterParser.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "filterParser" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "filterParserTest.cpp",
        ]
    }
}
