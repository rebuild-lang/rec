import qbs

Project {
    minimumQbsVersion: "1.7.1"

    Product {
        name: "parser"
        Depends { name: "cpp" }
        Depends { name: "scanner" }

        files: [
            "block_line_grouping.cpp",
            "block_line_grouping.h",
            "grouping_builder.h",
            "grouping_data.h",
            "prepared_token.h",
            "prepared_token_builder.h",
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
            "block_line_grouping_test.cpp",
            "token_preparation_test.cpp",
        ]
    }
}
