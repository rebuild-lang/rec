import qbs

Project {
    name: "Rebuild Experimental Compiler"
    minimumQbsVersion: "1.7.1"

    // Customizations:
    // - enforce debug symbols for debuggig
    //   modules.cpp.debugInformation:true

    references: [
        "third_party",
        "shared",
        "src",
    ]

    AutotestRunner {}

    Product {
        name: "cpp20"

        Export {
            Depends { name: "cpp" }
            cpp.cxxLanguageVersion: "c++20"
            cpp.treatWarningsAsErrors: true
            cpp.enableRtti: false

            Properties {
                condition: qbs.toolchain.contains('msvc')
                cpp.cxxFlags: base.concat(
                    "/permissive-", "/Zc:__cplusplus", "/Zc:inline", // best C++ compatibilty
                    "/diagnostics:caret", // better error messages
                    "/W4", // enable all warnings
                    "/external:anglebrackets", "/external:W0" // ignore warnings from external headers
                )
            }
            Properties {
                condition: qbs.toolchain.contains('clang')
                cpp.cxxFlags: base.concat(
                    "--pedantic", // best C++ compatibility
                    "-Wall", "-Wextra", // enable more warnings
                    "-Wno-missing-braces", // relax bracing rules
                    "-Wno-invalid-noreturn", // we need type for type checking
                    "-Wno-gnu-zero-variadic-macro-arguments" // google test uses this
                )
                cpp.cxxStandardLibrary: "libc++"
                cpp.staticLibraries: ["c++", "c++abi"]
            }
        }
    }

    Product {
        name: "[Extra Files]"
        files: [
            ".clang-format",
            ".clang-tidy",
            ".editorconfig",
            ".gitattributes",
            ".github/workflows/test_runner.yml",
            ".gitignore",
            ".travis.yml",
            "Makefile",
            "Readme.md",
            "Vagrantfile",
            "azure-pipelines.yml",
            "docs/modules.adoc",
            "docs/rebuild_API.adoc",
            "vagrant_install.sh",
            "vagrant_make.bat",
        ]
    }
}
