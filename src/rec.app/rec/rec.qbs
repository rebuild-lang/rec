import qbs

Project {
    name: "rec.app"

    Application {
        name: "rec.app"
        targetName: "rec"

        consoleApplication: true
        Depends { name: "rec.lib" }
        files: [
            "main.cpp",
        ]
    }
}
