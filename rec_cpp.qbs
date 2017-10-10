import qbs

Project {
    minimumQbsVersion: "1.7.1"

    references: [
        "tools",
        "scanner",
        "parser",
        "thirdparty",
    ]

    AutotestRunner {}

    Application {
        name: "rec"
        consoleApplication: true
        Depends { name: "scanner" }
        files: [
            "main.cpp",
        ]
    }

    Product {
        name: "Extra Files"
        files: [
            ".clang-format",
            ".editorconfig",
            ".gitignore",
            "Readme.md",
        ]
    }
}
