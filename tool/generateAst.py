#!/usr/bin/env python3
import sys
import os


def preprocessTypes(baseName: str, types: list[str]):
    processedClasses = list[dict]()
    for Type in types:
        (className, fieldLines) = [x.strip() for x in Type.split("=")]
        fields = list[dict]()
        for field in fieldLines.split(","):
            (fieldType, name) = field.strip().split(" ")
            fields.append({
                "Type": fieldType.strip(),
                "Name": name.strip()
            })
        processedClass = {
            "Name": className,
            "Fields": fields
        }
        processedClasses.append(processedClass)
    return processedClasses


def defineAst(outputDir: str, baseName: str, requiredHeaders: list[str], types: list[str]):
    filePath = os.path.join(outputDir, f"{baseName.lower()}.hpp")
    processedClasses = preprocessTypes(baseName, types)
    with open(filePath, "w") as headerFile:
        headerFile.write("#pragma once\n")
        for header in requiredHeaders:
            headerFile.write(f"#include <{header}>\n")
        headerFile.write("\n")
        headerFile.write("namespace ray::compiler::ast {\n")
        headerFile.write("\n")
        headerFile.write(f"class {baseName} {{\n")
        headerFile.write("  public:\n")
        headerFile.write("};\n\n")

        for clazz in processedClasses:
            result = defineType(baseName, clazz)
            headerFile.write(result)
            headerFile.write("\n")

        headerFile.write("\n")

        headerFile.write("} // namespace ray::compiler::ast\n")


def defineType(baseName: str, clazz: dict[str]):
    stringList = list[str]()

    stringList.append(
        f"class {clazz["Name"]} : public {baseName} {{\n")
    stringList.append(f"  public:\n")
    for field in clazz["Fields"]:
        stringList.append(
            f"\tstd::unique_ptr<{field["Type"]}> {field["Name"]};\n")
    stringList.append("\n")

    # define constructor
    stringList.append(f"\t{clazz["Name"]}(")
    constructorParams = list[str]()
    for field in clazz["Fields"]:
        constructorParams.append(
            f"std::unique_ptr<{field["Type"]}> {field["Name"]}")

    stringList.append(f",\n\t{" " * 8}".join(constructorParams))
    stringList.append(f")\n\t{" " * 4}: ")
    initializerList = list[str]()
    for field in clazz["Fields"]:
        initializerList.append(f"{field["Name"]}(std::move({field["Name"]}))")

    stringList.append(", ".join(initializerList))
    stringList.append(" {}\n\n")

    stringList.append("};")

    return "".join(stringList)



def main():
    outputDir = "./RayC/include/ray/compiler/ast"
    defineAst(outputDir, "Expression", ["memory", "ray/compiler/lexer/token.hpp"], [
        "Ternary		= Expression cond, Expression left, Expression right",
        "Variable		= Token name",
    ])
    defineAst(outputDir, "Statement",
            ["memory",
             "vector",
             "ray/compiler/lexer/token.hpp",
             "ray/compiler/ast/expression.hpp"
            ],
            [
                "Block			= std::vector<Statement> statements",
                "TerminalExpr	= Expression expression",
                "ExpressionStmt	= Expression expression",
                "Function		= Token name, std::vector<Token> params, std::vector<Statement> body",
                "If				= Expression condition, Statement thenBranch, Statement elseBranch",
                "Print			= Expression expression",
                "Jump			= Token keyword, Expression value",
                "Var			= Token name, Expression initializer",
                "While			= Expression condition, Statement body"
            ])


if __name__ == "__main__":
    sys.exit(main())
