import qbs

Project {
    minimumQbsVersion: "1.7.1"

    Product {
        name: "parser"
        Depends { name: "cpp" }
        Depends { name: "scanner" }

        files: [
            "block/block_parser.cpp",
            "block/block_parser.h",
            "block/block_token.h",
            "block/block_token_builder.h",
            "expression/expression_line_view.h",
            "expression/expression_parser.cpp",
            "expression/expression_parser.h",
            "expression/expression_tree.h",
            "expression/expression_tree_builder.h",
            "filter/filter_parser.cpp",
            "filter/filter_parser.h",
            "filter/filter_token.h",
            "filter/filter_token_builder.h",
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
            "block/block_parser_test.cpp",
            "expression/expression_parser_test.cpp",
            "filter/filter_parser_test.cpp",
        ]
    }
}
