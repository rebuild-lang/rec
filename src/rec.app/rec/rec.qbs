import qbs

Project {
    name: "rec.app"
    minimumQbsVersion: "1.7.1"

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
