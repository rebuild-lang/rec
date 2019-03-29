import qbs

Project {
    minimumQbsVersion: "1.7.1"

    references: [
        "api.lib/api",
        "diagnostic.data/diagnostic",
        "diagnostic.ostream/diagnostic",
        "execution.lib/execution",
        "instance.view/instance",
        "instance.data/instance",
        "instance.ostream/instance",
        "intrinsic.data/intrinsic",
        "intrinsic.ostream/intrinsic",
        "intrinsic.lib/intrinsic",
        "lexer",
        "parser.data/parser",
        "parser.ostream/parser",
        "parser.lib/parser",
        "rec.app/rec",
    ]
}
