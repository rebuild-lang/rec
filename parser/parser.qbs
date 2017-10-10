import qbs

Project {
    minimumQbsVersion: "1.7.1"

    Product {
        name: "parser"
        Depends { name: "cpp" }
        Depends { name: "scanner" }

        files: [
            "token_preparation.cpp",
            "token_preparation.h",
        ]

        Export {
            Depends { name: "cpp" }
            Depends { name: "scanner" }
            cpp.includePaths: [".."]
        }
    }

    Application {
        name: "parser.tests"
        consoleApplication: true
        type: ["application", "autotest"]

        Depends { name: "parser" }
        Depends { name: "googletest" }
        googletest.useMain: true

        files: [
            "token_preparation_test.cpp",
        ]
    }
}
