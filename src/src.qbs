import qbs

Project {
    minimumQbsVersion: "1.7.1"

    references: [
        "lexer",
        "parser.data/parser",
        "parser.ostream/parser",
        "parser.lib/parser",
        "instance.view/instance",
        "instance.data/instance",
        "instance.ostream/instance",
        "execution.lib/execution",
        "intrinsic.data/intrinsic",
        "intrinsic.ostream/intrinsic",
        "intrinsic.lib/intrinsic",
        "api.lib/api",
        "rec.app/rec",
    ]
}