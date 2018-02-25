import qbs

Project {
    minimumQbsVersion: "1.7.1"

    references: [
        "thirdparty",
        "tools",
        "scanner",
        "instance",
        "parser",
        "intrinsic",
        "api",
        "execution",
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
            "Makefile",
            "Readme.md",
            "Vagrantfile",
            "vagrant_install.sh",
            "vagrant_make.bat",
        ]
    }
}
