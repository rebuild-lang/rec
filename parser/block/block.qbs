import qbs

Project {
    minimumQbsVersion: "1.7.1"

    StaticLibrary {
        name: "blockToken"
        Depends { name: "cpp" }

        Depends { name: "filterToken" }

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

            Depends { name: "filterToken" }
        }
    }

    StaticLibrary {
        name: "blockParser"
        Depends { name: "cpp" }

        Depends { name: "filterToken" }
        Depends { name: "blockToken" }

        files: [
            "Parser.cpp",
            "Parser.h",
        ]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [".."]

            Depends { name: "blockToken" }
        }
    }

    Application {
        name: "blockParser.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "blockParser" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "blockParserTest.cpp",
        ]
    }
}
