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
        //multiplexByQbsProperties: ["buildVariants", "profiles"]
        //qbs.buildVariants: ["debug", "release"]
        name: "rec"
        consoleApplication: true
        Depends { name: "scanner" }
        Depends { name: "parser" }
        Depends { name: "intrinsicAdapter" }
        Depends { name: "api" }
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
